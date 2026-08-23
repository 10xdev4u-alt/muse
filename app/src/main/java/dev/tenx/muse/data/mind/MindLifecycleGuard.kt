package dev.tenx.muse.data.mind

import androidx.lifecycle.DefaultLifecycleObserver
import androidx.lifecycle.LifecycleOwner
import dev.tenx.muse.domain.mind.ReflectionEngine
import java.util.concurrent.atomic.AtomicReference
import javax.inject.Inject
import javax.inject.Singleton
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch

/**
 * Unloads the model after [UNLOAD_DELAY_MS] in the background so a journaling
 * app never holds ~1 GB of RAM while the user answers a message. A quick
 * notification check cancels the pending unload instead of paying reload cost;
 * genuinely backgrounded sessions come back to a fresh prepare().
 *
 * Context-length cap (2048) lives in model_metadata.json; llama.cpp thread
 * count stays at backend default until device profiling says otherwise (#25 run).
 */
@Singleton
class MindLifecycleGuard @Inject constructor(
    private val engine: ReflectionEngine
) : DefaultLifecycleObserver {

    private val scope = CoroutineScope(SupervisorJob() + Dispatchers.Default)
    private val pendingRelease = AtomicReference<Job?>(null)

    /** Registers against the process lifecycle; call once from Application.onCreate. */
    fun registerWithProcessLifecycle() {
        androidx.lifecycle.ProcessLifecycleOwner.get().lifecycle.addObserver(this)
    }

    override fun onStop(owner: LifecycleOwner) {
        pendingRelease.getAndSet(
            scope.launch {
                delay(UNLOAD_DELAY_MS)
                engine.release()
            }
        )?.cancel()
    }

    override fun onStart(owner: LifecycleOwner) {
        pendingRelease.getAndSet(null)?.cancel()
    }

    companion object {
        const val UNLOAD_DELAY_MS = 60_000L
    }
}
