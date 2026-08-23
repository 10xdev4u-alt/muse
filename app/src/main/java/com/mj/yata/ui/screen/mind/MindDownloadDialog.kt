package com.mj.yata.ui.screen.mind

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.hilt.navigation.compose.hiltViewModel
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.mj.yata.data.mind.DownloadState
import com.mj.yata.data.mind.ModelStorageGuard

/**
 * First-run acquisition dialog for the reflection model. Shows size up front,
 * streams byte progress, pauses via cancel (resumable), and translates every
 * failure reason into copy with a next action.
 */
@Composable
fun MindDownloadDialog(
    viewModel: MindViewModel = hiltViewModel()
) {
    val show by viewModel.showDownloadSheet.collectAsStateWithLifecycle()
    val download by viewModel.modelDownloader.state.collectAsStateWithLifecycle()
    if (!show) return

    AlertDialog(
        onDismissRequest = { if (download is DownloadState.Ready) viewModel.dismissDownloadSheet() },
        title = { Text("Download Muse's model?") },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                when (val d = download) {
                    is DownloadState.Downloading -> {
                        LinearProgressIndicator(
                            progress = { d.fraction },
                            modifier = Modifier.fillMaxWidth()
                        )
                        Text(
                            "${d.bytesDone / 1_000_000} / ${d.bytesTotal / 1_000_000} MB",
                            style = MaterialTheme.typography.labelMedium
                        )
                    }
                    is DownloadState.Failed -> Text(
                        when (d.reason) {
                            DownloadState.Reason.INSUFFICIENT_STORAGE ->
                                "Not enough free space — need about ${ModelStorageGuard.requiredBytes() / 1_000_000} MB."
                            DownloadState.Reason.METERED_BLOCKED ->
                                "This is a 731 MB download. Connect to Wi-Fi or allow mobile data and retry."
                            DownloadState.Reason.CHECKSUM_MISMATCH ->
                                "Download failed integrity check and was deleted. Retry to re-download."
                            DownloadState.Reason.NETWORK ->
                                "Connection dropped. Retry — it resumes where it left off."
                        },
                        style = MaterialTheme.typography.bodyMedium
                    )
                    else -> Text(
                        "One-time ${ModelStorageGuard.MODEL_BYTES / 1_000_000} MB download over Wi-Fi. " +
                            "After this, everything runs on your phone — no internet needed.",
                        style = MaterialTheme.typography.bodyMedium
                    )
                }
            }
        },
        confirmButton = {
            when (download) {
                is DownloadState.Downloading -> {
                    Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                        CircularProgressIndicator(
                            modifier = Modifier.padding(6.dp).align(Alignment.CenterVertically)
                        )
                        OutlinedButton(onClick = viewModel::pauseDownload) { Text("Pause") }
                    }
                }
                is DownloadState.Ready ->
                    Button(onClick = viewModel::dismissDownloadSheet) { Text("Start reflecting") }
                else -> Button(onClick = viewModel::startDownload) {
                    Text(if (download is DownloadState.Failed) "Retry" else "Download")
                }
            }
        },
        dismissButton = {
            if (download !is DownloadState.Downloading && download != DownloadState.Ready &&
                modelPresent(viewModel)
            ) {
                OutlinedButton(onClick = viewModel::dismissDownloadSheet) { Text("Later") }
            }
        }
    )
}

private fun modelPresent(viewModel: MindViewModel): Boolean = viewModel.modelDownloader.isModelPresent()
