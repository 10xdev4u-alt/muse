package com.mj.yata.domain.mind

import com.mj.yata.domain.model.Task
import java.time.LocalDate
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class MindPromptBuilderTest {

    private val today = LocalDate.parse("2026-08-23")

    private fun task(title: String, due: String?, done: Boolean = false) = Task(
        id = title, title = title, listId = null, projectId = null, section = "",
        due = due, time = null, reminder = null, priority = "none", flag = false,
        done = done, completedAt = if (done) 1L else null, createdAt = 0L, deletedAt = null,
        assigneeIds = emptyList(), tagIds = emptyList(), recurrence = null,
        subtasks = emptyList(), notes = null
    )

    @Test
    fun reflectionPrompt_containsPersona_userText_andContextWhenGiven() {
        val p = MindPromptBuilder.buildReflectionPrompt("I feel scattered")
        assertTrue(p.startsWith(MindPromptBuilder.PERSONA))
        assertTrue(p.endsWith("I feel scattered"))

        val withCtx = MindPromptBuilder.buildReflectionPrompt(
            "I feel scattered",
            MindPromptBuilder.DayContext(listOf("Shipped PR"), listOf("Write docs"), 2)
        )
        assertTrue(withCtx.contains("Completed:\n- Shipped PR"))
        assertTrue(withCtx.contains("Still open:\n- Write docs"))
        assertTrue(withCtx.contains("Overdue (not listed): 2"))
    }

    @Test
    fun reviewPrompt_asksThreeNumberedPrompts() {
        val p = MindPromptBuilder.buildDailyReviewPrompt(
            MindPromptBuilder.DayContext(emptyList(), listOf("A", "B"), 0)
        )
        assertTrue(p.contains("three gentle reflection prompts"))
        assertFalse(p.endsWith("\n"))
    }

    @Test
    fun dayContext_bucketsByDueDate_andCapsListsAtFive() {
        val tasks = (1..7).map { task("pending$it", today.toString()) } +
            (1..6).map { task("done$it", today.toString(), done = true) } +
            listOf(task("old", "2026-08-01"), task("future", "2026-09-30"))

        val ctx = MindPromptBuilder.dayContextFrom(tasks, today)

        // Buckets stay complete; capping is a display concern (formatDayContext).
        assertEquals(6, ctx.completedTitles.size)
        assertEquals(7, ctx.pendingTitles.size)
        // old task counts once; future task is neither pending nor overdue
        assertEquals(1, ctx.overdueCount)

        val formatted = MindPromptBuilder.buildDailyReviewPrompt(ctx)
        assertTrue(formatted.contains("- …and 1 more"))
        assertTrue(formatted.contains("- …and 2 more"))
        assertTrue(formatted.count { it == '-' } <= 12) // 5 + ellipsis line per bucket
    }

    @Test
    fun noTasks_yieldsEmptyContext() {
        val ctx = MindPromptBuilder.dayContextFrom(emptyList(), today)
        assertTrue(ctx.completedTitles.isEmpty() && ctx.pendingTitles.isEmpty() && ctx.overdueCount == 0)
        val prompt = MindPromptBuilder.buildDailyReviewPrompt(ctx)
        assertFalse(prompt.contains("Completed:"))
    }
}
