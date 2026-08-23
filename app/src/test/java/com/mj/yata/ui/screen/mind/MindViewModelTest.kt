package com.mj.yata.ui.screen.mind

import com.mj.yata.domain.model.JournalEntry
import com.mj.yata.domain.model.JournalRole
import com.mj.yata.domain.mind.MindError
import com.mj.yata.domain.mind.ReflectionEngine
import com.mj.yata.domain.repository.JournalRepository
import java.util.UUID
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.flow
import kotlinx.coroutines.flow.map
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.test.StandardTestDispatcher
import kotlinx.coroutines.test.advanceTimeBy
import kotlinx.coroutines.test.advanceUntilIdle
import kotlinx.coroutines.test.resetMain
import kotlinx.coroutines.test.runCurrent
import kotlinx.coroutines.test.runTest
import kotlinx.coroutines.test.setMain
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test

@OptIn(ExperimentalCoroutinesApi::class)
class MindViewModelTest {

    private val testDispatcher = StandardTestDispatcher()

    private lateinit var repository: FakeJournalRepository
    private lateinit var viewModel: MindViewModel

    @Before
    fun setUp() {
        Dispatchers.setMain(testDispatcher)
    }

    @After
    fun tearDown() {
        Dispatchers.resetMain()
    }

    private fun install(engine: ReflectionEngine): String {
        repository = FakeJournalRepository()
        viewModel = MindViewModel(repository, engine)
        return viewModel.sessionId.value
    }

    /**
     * Streams on virtual time (1s per token) so tests can observe in-flight
     * state between runCurrent/advanceTimeBy steps instead of racing an
     * instant stream.
     */
    private class FakeEngine(
        private val tokens: List<String> = listOf("Hello ", "there."),
        private val msPerToken: Long = 1_000,
        private val failInPrepare: Boolean = false,
        private val throwOnTokenIndex: Int? = null
    ) : ReflectionEngine {
        var prepared = false
            private set

        override fun reflect(prompt: String): Flow<String> = flow {
            tokens.forEachIndexed { i, t ->
                if (i == throwOnTokenIndex) throw RuntimeException("boom")
                kotlinx.coroutines.delay(msPerToken)
                emit(t)
            }
        }

        override suspend fun prepare() {
            prepared = true
            if (failInPrepare) throw RuntimeException("no model")
        }

        override suspend fun release() = Unit
    }

    private class FakeJournalRepository : JournalRepository {
        val store = MutableStateFlow<Map<String, List<JournalEntry>>>(emptyMap())

        override fun observeSession(sessionId: String): Flow<List<JournalEntry>> =
            store.map { rows -> rows[sessionId].orEmpty().sortedBy { it.createdAt } }

        override fun observeRecentSessions(): Flow<List<String>> =
            store.map { rows ->
                rows.entries
                    .groupBy({ it.key }) { s -> s.value.maxOf { it.createdAt } }
                    .map { it.key to (it.value.maxOrNull() ?: 0L) }
                    .sortedByDescending { it.second }
                    .map { it.first }
            }

        override suspend fun appendUserEntry(
            sessionId: String,
            body: String,
            moodTag: String?,
            at: Long
        ): JournalEntry = add(sessionId, JournalRole.USER, body, at)

        override suspend fun beginAssistantReply(
            sessionId: String,
            at: Long
        ): JournalEntry = add(sessionId, JournalRole.ASSISTANT, "", at)

        override suspend fun updateAssistantBody(entryId: String, body: String) {
            store.update { all ->
                all.mapValues { (_, entries) ->
                    entries.map {
                        if (it.id == entryId) it.copy(body = body, updatedAt = body.length.toLong()) else it
                    }
                }
            }
        }

        override suspend fun deleteSession(sessionId: String) {
            store.update { it - sessionId }
        }

        private fun add(
            sessionId: String,
            role: JournalRole,
            body: String,
            at: Long
        ): JournalEntry {
            val entry = JournalEntry(
                id = UUID.randomUUID().toString(),
                sessionId = sessionId,
                role = role,
                body = if (role == JournalRole.USER) body else "",
                createdAt = at,
                updatedAt = at
            )
            store.update { all ->
                all + (sessionId to all.getOrDefault(sessionId, emptyList()) + entry)
            }
            return entry
        }
    }

    private fun assistantBodies(sessionId: String): List<String> =
        repository.store.value[sessionId].orEmpty()
            .filter { it.role == JournalRole.ASSISTANT }
            .map { it.body }

    @Test
    fun send_persistsUserEntry_andStreamsFullAssistantBody() = runTest(testDispatcher) {
        val sid = install(FakeEngine(tokens = listOf("Hello ", "there.")))

        viewModel.onInputChanged("I feel stuck")
        viewModel.send()
        advanceUntilIdle()

        val session = repository.store.value[sid].orEmpty()
        assertEquals(2, session.size)
        assertEquals("I feel stuck", session.first { it.role == JournalRole.USER }.body)
        assertEquals("Hello there.", assistantBodies(sid).single())
        assertFalse(viewModel.uiState.value.isReflecting)
        assertNull(viewModel.uiState.value.error)
    }

    @Test
    fun send_clearsInputImmediately_andBlocksWhileReflecting() = runTest(testDispatcher) {
        install(FakeEngine())

        viewModel.onInputChanged("first thought")
        viewModel.send()
        runCurrent() // user entry persisted, assistant row reserved, first token still pending

        assertEquals("", viewModel.uiState.value.inputDraft)
        assertTrue(viewModel.uiState.value.isReflecting)

        viewModel.onInputChanged("second while busy")
        viewModel.send()
        advanceUntilIdle()

        // Second send was rejected while reflecting; exactly one exchange exists.
        assertEquals(1, assistantBodies(viewModel.sessionId.value).size)
    }

    @Test
    fun midStreamFailure_setsError_andKeepsPartialOutput() = runTest(testDispatcher) {
        val sid = install(
            FakeEngine(tokens = listOf("one ", "two ", "three "), throwOnTokenIndex = 2)
        )

        viewModel.onInputChanged("tell me")
        viewModel.send()
        advanceUntilIdle()

        assertEquals(MindError.ENGINE, viewModel.uiState.value.error)
        assertFalse(viewModel.uiState.value.isReflecting)
        assertEquals("one two ", assistantBodies(sid).single())
    }

    @Test
    fun prepareFailure_setsError_andLeavesNoAssistantRow() = runTest(testDispatcher) {
        val sid = install(FakeEngine(failInPrepare = true))

        viewModel.onInputChanged("tell me")
        viewModel.send()
        advanceUntilIdle()

        assertEquals(MindError.ENGINE, viewModel.uiState.value.error)
        assertFalse(viewModel.uiState.value.isReflecting)
        assertEquals(1, repository.store.value[sid].orEmpty().size) // user entry only
    }

    @Test
    fun stopReflection_cancelsStream_partialBodyStays() = runTest(testDispatcher) {
        val sid = install(FakeEngine(tokens = (1..50).map { "tok$it " }))

        viewModel.onInputChanged("long one")
        viewModel.send()
        runCurrent()
        advanceTimeBy(2_500) // exactly two tokens emitted
        runCurrent()
        viewModel.stopReflection()
        advanceUntilIdle()

        assertFalse(viewModel.uiState.value.isReflecting)
        val bodies = assistantBodies(sid)
        assertEquals(1, bodies.size)
        assertEquals("tok1 tok2 ", bodies.single()) // stopped well before 50 tokens
    }

    @Test
    fun newSession_switchesContext_emptyEntries() = runTest(testDispatcher) {
        val oldSid = install(FakeEngine())
        viewModel.onInputChanged("old session thought")
        viewModel.send()
        advanceUntilIdle()

        viewModel.newSession()
        runCurrent()

        assertTrue(viewModel.sessionId.value != oldSid)
        // Old session data is untouched on disk; the view simply points elsewhere now.
        assertEquals(2, repository.store.value[oldSid]!!.size)
    }
}
