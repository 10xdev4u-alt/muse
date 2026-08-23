package dev.tenx.muse.notification

import android.app.NotificationManager
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import dev.tenx.muse.data.local.db.AppDatabase
import dev.tenx.muse.data.local.operationhistory.OperationHistoryStore
import dev.tenx.muse.domain.repository.YataRepository
import dev.tenx.muse.data.local.datastore.UserPreferences
import dagger.hilt.android.EntryPointAccessors
import dagger.hilt.InstallIn
import dagger.hilt.components.SingletonComponent
import dagger.hilt.EntryPoint
import kotlinx.coroutines.flow.first
import java.time.LocalDate
import java.time.LocalTime
import java.time.ZoneId

/**
 * Handles notification action buttons:
 *   - "Complete" → marks the task as completed.
 *   - "Snooze 1hr" → reschedules the reminder 1 hour from now.
 */
class NotificationActionReceiver : BroadcastReceiver() {

    @EntryPoint
    @InstallIn(SingletonComponent::class)
    interface ActionReceiverEntryPoint {
        fun appDatabase(): AppDatabase
        fun yataRepository(): YataRepository
        fun reminderScheduler(): ReminderScheduler
        fun userPreferences(): UserPreferences
    }

    companion object {
        private const val TAG = "NotificationActionReceiver"
        const val ACTION_COMPLETE_TASK    = "dev.tenx.muse.ACTION_COMPLETE_TASK"
        const val ACTION_SNOOZE_TASK      = "dev.tenx.muse.ACTION_SNOOZE_TASK"
        const val ACTION_SNOOZE_15M       = "dev.tenx.muse.ACTION_SNOOZE_15M"
        const val ACTION_SNOOZE_TOMORROW  = "dev.tenx.muse.ACTION_SNOOZE_TOMORROW"
        const val EXTRA_TASK_ID        = "EXTRA_TASK_ID"
        const val EXTRA_NOTIFICATION_ID = "EXTRA_NOTIFICATION_ID"
    }

    override fun onReceive(context: Context, intent: Intent) = onReceiveSafely(
        context = context,
        tag = TAG,
        operationId = OperationHistoryStore.REMINDERS_TASK
    ) {
        val taskId = intent.getStringExtra(EXTRA_TASK_ID) ?: return@onReceiveSafely
        val notifId = intent.getIntExtra(EXTRA_NOTIFICATION_ID, taskId.hashCode())

        val entryPoint = EntryPointAccessors.fromApplication(
            context.applicationContext,
            ActionReceiverEntryPoint::class.java
        )
        val db = entryPoint.appDatabase()
        val repository = entryPoint.yataRepository()
        val scheduler = entryPoint.reminderScheduler()
        val userPreferences = entryPoint.userPreferences()
        val notifManager = context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager

        goAsyncSafely(context, TAG, OperationHistoryStore.REMINDERS_TASK) {
            when (intent.action) {
                ACTION_COMPLETE_TASK -> {
                    repository.toggleTaskDone(taskId)
                    notifManager.cancel(notifId)
                }
                ACTION_SNOOZE_TASK -> {
                    val task = db.taskDao().getByIdDirect(taskId)
                    if (task != null) {
                        scheduler.scheduleReminderDelayed(task, 60 * 60 * 1000L) // +1 hour
                        notifManager.cancel(notifId)
                    }
                }
                ACTION_SNOOZE_15M -> {
                    val task = db.taskDao().getByIdDirect(taskId)
                    if (task != null) {
                        scheduler.scheduleReminderDelayed(task, 15 * 60 * 1000L) // +15 min
                        notifManager.cancel(notifId)
                    }
                }
                ACTION_SNOOZE_TOMORROW -> {
                    val task = db.taskDao().getByIdDirect(taskId)
                    if (task != null) {
                        val tomorrowTime = LocalTime.of(
                            userPreferences.snoozeTomorrowHourFlow.first(),
                            userPreferences.snoozeTomorrowMinuteFlow.first()
                        )
                        val tomorrow = LocalDate.now().plusDays(1).atTime(tomorrowTime)
                            .atZone(ZoneId.systemDefault()).toInstant().toEpochMilli()
                        val delay = (tomorrow - System.currentTimeMillis()).coerceAtLeast(0L)
                        scheduler.scheduleReminderDelayed(task, delay)
                        notifManager.cancel(notifId)
                    }
                }
            }
        }
    }
}
