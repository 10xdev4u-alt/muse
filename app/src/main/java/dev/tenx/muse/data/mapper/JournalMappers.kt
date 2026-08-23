package dev.tenx.muse.data.mapper

import dev.tenx.muse.data.local.db.entity.JournalEntryEntity
import dev.tenx.muse.domain.model.JournalEntry
import dev.tenx.muse.domain.model.JournalRole

fun JournalEntryEntity.toDomain(): JournalEntry = JournalEntry(
    id = id,
    sessionId = sessionId,
    role = JournalRole.valueOf(role.uppercase()),
    body = body,
    createdAt = createdAt,
    updatedAt = updatedAt,
    moodTag = moodTag
)

fun JournalEntry.toEntity(): JournalEntryEntity = JournalEntryEntity(
    id = id,
    sessionId = sessionId,
    role = role.name.lowercase(),
    body = body,
    createdAt = createdAt,
    updatedAt = updatedAt,
    moodTag = moodTag
)
