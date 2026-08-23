package com.mj.yata.data.repository

import com.mj.yata.data.local.db.AppDatabase
import com.mj.yata.data.mapper.toDomain
import com.mj.yata.data.mapper.toEntity
import com.mj.yata.domain.model.JournalEntry
import com.mj.yata.domain.model.JournalRole
import com.mj.yata.domain.repository.JournalRepository
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.map
import kotlinx.coroutines.withContext
import java.util.UUID
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class JournalRepositoryImpl @Inject constructor(
    private val db: AppDatabase
) : JournalRepository {

    private val dao get() = db.journalDao()

    override fun observeSession(sessionId: String): Flow<List<JournalEntry>> =
        dao.observeSession(sessionId).map { rows -> rows.map { it.toDomain() } }

    override fun observeRecentSessions(): Flow<List<String>> =
        dao.observeRecentSessions()

    override fun observeSessionSummaries(): Flow<List<com.mj.yata.domain.model.JournalSessionSummary>> =
        db.journalSessionDao().observeSessionSummaries().map { rows ->
            rows.map { com.mj.yata.domain.model.JournalSessionSummary(it.sessionId, it.startedAt, it.preview) }
        }

    override suspend fun snapshotForRestore(sessionId: String): List<JournalEntry> =
        withContext(Dispatchers.IO) {
            db.journalSessionDao().entriesForRestore(sessionId).map { it.toDomain() }
        }

    override suspend fun restoreSession(entries: List<JournalEntry>) =
        withContext(Dispatchers.IO) {
            db.journalSessionDao().restoreEntries(entries.map { it.toEntity() })
        }

    override suspend fun appendUserEntry(
        sessionId: String,
        body: String,
        moodTag: String?,
        at: Long
    ): JournalEntry = withContext(Dispatchers.IO) {
        val entry = JournalEntry(
            id = UUID.randomUUID().toString(),
            sessionId = sessionId,
            role = JournalRole.USER,
            body = body,
            createdAt = at,
            updatedAt = at,
            moodTag = moodTag
        )
        dao.insert(entry.toEntity())
        entry
    }

    override suspend fun beginAssistantReply(
        sessionId: String,
        at: Long
    ): JournalEntry = withContext(Dispatchers.IO) {
        val entry = JournalEntry(
            id = UUID.randomUUID().toString(),
            sessionId = sessionId,
            role = JournalRole.ASSISTANT,
            body = "",
            createdAt = at,
            updatedAt = at
        )
        dao.insert(entry.toEntity())
        entry
    }

    override suspend fun updateAssistantBody(entryId: String, body: String) =
        withContext(Dispatchers.IO) {
            dao.updateBody(entryId, body, System.currentTimeMillis())
        }

    override suspend fun deleteSession(sessionId: String) = withContext(Dispatchers.IO) {
        dao.deleteSession(sessionId)
    }
}
