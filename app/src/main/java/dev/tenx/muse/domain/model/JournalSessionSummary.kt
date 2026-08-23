package dev.tenx.muse.domain.model

/** One row in the Mind history list: a past session with its first user words as label. */
data class JournalSessionSummary(
    val sessionId: String,
    val startedAt: Long,
    val preview: String?
)
