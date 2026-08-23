package dev.tenx.muse.di

import dev.tenx.muse.data.demo.RoutingYataRepository
import dev.tenx.muse.data.repository.JournalRepositoryImpl
import dev.tenx.muse.domain.repository.JournalRepository
import dev.tenx.muse.domain.repository.YataRepository
import dev.tenx.muse.notification.ReminderScheduler
import dev.tenx.muse.notification.TaskReminderScheduler
import dagger.Binds
import dagger.Module
import dagger.hilt.InstallIn
import dagger.hilt.components.SingletonComponent
import javax.inject.Singleton

@Module
@InstallIn(SingletonComponent::class)
abstract class RepositoryModule {

    @Binds @Singleton
    abstract fun bindYataRepository(impl: RoutingYataRepository): YataRepository

    @Binds @Singleton
    abstract fun bindJournalRepository(impl: JournalRepositoryImpl): JournalRepository

    @Binds @Singleton
    abstract fun bindTaskReminderScheduler(impl: ReminderScheduler): TaskReminderScheduler
}
