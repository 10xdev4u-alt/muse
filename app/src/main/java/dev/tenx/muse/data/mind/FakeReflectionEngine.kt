package dev.tenx.muse.data.mind

import dev.tenx.muse.domain.mind.ReflectionEngine
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flow
import javax.inject.Inject
import javax.inject.Singleton

/**
 * Streams a canned reflection token-by-token so the Mind UI is fully buildable,
 * demoable, and unit-testable with zero model bytes on disk. The persona text
 * doubles as the acceptance sample for prompt iteration (#27).
 */
@Singleton
class FakeReflectionEngine @Inject constructor() : ReflectionEngine {

    private val cannedReply = listOf(
        "You've ", "been ", "circling ", "this ", "for ", "a ", "while. ",
        "The ", "idea ", "clearly ", "matters ", "to ", "you — ",
        "so ", "what's ", "the ", "smallest ", "version ", "of ", "it ",
        "you ", "could ", "face ", "tomorrow ", "morning?"
    )

    override fun reflect(prompt: String): Flow<String> = flow {
        for (token in cannedReply) {
            delay(30)
            emit(token)
        }
    }

    override suspend fun prepare() {
        delay(50) // simulate model load
    }

    override suspend fun release() {
        // nothing held
    }
}
