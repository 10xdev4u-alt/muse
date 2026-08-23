package dev.tenx.muse.util

import dev.tenx.muse.domain.model.Task

/** Used by the scoped search box on Project/List/Tag/Person detail screens — matches
 * within a task's own text fields only, not tag/assignee names, since those screens
 * are already scoped by tag/assignee. */
fun taskMatchesQuery(task: Task, query: String): Boolean {
    if (query.isBlank()) return true
    return task.title.contains(query, ignoreCase = true) ||
        task.notes?.contains(query, ignoreCase = true) == true ||
        task.subtasks.any { it.title.contains(query, ignoreCase = true) }
}
