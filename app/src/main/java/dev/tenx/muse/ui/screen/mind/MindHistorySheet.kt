package dev.tenx.muse.ui.screen.mind

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import dev.tenx.muse.domain.model.JournalSessionSummary
import java.text.SimpleDateFormat
import java.util.Calendar
import java.util.Date
import java.util.Locale

/**
 * Session picker. Rows group under day headers (Today / Yesterday / date) by
 * session start time — matching how the drawer and agenda already talk about
 * days in this app.
 */
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun MindHistorySheet(
    summaries: List<JournalSessionSummary>,
    onOpen: (String) -> Unit,
    onDelete: (String) -> Unit,
    onDismiss: () -> Unit
) {
    val dayFormat = SimpleDateFormat("EEEE, MMM d", Locale.getDefault())
    val groups = summaries.groupBy { summary ->
        val cal = Calendar.getInstance().apply { timeInMillis = summary.startedAt }
        val today = Calendar.getInstance()
        when {
            cal.get(Calendar.YEAR) == today.get(Calendar.YEAR) &&
                cal.get(Calendar.DAY_OF_YEAR) == today.get(Calendar.DAY_OF_YEAR) -> "Today"
            cal.get(Calendar.YEAR) == today.get(Calendar.YEAR) &&
                cal.get(Calendar.DAY_OF_YEAR) == today.get(Calendar.DAY_OF_YEAR) - 1 -> "Yesterday"
            else -> dayFormat.format(Date(summary.startedAt))
        }
    }

    ModalBottomSheet(onDismissRequest = onDismiss) {
        LazyColumn(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 16.dp)
                .padding(bottom = 24.dp),
            verticalArrangement = Arrangement.spacedBy(4.dp)
        ) {
            if (summaries.isEmpty()) {
                item {
                    Text(
                        "No sessions yet.",
                        style = MaterialTheme.typography.bodyMedium,
                        modifier = Modifier.padding(vertical = 24.dp)
                    )
                }
            }
            groups.forEach { (day, rows) ->
                item(key = "header-$day") {
                    Text(
                        day,
                        style = MaterialTheme.typography.labelLarge,
                        color = MaterialTheme.colorScheme.primary,
                        modifier = Modifier.padding(top = 12.dp, bottom = 4.dp)
                    )
                }
                items(rows, key = { it.sessionId }) { row ->
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .clickable { onOpen(row.sessionId) }
                            .padding(vertical = 10.dp, horizontal = 4.dp),
                        verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.SpaceBetween
                    ) {
                        Column(modifier = Modifier.weight(1f)) {
                            Text(
                                text = row.preview?.takeIf { it.isNotBlank() } ?: "(no text)",
                                style = MaterialTheme.typography.bodyLarge,
                                maxLines = 2
                            )
                            Text(
                                text = SimpleDateFormat("HH:mm", Locale.getDefault())
                                    .format(Date(row.startedAt)),
                                style = MaterialTheme.typography.labelSmall,
                                color = MaterialTheme.colorScheme.onSurfaceVariant
                            )
                        }
                        IconButton(onClick = { onDelete(row.sessionId) }) {
                            Icon(
                                Icons.Default.Delete,
                                contentDescription = "Delete session",
                                tint = MaterialTheme.colorScheme.onSurfaceVariant
                            )
                        }
                    }
                }
            }
        }
    }
}
