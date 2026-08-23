package com.mj.yata.di

import com.mj.yata.data.mind.FakeReflectionEngine
import com.mj.yata.domain.mind.ReflectionEngine
import dagger.Binds
import dagger.Module
import dagger.hilt.InstallIn
import dagger.hilt.components.SingletonComponent
import javax.inject.Singleton

/**
 * Binds the fake engine for Phase 2. Phase 3 (#20) swaps this binding to the
 * real XybridRuntime — one line here, zero UI changes.
 */
@Module
@InstallIn(SingletonComponent::class)
abstract class MindModule {

    @Binds @Singleton
    abstract fun bindReflectionEngine(impl: FakeReflectionEngine): ReflectionEngine
}
