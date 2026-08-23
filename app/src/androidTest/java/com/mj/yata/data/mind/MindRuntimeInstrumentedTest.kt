package com.mj.yata.data.mind

import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import java.io.File
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith

/**
 * Device-side checks for the Mind runtime pieces that need a real filesystem.
 * Full UI smoke (open tab -> fake-engine reflection renders -> entry saved)
 * additionally needs either downloaded weights or a Hilt test-binding swap;
 * that extension is tracked on issue #25's follow-up note.
 */
@RunWith(AndroidJUnit4::class)
class MindRuntimeInstrumentedTest {

    private val context = InstrumentationRegistry.getInstrumentation().targetContext

    @Test
    fun freshInstall_reportsModelAbsent() {
        val dir = File(context.filesDir, "xybrid/${XybridRuntime.MODEL_ID}")
        dir.deleteRecursively()
        val downloader = ModelDownloader(context)
        assertFalse(downloader.isModelPresent())
    }

    @Test
    fun metadataPlusGguf_reportsModelPresent() {
        val dir = File(context.filesDir, "xybrid/${XybridRuntime.MODEL_ID}").apply { mkdirs() }
        File(dir, "model_metadata.json").writeText("{}")
        File(dir, ModelDownloader.GGUF_NAME).writeBytes(byteArrayOf(1))
        try {
            assertTrue(ModelDownloader(context).isModelPresent())
        } finally {
            dir.deleteRecursively()
        }
    }

    @Test
    fun storageGuard_seesRealFreeSpace() {
        assertTrue(ModelStorageGuard.availableBytes(context.filesDir) > 0)
    }
}
