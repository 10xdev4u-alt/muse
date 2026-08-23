package dev.tenx.muse.domain.repository

import dev.tenx.muse.domain.model.JournalEntry
import kotlinx.coroutines.flow.Flow

/**
 * Journal sessions for the Mind tab. A session is a chat-shaped exchange:
 * user entries hold what was written, assistant entries hold the streamed
 * reflection. Callers start an assistant reply to reserve its row, then
 * push body updates as tokens stream in.
 */
interface JournalRepository {
    fun observeSession(sessionId: String): Flow<List<JournalEntry>>

    /** One row per session, most recently started first. */
    fun observeSessionSummaries(): Flow<List<dev.tenx.muse.domain.model.JournalSessionSummary>>

    /** Rows of a session, for delete-with-undo. */
    suspend fun snapshotForRestore(sessionId: String): List<JournalEntry>

    /** Puts back a snapshot from [snapshotForRestore]. */
    suspend fun restoreSession(entries: List<JournalEntry>)

    fun observeRecentSessions(): Flow<List<String>>

    suspend fun appendUserEntry(
        sessionId: String,
        body: String,
        moodTag: String? = null,
        at: Long = System.currentTimeMillis()
    ): JournalEntry

    /** Reserves the assistant row with an empty body; returns the created entry. */
    suspend fun beginAssistantReply(sessionId: String, at: Long = System.currentTimeMillis()): JournalEntry

    /** Streams and edits land here — replaces the whole body, bumps updatedAt only. */
    suspend fun updateAssistantBody(entryId: String, body: String)

    suspend fun deleteSession(sessionId: String)
}
