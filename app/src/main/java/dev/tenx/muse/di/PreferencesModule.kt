package dev.tenx.muse.di

import dev.tenx.muse.data.local.datastore.TaskListPreferences
import dev.tenx.muse.data.local.datastore.UserPreferences
import dagger.Binds
import dagger.Module
import dagger.hilt.InstallIn
import dagger.hilt.components.SingletonComponent
import javax.inject.Singleton

@Module
@InstallIn(SingletonComponent::class)
abstract class PreferencesModule {

    @Binds
    @Singleton
    abstract fun bindTaskListPreferences(impl: UserPreferences): TaskListPreferences
}
