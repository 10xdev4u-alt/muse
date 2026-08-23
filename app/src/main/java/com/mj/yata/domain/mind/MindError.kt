package com.mj.yata.domain.mind

/** User-facing failure categories for the Mind tab. */
enum class MindError {
    /** Reflection generation failed mid-turn. Partial output is kept. */
    ENGINE,

    /** Weights absent or unreadable — route the user to the download flow. */
    MODEL_MISSING,

    /** No connectivity for a network-dependent step. */
    OFFLINE,

    /** Journal persistence failed. */
    STORAGE
}
