package dev.tenx.muse.data.local.db.dao

import androidx.room.Dao
import androidx.room.Insert
import androidx.room.Query
import dev.tenx.muse.data.local.db.entity.JournalEntryEntity
import kotlinx.coroutines.flow.Flow

/**
 * Plain projection POJO for the Mind session list — no @DatabaseView, so the
 * schema (and its migration story) stays untouched. Room maps the query
 * columns positionally.
 */
data class JournalSessionSummary(
    val sessionId: String,
    val startedAt: Long,
    val preview: String?
)

@Dao
interface JournalSessionDao {

    @Query(
        "SELECT e1.sessionId AS sessionId, " +
            "MIN(e1.createdAt) AS startedAt, " +
            "(SELECT e2.body FROM journal_entries e2 " +
            " WHERE e2.sessionId = e1.sessionId AND e2.role = 'user' " +
            " ORDER BY e2.createdAt ASC LIMIT 1) AS preview " +
            "FROM journal_entries e1 GROUP BY e1.sessionId ORDER BY startedAt DESC"
    )
    fun observeSessionSummaries(): Flow<List<JournalSessionSummary>>

    @Query("SELECT * FROM journal_entries WHERE sessionId = :sessionId ORDER BY createdAt ASC")
    suspend fun entriesForRestore(sessionId: String): List<JournalEntryEntity>

    @Insert
    suspend fun restoreEntries(entries: List<JournalEntryEntity>)
}
