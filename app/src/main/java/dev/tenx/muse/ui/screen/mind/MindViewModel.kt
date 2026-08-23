package dev.tenx.muse.ui.screen.mind

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import dev.tenx.muse.domain.mind.MindError
import dev.tenx.muse.domain.mind.ReviewDayStore
import dev.tenx.muse.domain.mind.ReflectionEngine
import dev.tenx.muse.domain.repository.JournalRepository
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
import kotlinx.coroutines.flow.first
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
    val modelDownloader: dev.tenx.muse.data.mind.ModelAcquisition,
    private val tasksProvider: dev.tenx.muse.domain.mind.TasksProvider,
    private val reviewPrefs: ReviewDayStore
) : ViewModel() {

    private val sessionIdInternal = MutableStateFlow(UUID.randomUUID().toString())

    /** Open session id. Public because the screen shows session identity and tests assert on it. */
    val sessionId: StateFlow<String> = sessionIdInternal.asStateFlow()

    private val _uiState = MutableStateFlow(MindUiState())
    val uiState: StateFlow<MindUiState> = _uiState.asStateFlow()

    /** History rows for the session picker. */
    val sessionSummaries: StateFlow<List<dev.tenx.muse.domain.model.JournalSessionSummary>> =
        journalRepository.observeSessionSummaries()
            .stateIn(viewModelScope, SharingStarted.WhileSubscribed(5_000), emptyList())

    /** Session id awaiting undo, surfaced by the screen as a snackbar action. */
    private val _deletedSessionId = MutableStateFlow<String?>(null)
    val deletedSessionId: StateFlow<String?> = _deletedSessionId.asStateFlow()

    /** Entries of the open session, straight from Room — screen text IS database text. */
    val entries: StateFlow<List<dev.tenx.muse.domain.model.JournalEntry>> = sessionId
        .flatMapLatest { journalRepository.observeSession(it) }
        .stateIn(viewModelScope, SharingStarted.WhileSubscribed(5_000), emptyList())

    private var reflectJob: Job? = null
    private var lastDeletedSnapshot: Pair<String, List<dev.tenx.muse.domain.model.JournalEntry>>? = null

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
                    it.copy(isReflecting = false, error = dev.tenx.muse.data.mind.xybridErrorToMindError(e))
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

    /** True while the once-a-day review is generating. */
    private val _isReviewing = MutableStateFlow(false)
    val isReviewing: StateFlow<Boolean> = _isReviewing.asStateFlow()

    /**
     * Runs the grounded daily review at most once per calendar day (#28/#29):
     * buckets today's real tasks, streams three numbered reflection prompts,
     * and persists them as an assistant entry in a dedicated per-day session
     * so history groups reviews like any other exchange.
     */
    fun maybeRunDailyReview(today: java.time.LocalDate = java.time.LocalDate.now()) {
        viewModelScope.launch {
            try {
                val todayStr = today.toString()
                if (reviewPrefs.lastReviewDay() == todayStr) return@launch

                val dayContext = dev.tenx.muse.domain.mind.MindPromptBuilder
                    .dayContextFrom(tasksProvider.all(), today)
                if (dayContext.completedTitles.isEmpty() &&
                    dayContext.pendingTitles.isEmpty() && dayContext.overdueCount == 0
                ) {
                    // Nothing to reflect on yet; retry on a later visit today.
                    return@launch
                }

                _isReviewing.value = true
                val reviewSessionId = "daily-review-$todayStr"
                sessionIdInternal.value = reviewSessionId

                val reply = journalRepository.beginAssistantReply(reviewSessionId)
                var accumulated = ""
                reflectionEngine
                    .reflect(
                        dev.tenx.muse.domain.mind.MindPromptBuilder.buildDailyReviewPrompt(dayContext)
                    )
                    .collect { token ->
                        accumulated += token
                        journalRepository.updateAssistantBody(reply.id, accumulated)
                    }
                reviewPrefs.setLastReviewDay(todayStr)
            } catch (e: CancellationException) {
                throw e
            } catch (e: Exception) {
                _uiState.update { it.copy(error = MindError.ENGINE) }
            } finally {
                _isReviewing.value = false
            }
        }
    }

    /**
     * Dialog visibility across the whole acquisition lifecycle (#75):
     * active downloads and failures are ALWAYS visible; the first-run offer
     * can be dismissed but returns until weights exist; Ready stays up until
     * acknowledged so completion is never missed.
     */
    val showDownloadSheet: StateFlow<Boolean> = kotlinx.coroutines.flow.combine(
        _uiState,
        modelDownloader.state
    ) { ui, download ->
        when {
            ui.error == MindError.MODEL_MISSING -> true
            download is dev.tenx.muse.data.mind.DownloadState.Downloading -> true
            download is dev.tenx.muse.data.mind.DownloadState.Failed -> true
            download == dev.tenx.muse.data.mind.DownloadState.Ready -> !readyAcknowledged
            !modelDownloader.isModelPresent() && !offerDismissed -> true
            else -> false
        }
    }.stateIn(viewModelScope, SharingStarted.WhileSubscribed(5_000), false)

    private var downloadJob: Job? = null
    private var offerDismissed = false
    private var readyAcknowledged = false

    fun startDownload() {
        offerDismissed = false
        readyAcknowledged = false
        if (downloadJob?.isActive == true) return
        downloadJob = viewModelScope.launch { modelDownloader.download() }
    }

    /** Pause keeps the .part file; next start resumes from byte offset. */
    fun pauseDownload() {
        downloadJob?.cancel()
    }

    fun dismissDownloadSheet() {
        val dl = modelDownloader.state.value
        if (dl is dev.tenx.muse.data.mind.DownloadState.Downloading ||
            dl is dev.tenx.muse.data.mind.DownloadState.Failed
        ) return // active downloads and failures cannot be dismissed away
        offerDismissed = true
        readyAcknowledged = modelDownloader.isModelPresent()
        _uiState.update { it.copy(error = null) }
    }

    /** 'Start reflecting': acknowledge completion and warm-load for an instant first send. */
    fun acknowledgeReady() {
        readyAcknowledged = true
        viewModelScope.launch {
            try {
                reflectionEngine.prepare()
            } catch (e: Exception) {
                _uiState.update { it.copy(error = dev.tenx.muse.data.mind.xybridErrorToMindError(e)) }
            }
        }
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
        dev.tenx.muse.domain.mind.MindPromptBuilder.PERSONA + "\n\n$userText"
}
