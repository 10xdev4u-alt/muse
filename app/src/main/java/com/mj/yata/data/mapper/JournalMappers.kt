package com.mj.yata.data.mapper

import com.mj.yata.data.local.db.entity.JournalEntryEntity
import com.mj.yata.domain.model.JournalEntry
import com.mj.yata.domain.model.JournalRole

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
