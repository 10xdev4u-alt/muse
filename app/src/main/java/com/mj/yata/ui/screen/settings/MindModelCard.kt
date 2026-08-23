package com.mj.yata.ui.screen.settings

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Card
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.hilt.navigation.compose.hiltViewModel
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.mj.yata.data.mind.ModelAcquisition
import com.mj.yata.domain.mind.ReflectionEngine
import dagger.hilt.android.lifecycle.HiltViewModel
import java.io.File
import javax.inject.Inject
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch

@HiltViewModel
class MindModelViewModel @Inject constructor(
    private val acquisition: ModelAcquisition,
    private val engine: ReflectionEngine
) : ViewModel() {

    private val modelDir: File get() = acquisition.modelDir

    private val _modelPresent = MutableStateFlow(acquisition.isModelPresent())
    val modelPresent: StateFlow<Boolean> = _modelPresent.asStateFlow()

    /** Refreshed on recomposition triggers via refresh(); cheap directory stat. */
    fun storageUsedBytes(): Long =
        modelDir.walkTopDown().filter { it.isFile }.sumOf { it.length() }

    fun modelPresentNow(): Boolean = acquisition.isModelPresent()

    suspend fun deleteModel() {
        engine.release()
        modelDir.deleteRecursively()
        _modelPresent.value = false
    }

    fun refresh() {
        _modelPresent.value = acquisition.isModelPresent()
    }

    fun deleteModelInScope() {
        viewModelScope.launch {
            deleteModel()
            refresh()
        }
    }
}


/**
 * Settings card for the on-device reflection model: presence, disk usage,
* and delete (with confirm). Download lives in the Mind tab's first-run dialog;
* this card is management, not acquisition.
 */
@Composable
fun MindModelCard(viewModel: MindModelViewModel = hiltViewModel()) {
    val present by viewModel.modelPresent.collectAsStateWithLifecycle()
    var confirmDelete by remember { mutableStateOf(false) }

    // Cheap stat re-read whenever presence flips.
    val usedMb = remember(present) {
        if (present) viewModel.storageUsedBytes() / 1_000_000 else 0
    }

    Card(modifier = Modifier.fillMaxWidth().padding(horizontal = 16.dp, vertical = 8.dp)) {
        Column(modifier = Modifier.padding(16.dp)) {
            Text("Mind model", style = MaterialTheme.typography.titleMedium)
            Text(
                text = if (present) "Downloaded · ${usedMb} MB on device"
                else "Not downloaded — open the Mind tab to fetch it",
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                modifier = Modifier.padding(top = 4.dp, bottom = 12.dp)
            )
            Row(verticalAlignment = Alignment.CenterVertically) {
                if (present) {
                    Button(onClick = { confirmDelete = true }) { Text("Delete") }
                    Text(
                        "Frees space; Mind will offer the download again.",
                        style = MaterialTheme.typography.labelSmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant,
                        modifier = Modifier.padding(start = 10.dp).weight(1f)
                    )
                }
            }
        }
    }

    if (confirmDelete) {
        AlertDialog(
            onDismissRequest = { confirmDelete = false },
            title = { Text("Delete the model?") },
            text = { Text("Removes ~${usedMb} MB. Your journal entries are untouched. Mind will ask to download again next time.") },
            confirmButton = {
                Button(onClick = {
                    confirmDelete = false
                    viewModel.deleteModelInScope()
                }) { Text("Delete") }
            },
            dismissButton = {
                OutlinedButton(onClick = { confirmDelete = false }) { Text("Cancel") }
            }
        )
    }
}
