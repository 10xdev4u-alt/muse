# Research: xybrid-kotlin 0.5.0 API audit vs Muse requirements

Sources: `bindings/kotlin/src/main/kotlin/ai/xybrid/Xybrid.kt` and
`XybridBolt.kt` at xybrid-ai/xybrid master, plus docs.xybrid.dev Kotlin pages.

| Muse need | SDK API found | Verdict |
|---|---|---|
| Load model from our directory layout | `ModelSource.Directory(path)`, `XybridModelLoader.fromDirectory(path)`, suspending `load()` | present |
| Download from HuggingFace (fallback path) | `ModelSource.huggingFace(repo)`, `fromHuggingfaceAsync(repo)` | present |
| Streaming reflection tokens | `model.streamTokens(envelope, options): Flow<XybridStreamToken>` — pull-based session wrapped as Flow with cooperative cancellation; cancelling collection aborts generation and closes the session | present |
| Unload on background (RAM guardrail) | `model.unloadAsync()` | present |
| First-token latency mitigation | `model.warmupAsync()` | present |
| Persona tuning (short answers, low temp) | `XybridGenerationConfig(maxTokens, temperature, topP, minP, topK, repetitionPenalty, stopSequences, grammar, tools)` via `XybridRunOptions` | present |
| Result data | `XybridResult(envelope, outputType, modelId, latencyMs, executionTarget, metrics, toolCalls, reasoningContent)` | present |
| Typed error handling | sealed `XybridError` variants (alias `XybridException`) | present |
| Battery/thermal routing context | `Xybrid.init(context)` auto-registers battery receiver + thermal observers | present |

## Gaps and notes

- No built-in download manager with progress/resume. Our downloader (#19) is real
  work: WorkManager + resumable HTTP + SHA256 verify against the HF artifact.
- `reasoningContent` exists on results; irrelevant for LFM2.5-Instruct (non-thinking)
  but noted for a possible future Thinking-variant toggle.
- Tool calling ships in the wire types (`XybridToolDefinition`) though README marks
  Android tool calling 🔜 — do not build on it until it flips.

## Consequence

No SDK blocker. ADR-004 pins these APIs as Muse's dependency surface.
