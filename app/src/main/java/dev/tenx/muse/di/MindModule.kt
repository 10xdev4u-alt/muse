package dev.tenx.muse.di

import dev.tenx.muse.data.mind.ModelAcquisition
import dev.tenx.muse.data.mind.ModelDownloader
import dev.tenx.muse.data.mind.XybridRuntime
import dev.tenx.muse.domain.mind.ReflectionEngine
import dev.tenx.muse.domain.mind.ReviewDayStore
import dev.tenx.muse.domain.repository.YataRepository
import dagger.Binds
import dagger.Module
import dagger.Provides
import kotlinx.coroutines.flow.first
import dagger.hilt.InstallIn
import dagger.hilt.components.SingletonComponent
import javax.inject.Singleton

/**
 * Binds the REAL runtime as of #20. The fake lives on at
 * data/mind/FakeReflectionEngine.kt for tests and emulator-only UI work —
 * swap this single binding back to use it.
 *
 * NOTE: Mind tab stays flag-OFF in UserPreferences until the downloader
 * (#21) ships; prepare() throws a clear "model not downloaded" if flipped
 * on early, which surfaces as an ENGINE error rather than a hang.
 */
@Module
@InstallIn(SingletonComponent::class)
abstract class MindModule {

    @Binds @Singleton
    abstract fun bindReflectionEngine(impl: XybridRuntime): ReflectionEngine

    @Binds @Singleton
    abstract fun bindModelAcquisition(impl: ModelDownloader): ModelAcquisition

    @Binds @Singleton
    abstract fun bindReviewDayStore(impl: dev.tenx.muse.data.mind.ReviewDayStoreImpl): ReviewDayStore

    companion object {
        /** Mind reads today's reality through one narrow seam, not the whole repo. */
        @Provides
        fun provideTasksProvider(repo: YataRepository): dev.tenx.muse.domain.mind.TasksProvider =
            dev.tenx.muse.domain.mind.TasksProvider { repo.getTasks().first() }
    }
}
