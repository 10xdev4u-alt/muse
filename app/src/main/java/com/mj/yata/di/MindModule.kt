package com.mj.yata.di

import com.mj.yata.data.mind.ModelAcquisition
import com.mj.yata.data.mind.ModelDownloader
import com.mj.yata.data.mind.XybridRuntime
import com.mj.yata.domain.mind.ReflectionEngine
import dagger.Binds
import dagger.Module
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
}
