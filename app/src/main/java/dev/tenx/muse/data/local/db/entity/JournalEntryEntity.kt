package dev.tenx.muse.data.local.db.entity

import androidx.room.Entity
import androidx.room.Index
import androidx.room.PrimaryKey

/**
 * One message inside a Mind journaling session. Sessions are chat-shaped:
 * a user entry (what was written) and an assistant entry (the streamed
 * reflection) land as paired rows sharing a sessionId, so streaming can
 * append tokens to the assistant row without rewriting history.
 */
@Entity(
    tableName = "journal_entries",
    indices = [Index("sessionId")]
)
data class JournalEntryEntity(
    @PrimaryKey val id: String,
    val sessionId: String,
    /** "user" or "assistant". Kept as TEXT to avoid a TypeConverter in v32. */
    val role: String,
    val body: String,
    val createdAt: Long,
    val updatedAt: Long,
    /** Optional free-form mood label captured at write time. */
    val moodTag: String? = null
)
