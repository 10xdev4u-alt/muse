package com.mj.yata.domain.mind

/** Narrow seam for Mind to read today's reality without the whole task repo. */
fun interface TasksProvider {
    suspend fun all(): List<com.mj.yata.domain.model.Task>
}
