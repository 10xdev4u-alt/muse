package com.mj.yata.domain.mind

import kotlinx.coroutines.flow.Flow

/**
 * The seam between the Mind UI and whatever generates reflections. The only
 * implementation allowed to touch the Xybrid SDK is the real one (Phase 3);
 * tests and UI development run on [com.mj.yata.data.mind.FakeReflectionEngine].
 *
 * Cancellation contract: cancelling collection of [reflect] stops generation —
 * implementors must propagate cancellation to the backend, not run to completion.
 */
interface ReflectionEngine {
    /** Cold stream of reflection tokens for one user turn. */
    fun reflect(prompt: String): Flow<String>

    /**
     * Warms the backend (model load) so the first token arrives fast.
     * Safe to call repeatedly; cheap once prepared. Returns when ready or throws.
     */
    suspend fun prepare()

    /** Frees RAM. Cheap once already released; safe to call repeatedly. */
    suspend fun release()
}
