package com.mj.yata.data.mind

import ai.xybrid.XybridError
import com.mj.yata.domain.mind.MindError

/**
 * Exhaustive translation of the SDK's sealed error hierarchy into
 * user-facing categories. Lives in the only module that knows about
 * ai.xybrid types (ADR-004); the ViewModel sees just [MindError].
 */
fun xybridErrorToMindError(t: Throwable): MindError = when (t) {
    is XybridError.ModelNotFound,
    is XybridError.DirectoryNotFound,
    is XybridError.MetadataNotFound,
    is XybridError.MetadataInvalid -> MindError.MODEL_MISSING

    is XybridError.LoadError,
    is XybridError.NotLoaded,
    is XybridError.ConfigError,
    is XybridError.InferenceError,
    is XybridError.StreamingNotSupported,
    is XybridError.AbortedForCloudFallback -> MindError.ENGINE

    is XybridError.NetworkError,
    is XybridError.Offline -> MindError.OFFLINE

    else ->
        if (t.message?.contains("not downloaded") == true) MindError.MODEL_MISSING
        else MindError.ENGINE
}
