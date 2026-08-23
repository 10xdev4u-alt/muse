package dev.tenx.muse.domain.model

enum class JournalRole { USER, ASSISTANT }

data class JournalEntry(
    val id: String,
    val sessionId: String,
    val role: JournalRole,
    val body: String,
    val createdAt: Long,
    val updatedAt: Long,
    val moodTag: String? = null
)
