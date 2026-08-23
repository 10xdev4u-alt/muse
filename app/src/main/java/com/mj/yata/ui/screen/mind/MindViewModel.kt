package com.mj.yata.ui.screen.mind

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.mj.yata.domain.mind.MindError
import com.mj.yata.domain.mind.ReflectionEngine
import com.mj.yata.domain.repository.JournalRepository
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.ExperimentalCoroutinesApi
import java.util.UUID
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.flatMapLatest
import kotlinx.coroutines.flow.stateIn
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import javax.inject.Inject

data class MindUiState(
    val inputDraft: String = "",
    val isReflecting: Boolean = false,
    val error: MindError? = null
)

@OptIn(ExperimentalCoroutinesApi::class)
@HiltViewModel
class MindViewModel @Inject constructor(
    private val journalRepository: JournalRepository,
    private val reflectionEngine: ReflectionEngine,
    val modelDownloader: com.mj.yata.data.mind.ModelAcquisition
) : ViewModel() {

    private val sessionIdInternal = MutableStateFlow(UUID.randomUUID().toString())

    /** Open session id. Public because the screen shows session identity and tests assert on it. */
    val sessionId: StateFlow<String> = sessionIdInternal.asStateFlow()

    private val _uiState = MutableStateFlow(MindUiState())
    val uiState: StateFlow<MindUiState> = _uiState.asStateFlow()

    /** History rows for the session picker. */
    val sessionSummaries: StateFlow<List<com.mj.yata.domain.model.JournalSessionSummary>> =
        journalRepository.observeSessionSummaries()
            .stateIn(viewModelScope, SharingStarted.WhileSubscribed(5_000), emptyList())

    /** Session id awaiting undo, surfaced by the screen as a snackbar action. */
    private val _deletedSessionId = MutableStateFlow<String?>(null)
    val deletedSessionId: StateFlow<String?> = _deletedSessionId.asStateFlow()

    /** Entries of the open session, straight from Room — screen text IS database text. */
    val entries: StateFlow<List<com.mj.yata.domain.model.JournalEntry>> = sessionId
        .flatMapLatest { journalRepository.observeSession(it) }
        .stateIn(viewModelScope, SharingStarted.WhileSubscribed(5_000), emptyList())

    private var reflectJob: Job? = null
    private var lastDeletedSnapshot: Pair<String, List<com.mj.yata.domain.model.JournalEntry>>? = null

    fun onInputChanged(text: String) {
        _uiState.update { it.copy(inputDraft = text) }
    }

    fun newSession() {
        reflectJob?.cancel()
        sessionIdInternal.value = UUID.randomUUID().toString()
        _uiState.update { it.copy(inputDraft = "", isReflecting = false, error = null) }
    }

    fun openSession(id: String) {
        if (id == sessionId.value) return
        reflectJob?.cancel()
        sessionIdInternal.value = id
        _uiState.update { it.copy(isReflecting = false, error = null) }
    }

    fun send() {
        val current = _uiState.value
        val text = current.inputDraft.trim()
        if (text.isEmpty() || current.isReflecting) return

        reflectJob = viewModelScope.launch {
            try {
                journalRepository.appendUserEntry(sessionIdInternal.value, text)
                reflectionEngine.prepare()
                val reply = journalRepository.beginAssistantReply(sessionIdInternal.value)
                var accumulated = ""
                reflectionEngine.reflect(buildPrompt(text)).collect { token ->
                    accumulated += token
                    journalRepository.updateAssistantBody(reply.id, accumulated)
                }
                _uiState.update { it.copy(isReflecting = false) }
            } catch (e: CancellationException) {
                // stopReflection() or session switch: partial body stays on disk by design.
                _uiState.update { it.copy(isReflecting = false) }
                throw e
            } catch (e: Exception) {
                _uiState.update {
                    it.copy(isReflecting = false, error = com.mj.yata.data.mind.xybridErrorToMindError(e))
                }
            }
        }
        _uiState.update { it.copy(inputDraft = "", isReflecting = true) }
    }

    fun stopReflection() {
        reflectJob?.cancel()
    }

    fun dismissError() {
        _uiState.update { it.copy(error = null) }
    }

    /** Sheet visibility: explicit MODEL_MISSING error, or first visit with nothing on disk. */
    val showDownloadSheet: StateFlow<Boolean> = kotlinx.coroutines.flow.combine(
        _uiState,
        modelDownloader.state
    ) { ui, download ->
        ui.error == MindError.MODEL_MISSING ||
            (!modelDownloader.isModelPresent() &&
                download == com.mj.yata.data.mind.DownloadState.Idle)
    }.stateIn(viewModelScope, SharingStarted.WhileSubscribed(5_000), false)

    private var downloadJob: Job? = null

    fun startDownload() {
        if (downloadJob?.isActive == true) return
        downloadJob = viewModelScope.launch { modelDownloader.download() }
    }

    /** Pause keeps the .part file; next start resumes from byte offset. */
    fun pauseDownload() {
        downloadJob?.cancel()
        _uiState.update { it.copy(error = null) }
    }

    fun dismissDownloadSheet() {
        // Only dismissible when weights exist; otherwise first visit re-offers.
        _uiState.update { it.copy(error = null) }
    }

    fun deleteSession(id: String) {
        viewModelScope.launch {
            try {
                val snapshot = journalRepository.snapshotForRestore(id)
                if (snapshot.isEmpty()) return@launch
                journalRepository.deleteSession(id)
                lastDeletedSnapshot = id to snapshot
                if (sessionIdInternal.value == id) newSession()
                _deletedSessionId.value = id
            } catch (e: CancellationException) {
                throw e
            } catch (e: Exception) {
                _uiState.update { it.copy(error = MindError.STORAGE) }
            }
        }
    }

    fun undoDelete() {
        val snapshot = lastDeletedSnapshot ?: return
        viewModelScope.launch {
            try {
                journalRepository.restoreSession(snapshot.second)
                _deletedSessionId.value = null
                lastDeletedSnapshot = null
            } catch (e: CancellationException) {
                throw e
            } catch (e: Exception) {
                _uiState.update { it.copy(error = MindError.STORAGE) }
            }
        }
    }

    fun dismissDeletedSnackbar() {
        _deletedSessionId.value = null
        lastDeletedSnapshot = null
    }

    /**
     * Minimal persona carrier for Phase 2. The full template (mirror + one
     * question + word budget, with task context) is #27's prompt builder.
     */
    private fun buildPrompt(userText: String): String =
        "You are Muse, a thinking partner. Mirror the user's words, ask exactly ONE " +
            "open question, never give advice, max 100 words.\n\n$userText"
}
