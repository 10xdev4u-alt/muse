package dev.tenx.muse.domain.mind

import dev.tenx.muse.domain.model.Task
import java.time.LocalDate

/**
 * Builds the two prompts Mind uses, with the persona contract in one place.
 * Task context is a compact budgeted block — titles only, capped counts — so
 * a 2048-token context stays dominated by the user's actual writing.
 */
object MindPromptBuilder {

    const val PERSONA =
        "You are Muse, a developer companion. Answer directly and concretely: code, steps, or facts. Never ask questions, never lecture. Max 120 words."

    data class DayContext(
        val completedTitles: List<String>,
        val pendingTitles: List<String>,
        val overdueCount: Int
    )

    fun buildReflectionPrompt(userText: String, day: DayContext? = null): String {
        val ctx = day?.let { formatDayContext(it) }?.let { "\n\nContext — today's tasks:\n$it" } ?: ""
        return "$PERSONA$ctx\n\n$userText"
    }

    fun buildDailyReviewPrompt(day: DayContext): String {
        val ctx = formatDayContext(day)
        return "$PERSONA\n\nContext — today's tasks:\n$ctx\n\n" +
            "Offer three gentle reflection prompts about how today went. Number them. " +
            "Keep each under 25 words."
    }

    /** Titles only, 5 per bucket; overdue collapses to a count. */
    private fun formatDayContext(day: DayContext): String = buildString {
        if (day.completedTitles.isNotEmpty()) {
            appendLine("Completed:")
            day.completedTitles.take(5).forEach { appendLine("- $it") }
            if (day.completedTitles.size > 5) appendLine("- …and ${day.completedTitles.size - 5} more")
        }
        if (day.pendingTitles.isNotEmpty()) {
            appendLine("Still open:")
            day.pendingTitles.take(5).forEach { appendLine("- $it") }
            if (day.pendingTitles.size > 5) appendLine("- …and ${day.pendingTitles.size - 5} more")
        }
        if (day.overdueCount > 0) append("Overdue (not listed): ${day.overdueCount}")
    }.trim()

    /** Projects the live task list into today's three buckets. */
    fun dayContextFrom(tasks: List<Task>, today: LocalDate = LocalDate.now()): DayContext {
        val todayStr = today.toString()
        val active = tasks.filter { !it.done && it.due != null && it.due < todayStr }
        val dueToday = tasks.filter { it.due == todayStr }
        return DayContext(
            completedTitles = dueToday.filter { it.done }.map { it.title },
            pendingTitles = dueToday.filter { !it.done }.map { it.title },
            overdueCount = active.size
        )
    }
}
