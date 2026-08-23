package dev.tenx.muse.domain.mind

/** Narrow seam for Mind to read today's reality without the whole task repo. */
fun interface TasksProvider {
    suspend fun all(): List<dev.tenx.muse.domain.model.Task>
}
