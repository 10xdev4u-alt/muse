package dev.tenx.muse.data.mind

import dev.tenx.muse.data.local.datastore.UserPreferences
import dev.tenx.muse.domain.mind.ReviewDayStore
import kotlinx.coroutines.flow.first
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class ReviewDayStoreImpl @Inject constructor(
    private val prefs: UserPreferences
) : ReviewDayStore {
    override suspend fun lastReviewDay(): String? = prefs.lastMindReviewDayFlow.first()

    override suspend fun setLastReviewDay(day: String) = prefs.setLastMindReviewDay(day)
}
