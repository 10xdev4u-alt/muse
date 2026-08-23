package dev.tenx.muse.data.mind

import android.content.Context
import dev.tenx.muse.domain.mind.ReflectionEngine
import dagger.hilt.android.qualifiers.ApplicationContext
import ai.xybrid.ModelSource
import ai.xybrid.Envelope
import ai.xybrid.Xybrid
import ai.xybrid.XybridModel
import ai.xybrid.XybridModelLoader
import ai.xybrid.streamTokens
import java.io.File
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flow
import kotlinx.coroutines.flow.flowOn
import kotlinx.coroutines.withContext
import javax.inject.Inject
import javax.inject.Singleton

/**
 * The ONLY class in Muse allowed to import ai.xybrid.* (ADR-004). Wraps the
 * Xybrid Kotlin SDK behind [ReflectionEngine]; everything upstream stays on
 * the interface.
 *
 * Model location contract (mirrors models/lfm2.5-1.2b-instruct/ in-repo):
 * filesDir/xybrid/lfm2.5-1.2b-instruct/{model_metadata.json, *.gguf}.
 * The downloader (#21) populates it; prepare() fails loudly if absent so the
 * UI can route to the download flow instead of silently spinning.
 */
@Singleton
class XybridRuntime @Inject constructor(
    @ApplicationContext private val context: Context
) : ReflectionEngine {

    private var model: XybridModel? = null

    val modelDir: File
        get() = File(context.filesDir, "xybrid/$MODEL_ID").apply { mkdirs() }

    val isModelPresent: Boolean
        get() = modelDir.resolve("model_metadata.json").isFile &&
            modelDir.listFiles()?.any { it.name.endsWith(".gguf") } == true

    override suspend fun prepare() {
        if (model != null) return
        withContext(Dispatchers.IO) {
            check(isModelPresent) {
                "Model not downloaded yet: ${modelDir.path}"
            }
            // Idempotent per SDK docs; also wires battery/thermal observers once.
            Xybrid.init(context)
            model = XybridModelLoader.fromDirectory(modelDir.absolutePath).load()
        }
    }

    override fun reflect(prompt: String): Flow<String> = flow {
        val current = checkNotNull(model) {
            "prepare() must complete before reflect()"
        }
        // streamTokens is the SDK's Flow wrapper over its pull session API:
        // cancelling this collection aborts generation at a token boundary
        // and closes the native session (verified in research #4's audit).
        current.streamTokens(Envelope.text(prompt)).collect { token ->
            emit(token.token)
        }
    }.flowOn(Dispatchers.IO)

    override suspend fun release() {
        withContext(Dispatchers.IO) {
            model?.let {
                runCatching { it.unload() }
            }
            model = null
        }
    }

    companion object {
        const val MODEL_ID = "lfm2.5-1.2b-instruct"
    }
}
