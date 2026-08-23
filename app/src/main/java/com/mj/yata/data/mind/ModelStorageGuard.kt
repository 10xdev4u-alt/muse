package com.mj.yata.data.mind

import java.io.File

/**
 * Pre-download disk check. Pure math so it tests on the JVM; callers hand us
 * what StatFs reported. Margin covers extraction scratch and DB growth so a
 * "successful" download can never wedge the device at 100% full.
 */
object ModelStorageGuard {
    /** Q4_K_M GGUF size from models/lfm2.5-1.2b-instruct/README.md provenance. */
    const val MODEL_BYTES: Long = 730_900_000L

    /** Headroom beyond the artifact itself. */
    const val MARGIN_BYTES: Long = 300_000_000L

    fun requiredBytes(modelBytes: Long = MODEL_BYTES): Long = modelBytes + MARGIN_BYTES

    fun hasRoom(availableBytes: Long, modelBytes: Long = MODEL_BYTES): Boolean =
        availableBytes >= requiredBytes(modelBytes)

    fun availableBytes(dir: File): Long = dir.usableSpace
}
