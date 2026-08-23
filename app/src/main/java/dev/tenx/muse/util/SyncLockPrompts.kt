package dev.tenx.muse.util

import android.content.Context
import dev.tenx.muse.R
import dev.tenx.muse.domain.model.BackupDestination
import dev.tenx.muse.domain.model.BackupRunResult
import dev.tenx.muse.domain.model.SyncLockBusyException

fun List<BackupRunResult>.selfHostedSyncLockFailure(): SyncLockBusyException? =
    firstOrNull { it.destination == BackupDestination.SELF_HOSTED }
        ?.error
        ?.syncLockBusyException()

fun Throwable.syncLockBusyException(): SyncLockBusyException? {
    var current: Throwable? = this
    while (current != null) {
        if (current is SyncLockBusyException) return current
        current = current.cause
    }
    return null
}

fun syncLockClearPrompt(context: Context, error: SyncLockBusyException): String {
    val owner = error.lockInfo.ownerDevice?.takeIf { it.isNotBlank() }
    return if (owner != null) {
        context.getString(R.string.settings_clear_sync_lock_confirm_with_owner, owner, error.lockInfo.ageText)
    } else {
        context.getString(R.string.settings_clear_sync_lock_confirm_with_age, error.lockInfo.ageText)
    }
}
