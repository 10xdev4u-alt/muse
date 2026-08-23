package com.mj.yata.ui.screen.mind

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.imePadding
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.widthIn
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.Send
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.Close
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.History
import androidx.compose.material.icons.filled.Psychology
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Scaffold
import androidx.compose.material3.SnackbarHost
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.unit.dp
import androidx.hilt.navigation.compose.hiltViewModel
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.mj.yata.domain.model.JournalRole
import com.mj.yata.domain.mind.MindError
import com.mj.yata.ui.widgets.MarkdownText

/**
 * Chat surface for one journaling session. Assistant text renders as markdown;
 * user text stays plain — you wrote it, it shouldn't get re-interpreted.
 */
@Composable
fun MindScreen(
    viewModel: MindViewModel = hiltViewModel()
) {
    val uiState by viewModel.uiState.collectAsStateWithLifecycle()
    val entries by viewModel.entries.collectAsStateWithLifecycle()
    val summaries by viewModel.sessionSummaries.collectAsStateWithLifecycle()
    val deletedSessionId by viewModel.deletedSessionId.collectAsStateWithLifecycle()
    var showHistory by remember { mutableStateOf(false) }
    val listState = rememberLazyListState()
    val snackbarHostState = remember { SnackbarHostState() }

    LaunchedEffect(entries.size) {
        if (entries.isNotEmpty()) listState.animateScrollToItem(entries.lastIndex)
    }

    LaunchedEffect(deletedSessionId) {
        if (deletedSessionId != null) {
            val result = snackbarHostState.showSnackbar(
                message = "Session deleted",
                actionLabel = "Undo",
                withDismissAction = true
            )
            if (result == androidx.compose.material3.SnackbarResult.ActionPerformed) {
                viewModel.undoDelete()
            } else {
                viewModel.dismissDeletedSnackbar()
            }
        }
    }

    LaunchedEffect(uiState.error) {
        val message = when (uiState.error) {
            MindError.ENGINE -> "Reflection failed mid-turn. Partial answer kept."
            MindError.MODEL_MISSING -> "Model files missing. Open Settings to download."
            MindError.OFFLINE -> "This step needs a connection. Try again once online."
            MindError.STORAGE -> "Could not save to the journal."
            null -> null
        }
        message?.let {
            snackbarHostState.showSnackbar(it)
            viewModel.dismissError()
        }
    }

    Scaffold(
        snackbarHost = { SnackbarHost(snackbarHostState) }
    ) { padding ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(padding)
                .imePadding()
        ) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 12.dp, vertical = 4.dp),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text("Mind", style = MaterialTheme.typography.titleMedium)
                Row {
                    IconButton(onClick = { showHistory = true }) {
                        Icon(Icons.Default.History, contentDescription = "History")
                    }
                    IconButton(onClick = viewModel::newSession) {
                        Icon(Icons.Default.Add, contentDescription = "New session")
                    }
                }
            }

            if (entries.isEmpty()) {
                EmptySessionContent(
                    onStarterSelected = viewModel::onInputChanged,
                    modifier = Modifier.weight(1f)
                )
            } else {
            LazyColumn(
                state = listState,
                modifier = Modifier
                    .weight(1f)
                    .fillMaxWidth(),
                verticalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                items(entries, key = { it.id }) { entry ->
                    MessageBubble(
                        text = entry.body,
                        isUser = entry.role == JournalRole.USER,
                        modifier = Modifier.padding(horizontal = 12.dp)
                    )
                }
            }
            }

            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 12.dp, vertical = 8.dp),
                verticalAlignment = Alignment.Bottom
            ) {
                OutlinedTextField(
                    value = uiState.inputDraft,
                    onValueChange = viewModel::onInputChanged,
                    modifier = Modifier.weight(1f),
                    placeholder = { Text("Pour it out…") },
                    maxLines = 5
                )
                IconButton(
                    onClick = {
                        if (uiState.isReflecting) viewModel.stopReflection() else viewModel.send()
                    }
                ) {
                    Icon(
                        imageVector = if (uiState.isReflecting) Icons.Default.Close else Icons.AutoMirrored.Filled.Send,
                        contentDescription = if (uiState.isReflecting) "Stop" else "Send"
                    )
                }
            }
        }

        MindDownloadDialog(viewModel)

        if (showHistory) {
            MindHistorySheet(
                summaries = summaries,
                onOpen = { id ->
                    showHistory = false
                    viewModel.openSession(id)
                },
                onDelete = viewModel::deleteSession,
                onDismiss = { showHistory = false }
            )
        }
    }
}

@Composable
private fun EmptySessionContent(
    onStarterSelected: (String) -> Unit,
    modifier: Modifier = Modifier
) {
    val starters = listOf(
        "Today felt like…",
        "I keep avoiding…",
        "What I actually want is…"
    )
    Column(
        modifier = modifier.fillMaxWidth().padding(horizontal = 24.dp),
        verticalArrangement = androidx.compose.foundation.layout.Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Icon(
            Icons.Default.Psychology,
            contentDescription = null,
            tint = MaterialTheme.colorScheme.primary,
            modifier = Modifier.padding(bottom = 12.dp)
        )
        Text(
            "What's on your mind?",
            style = MaterialTheme.typography.headlineSmall
        )
        Text(
            "Write it raw. Muse reflects — it doesn't advise.",
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            modifier = Modifier.padding(top = 4.dp, bottom = 20.dp)
        )
        starters.forEach { starter ->
            androidx.compose.material3.SuggestionChip(
                onClick = { onStarterSelected(starter) },
                label = { Text(starter) },
                modifier = Modifier.padding(vertical = 4.dp)
            )
        }
    }
}

@Composable
private fun MessageBubble(
    text: String,
    isUser: Boolean,
    modifier: Modifier = Modifier
) {
    Box(modifier = modifier.fillMaxWidth()) {
        Box(
            modifier = Modifier
                .align(if (isUser) Alignment.CenterEnd else Alignment.CenterStart)
                .widthIn(max = 320.dp)
                .clip(
                    RoundedCornerShape(
                        topStart = 16.dp,
                        topEnd = 16.dp,
                        bottomStart = if (isUser) 16.dp else 4.dp,
                        bottomEnd = if (isUser) 4.dp else 16.dp
                    )
                )
                .background(
                    if (isUser) MaterialTheme.colorScheme.primaryContainer
                    else MaterialTheme.colorScheme.surfaceVariant
                )
                .padding(horizontal = 14.dp, vertical = 10.dp)
        ) {
            if (isUser) {
                Text(text = text, style = MaterialTheme.typography.bodyLarge)
            } else {
                MarkdownText(markdown = text)
            }
        }
    }
}
