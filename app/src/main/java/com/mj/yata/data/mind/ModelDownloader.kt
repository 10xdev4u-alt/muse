package com.mj.yata.data.mind

import android.content.Context
import android.net.ConnectivityManager
import android.net.NetworkCapabilities
import androidx.annotation.VisibleForTesting
import dagger.hilt.android.qualifiers.ApplicationContext
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream
import java.net.HttpURLConnection
import java.net.URL
import java.security.DigestInputStream
import java.security.MessageDigest
import javax.inject.Inject
import javax.inject.Singleton
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.currentCoroutineContext
import kotlinx.coroutines.ensureActive
import kotlinx.coroutines.job
import kotlinx.coroutines.withContext

/** Lifecycle of the one-time model acquisition. */
sealed class DownloadState {
    data object Idle : DownloadState()
    data class Downloading(val bytesDone: Long, val bytesTotal: Long) : DownloadState() {
        val fraction: Float get() = if (bytesTotal > 0) bytesDone.toFloat() / bytesTotal else 0f
    }

    /** Download finished and checksum verified — weights + metadata in place. */
    data object Ready : DownloadState()

    data class Failed(val reason: Reason) : DownloadState()

    enum class Reason { INSUFFICIENT_STORAGE, METERED_BLOCKED, CHECKSUM_MISMATCH, NETWORK }
}

/** What the MindViewModel may know about model acquisition — nothing about HTTP. */
interface ModelAcquisition {
    val state: StateFlow<DownloadState>
    fun isModelPresent(): Boolean
    suspend fun download()
}

/**
 * One-time GGUF fetch into XybridRuntime.modelDir, with HTTP Range resume,
 * streaming SHA256 verification against the pinned hash from
 * models/lfm2.5-1.2b-instruct/README.md, and the #22 storage guard.
 *
 * Resume model: partial data lives in `<gguf>.part`; cancelling keeps it, the
 * next attempt sends Range: bytes=<len>- and appends a 206 response (a 200
 * means the server ignored Range — restart clean).
 */
@Singleton
class ModelDownloader @Inject constructor(
    @ApplicationContext private val context: Context
) : ModelAcquisition {
    private val _state = MutableStateFlow<DownloadState>(DownloadState.Idle)
    override val state: StateFlow<DownloadState> = _state.asStateFlow()

    val modelDir: File
        get() = File(context.filesDir, "xybrid/${XybridRuntime.MODEL_ID}").apply { mkdirs() }

    override fun isModelPresent(): Boolean =
        modelDir.resolve("model_metadata.json").isFile &&
            modelDir.listFiles()?.any { it.name.endsWith(".gguf") } == true

    var allowMetered: Boolean = false

    @VisibleForTesting
    fun stateSinkForTests(): MutableStateFlow<DownloadState> = _state

    override suspend fun download() {
        try {
            withContext(Dispatchers.IO) {
                val dir = modelDir
                writeMetadataIfNeeded(dir)

                if (!ModelStorageGuard.hasRoom(dir.usableSpace)) {
                    _state.value = DownloadState.Failed(DownloadState.Reason.INSUFFICIENT_STORAGE)
                    return@withContext
                }
                if (!allowMetered && isMetered()) {
                    _state.value = DownloadState.Failed(DownloadState.Reason.METERED_BLOCKED)
                    return@withContext
                }

                val finalFile = File(dir, GGUF_NAME)
                val partFile = File(dir, "$GGUF_NAME.part")
                downloadWithResume(partFile, currentCoroutineContext().job)
                verifyChecksum(partFile)
                if (!partFile.renameTo(finalFile)) {
                    // Cross-file-system renames can fail; fall back to copy+delete.
                    partFile.copyTo(finalFile, overwrite = true)
                    partFile.delete()
                }
                _state.value = DownloadState.Ready
            }
        } catch (e: CancellationException) {
            // Pause: keep .part for resume; state returns to Idle via cancel caller.
            throw e
        } catch (e: Exception) {
            _state.value = DownloadState.Failed(DownloadState.Reason.NETWORK)
        }
    }

    private fun isMetered(): Boolean {
        val cm = context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
        val caps = cm.getNetworkCapabilities(cm.activeNetwork) ?: return true
        return !caps.hasCapability(NetworkCapabilities.NET_CAPABILITY_NOT_METERED)
    }

    private fun writeMetadataIfNeeded(dir: File) {
        val meta = File(dir, "model_metadata.json")
        if (meta.isFile && meta.length() > 0) return
        context.assets.open("xybrid/model_metadata.json").use { input ->
            FileOutputStream(meta).use { output -> input.copyTo(output) }
        }
    }

    private fun downloadWithResume(partFile: File, job: kotlinx.coroutines.Job) {
        val alreadyHave = if (partFile.exists()) partFile.length() else 0L
        val conn = (URL(GGUF_URL).openConnection() as HttpURLConnection).apply {
            connectTimeout = 15_000
            readTimeout = 30_000
            instanceFollowRedirects = true
            if (alreadyHave > 0) setRequestProperty("Range", "bytes=$alreadyHave-")
        }
        val code = conn.responseCode
        if (code !in 200..299) {
            _state.value = DownloadState.Failed(DownloadState.Reason.NETWORK)
            return
        }
        val resuming = code == 206 && alreadyHave > 0
        if (!resuming && alreadyHave > 0) partFile.delete() // server ignored Range

        val total = if (resuming) {
            alreadyHave + (conn.contentLengthLong.takeIf { it > 0 } ?: 0L)
        } else {
            conn.contentLengthLong.takeIf { it > 0 } ?: ModelStorageGuard.MODEL_BYTES
        }
        updateProgress(if (resuming) alreadyHave else 0L, total)

        FileOutputStream(partFile, resuming).use { out ->
            DigestInputStream(conn.inputStream, MessageDigest.getInstance("SHA-256")).use { input ->
                val buf = ByteArray(64 * 1024)
                var written = if (resuming) alreadyHave else 0L
                while (true) {
                    // Cooperative pause: job cancelled between chunks, .part preserved.
                    job.ensureActive()
                    val n = input.read(buf)
                    if (n == -1) break
                    out.write(buf, 0, n)
                    written += n
                    updateProgress(written, total)
                }
            }
        }
        conn.disconnect()
    }

    private fun verifyChecksum(file: File) {
        val digest = MessageDigest.getInstance("SHA-256")
        FileInputStream(file).use { input ->
            val buf = ByteArray(64 * 1024)
            while (true) {
                val n = input.read(buf)
                if (n == -1) break
                digest.update(buf, 0, n)
            }
        }
        val hex = digest.digest().joinToString("") { "%02x".format(it) }
        if (!hex.equals(PINNED_SHA256, ignoreCase = true)) {
            file.delete()
            _state.value = DownloadState.Failed(DownloadState.Reason.CHECKSUM_MISMATCH)
            return
        }
    }

    private fun updateProgress(done: Long, total: Long) {
        _state.value = DownloadState.Downloading(done, total)
    }

    companion object {
        const val GGUF_NAME = "LFM2.5-1.2B-Instruct-Q4_K_M.gguf"
        const val GGUF_URL =
            "https://huggingface.co/LiquidAI/LFM2.5-1.2B-Instruct-GGUF/resolve/main/$GGUF_NAME?download=true"
        const val PINNED_SHA256 =
            "b1b3de114215d9507409a662a501a631095a479a419584e8a2ded6304b19b4f5"
    }
}
