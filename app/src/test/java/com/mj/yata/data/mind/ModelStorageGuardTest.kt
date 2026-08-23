package com.mj.yata.data.mind

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class ModelStorageGuardTest {

    @Test
    fun requiresArtifactPlusMargin() {
        assertEquals(730_900_000L + 300_000_000L, ModelStorageGuard.requiredBytes())
    }

    @Test
    fun blocksBelowRequirement_allowsAbove() {
        assertFalse(ModelStorageGuard.hasRoom(ModelStorageGuard.requiredBytes() - 1))
        assertTrue(ModelStorageGuard.hasRoom(ModelStorageGuard.requiredBytes()))
        assertTrue(ModelStorageGuard.hasRoom(Long.MAX_VALUE))
    }

    @Test
    fun customModelSize_scalesRequirement() {
        val tiny = 50_000_000L
        assertTrue(ModelStorageGuard.hasRoom(350_000_000L, modelBytes = tiny))
        assertFalse(ModelStorageGuard.hasRoom(300_000_000L, modelBytes = tiny))
    }
}
