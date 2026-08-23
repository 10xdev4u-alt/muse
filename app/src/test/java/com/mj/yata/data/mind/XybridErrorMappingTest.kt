package com.mj.yata.data.mind

import ai.xybrid.XybridError
import com.mj.yata.domain.mind.MindError
import org.junit.Assert.assertEquals
import org.junit.Test

class XybridErrorMappingTest {

    @Test
    fun modelSideErrors_mapToModelMissing() {
        assertEquals(MindError.MODEL_MISSING, xybridErrorToMindError(XybridError.ModelNotFound("x")))
        assertEquals(MindError.MODEL_MISSING, xybridErrorToMindError(XybridError.DirectoryNotFound("/d")))
        assertEquals(MindError.MODEL_MISSING, xybridErrorToMindError(XybridError.MetadataNotFound("/m")))
        assertEquals(MindError.MODEL_MISSING, xybridErrorToMindError(XybridError.MetadataInvalid("bad")))
    }

    @Test
    fun runtimeErrors_mapToEngine() {
        assertEquals(MindError.ENGINE, xybridErrorToMindError(XybridError.LoadError("oom")))
        assertEquals(MindError.ENGINE, xybridErrorToMindError(XybridError.NotLoaded))
        assertEquals(MindError.ENGINE, xybridErrorToMindError(XybridError.InferenceError("boom")))
        assertEquals(MindError.ENGINE, xybridErrorToMindError(XybridError.StreamingNotSupported))
        assertEquals(MindError.ENGINE, xybridErrorToMindError(RuntimeException("unknown")))
    }

    @Test
    fun networkErrors_mapToOffline_andPrepareHintMapsToModelMissing() {
        assertEquals(MindError.OFFLINE, xybridErrorToMindError(XybridError.Offline("no route")))
        assertEquals(MindError.OFFLINE, xybridErrorToMindError(XybridError.NetworkError("timeout")))
        assertEquals(
            MindError.MODEL_MISSING,
            xybridErrorToMindError(IllegalStateException("Model not downloaded yet: /path"))
        )
    }
}
