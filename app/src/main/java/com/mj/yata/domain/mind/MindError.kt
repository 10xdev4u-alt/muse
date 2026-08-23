package com.mj.yata.domain.mind

/** User-facing failure categories for the Mind tab. Extended by #22's SDK error mapping. */
enum class MindError {
    /** Reflection generation failed mid-turn. Partial output is kept. */
    ENGINE,

    /** Journal persistence failed. */
    STORAGE
}
