package com.mj.yata.domain.mind

/** Once-a-day marker for the Mind daily review. */
interface ReviewDayStore {
    suspend fun lastReviewDay(): String?
    suspend fun setLastReviewDay(day: String)
}
