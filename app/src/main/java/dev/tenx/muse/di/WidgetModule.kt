package dev.tenx.muse.di

import dev.tenx.muse.widget.WidgetUpdater
import dev.tenx.muse.widget.WidgetUpdaterImpl
import dagger.Binds
import dagger.Module
import dagger.hilt.InstallIn
import dagger.hilt.components.SingletonComponent
import javax.inject.Singleton

@Module
@InstallIn(SingletonComponent::class)
abstract class WidgetModule {

    @Binds @Singleton
    abstract fun bindWidgetUpdater(impl: WidgetUpdaterImpl): WidgetUpdater
}
