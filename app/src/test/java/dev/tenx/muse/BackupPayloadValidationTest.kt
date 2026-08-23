package dev.tenx.muse

import dev.tenx.muse.util.CURRENT_BACKUP_VERSION
import dev.tenx.muse.util.isRecognizedBackup
import org.json.JSONArray
import org.json.JSONObject
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class BackupPayloadValidationTest {

    @Test
    fun arbitraryJsonIsRejected() {
        assertFalse(isRecognizedBackup(JSONObject()))
    }

    @Test
    fun tasksMustBeAnArray() {
        val root = JSONObject()
            .put("version", CURRENT_BACKUP_VERSION)
            .put("tasks", "corrupt")

        assertFalse(isRecognizedBackup(root))
    }

    @Test
    fun emptyDatabaseBackupIsStillValid() {
        val root = JSONObject()
            .put("version", CURRENT_BACKUP_VERSION)
            .put("tasks", JSONArray())

        assertTrue(isRecognizedBackup(root))
    }

    @Test
    fun futureFormatIsRejectedUntilSupported() {
        val root = JSONObject()
            .put("version", CURRENT_BACKUP_VERSION + 1)
            .put("tasks", JSONArray())

        assertFalse(isRecognizedBackup(root))
    }
}
