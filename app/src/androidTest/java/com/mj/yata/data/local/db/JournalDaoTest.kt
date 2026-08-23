package com.mj.yata.data.local.db

import android.content.Context
import androidx.room.Room
import androidx.test.core.app.ApplicationProvider
import androidx.test.ext.junit.runners.AndroidJUnit4
import com.mj.yata.data.local.db.entity.JournalEntryEntity
import com.mj.yata.data.local.db.dao.JournalDao
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.test.runTest
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith

/**
 * In-memory Room tests for JournalDao. Instrumented (Room 2.6.1 has no JVM
 * driver; Robolectric adoption would be a separate infra decision), so these
 * run on an emulator — exercised together with the migration suite and the
 * Mind smoke test, never against a personal device.
 */
@RunWith(AndroidJUnit4::class)
class JournalDaoTest {

    private val context: Context = ApplicationProvider.getApplicationContext()
    private lateinit var db: AppDatabase
    private lateinit var dao: JournalDao

    @Before
    fun setUp() {
        db = Room.inMemoryDatabaseBuilder(context, AppDatabase::class.java)
            .allowMainThreadQueries()
            .build()
        dao = db.journalDao()
    }

    @After
    fun tearDown() {
        db.close()
    }

    private fun entry(
        id: String,
        sessionId: String,
        role: String,
        createdAt: Long,
        body: String = "body-$id"
    ) = JournalEntryEntity(
        id = id,
        sessionId = sessionId,
        role = role,
        body = body,
        createdAt = createdAt,
        updatedAt = createdAt
    )

    @Test
    fun observeSession_returnsEntriesInInsertionOrder_perSession() = runTest {
        dao.insert(entry("u1", "s1", "user", createdAt = 100))
        dao.insert(entry("a1", "s1", "assistant", createdAt = 101))
        dao.insert(entry("u2", "s2", "user", createdAt = 102))

        val session = dao.observeSession("s1").first()

        assertEquals(listOf("u1", "a1"), session.map { it.id })
    }

    @Test
    fun observeRecentSessions_mostRecentlyActiveFirst_noDuplicates() = runTest {
        dao.insert(entry("u1", "older", "user", createdAt = 100))
        dao.insert(entry("u2", "newer", "user", createdAt = 200))
        dao.insert(entry("u3", "older", "assistant", createdAt = 300))

        val sessions = dao.observeRecentSessions().first()

        // "older" is named by its first entry but its latest entry (300) beats 200.
        assertEquals(listOf("older", "newer"), sessions)
    }

    @Test
    fun updateBody_changesBodyAndUpdatedAt_keepsCreatedAt() = runTest {
        dao.insert(entry("u1", "s1", "assistant", createdAt = 100, body = "partial"))
        val original = dao.observeSession("s1").first().single()

        dao.updateBody("u1", "partial + streamed tokens", updatedAt = 999)

        val updated = dao.observeSession("s1").first().single()
        assertEquals("partial + streamed tokens", updated.body)
        assertEquals(999L, updated.updatedAt)
        assertEquals(original.createdAt, updated.createdAt)
    }

    @Test
    fun deleteSession_removesOnlyTargetSession() = runTest {
        dao.insert(entry("u1", "s1", "user", createdAt = 1))
        dao.insert(entry("a1", "s1", "assistant", createdAt = 2))
        dao.insert(entry("u2", "s2", "user", createdAt = 3))

        dao.deleteSession("s1")

        assertTrue(dao.count() == 1)
        assertEquals(emptyList<String>(), dao.observeSession("s1").first().map { it.id })
        assertEquals("u2", dao.observeRecentSessions().first().single())
    }

    @Test
    fun upsert_sameId_replacesRow() = runTest {
        dao.insert(entry("u1", "s1", "user", createdAt = 1, body = "draft"))
        dao.insert(entry("u1", "s1", "user", createdAt = 1, body = "final"))

        assertEquals(1, dao.count())
        assertEquals("final", dao.observeSession("s1").first().single().body)
    }
}
