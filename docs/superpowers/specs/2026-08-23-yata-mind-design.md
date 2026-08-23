# Muse — Offline AI Thinking Partner

> Originally specced as "Muse" before the project was renamed Muse.
> File name kept for history.

**Date:** 2026-08-23
**Status:** Approved direction, pending implementation plan
**Base:** YATA (`dev.tenx.muse (rename pending)`) + Xybrid Kotlin SDK (`ai.xybrid:xybrid-kotlin:0.5.0`)

## 1. Product Vision

Muse turns YATA from a task manager into a **thinking environment**. You pour out your
thoughts in plain language; a real LLM running entirely on your phone reflects them back —
asking one good question at a time, mirroring your words, never lecturing. Because it lives
inside the task app, it also knows what your day actually looked like (tasks completed,
postponed, overdue) and grounds its daily review in that reality.

**One-liner:** *"A journal that talks back — 100% offline, inside your todo app."*

### Non-goals
- No cloud inference, ever. Airplane mode is the happy path.
- No advice-column persona by default; no motivational-poster tone.
- No voice I/O in v1 (text-only); YATA's `OnDeviceVoiceRecognizer` is the future path.
- No modification of Xybrid itself; we are a consumer of the published SDK.

## 2. Architecture

```
┌──────────────────────────────────────────────────────────┐
│ dev.tenx.muse (rename pending)                                              │
│                                                          │
│  [NEW] JournalEntryEntity (Room, v+1 migration)          │
│    id · createdAt · updatedAt · body(md) · role          │
│    (user|assistant) · sessionId · moodTag?               │
│                                                          │
│  [NEW] JournalDao ──► flows into existing Repository DI   │
│                                                          │
│  [NEW] MindViewModel (hiltViewModel, one per session)     │
│        │  StateFlow<MindUiState>                         │
│        ▼                                                 │
│  [NEW] MindReflectionUseCase                             │
│        │  builds prompt (system persona + history +      │
│        │  optional task-context block)                   │
│        ▼                                                 │
│  [NEW] XybridRuntime (@Singleton)                        │
│        ├─ Xybrid.init(context) in YataApplication        │
│        ├─ ensureModel(): downloads LFM2-1.2B Q4_K_M      │
│        │   (~700 MB) from HuggingFace to                 │
│        │   filesDir/xybrid/lfm2-1.2b/ + writes our       │
│        │   model_metadata.json beside it, loads via      │
│        │   ModelSource.directory(...)                     │
│        ├─ reflect(prompt): streaming token callback       │
│        │   → MutableStateFlow<String>                    │
│        └─ unloadOnBackground() (RAM guardrail)           │
│                                                          │
│  [NEW] MindScreen (Compose destination in nav graph)     │
│        chat-style list · MarkdownText rendering ·        │
│        model-download sheet w/ progress                  │
└──────────────────────────────────────────────────────────┘
```

**Dependency rule:** UI → ViewModel → UseCase → Runtime → SDK. The only class that imports
`ai.xybrid.*` is `XybridRuntime`. Everything else depends on a `ReflectionEngine` interface,
so unit tests run against a fake that streams canned tokens instantly.

## 3. Model Choice

**LFM2-1.2B instruct (Q4_K_M GGUF)** — Liquid AI's edge-first hybrid conv+attention arch.
- ~700–800 MB on disk, fits the 1–2 GB RAM budget with context ≤ 2048
- Same architecture family as registry's `lfm2.5-350m` → proven inside Xybrid's llama.cpp backend
- Delivered via first-run downloader (Wi-Fi default, resumable), NOT bundled in APK
- Fallback if LFM2 validation fails on desktop: `HuggingFaceTB/SmolLM3-1.7B` or registry
  `qwen3.5-0.8b` (auto-download path), decided at Phase 0 gate

## 4. Experience

### 4a. Free-write mode
Type stream-of-consciousness → tap Reflect → tokens stream into a Markdown-rendered reply.
System prompt contract ("thinking partner, not advice column"):
mirror the user's words, ask exactly one open question, ≤ 120 words, never diagnose.
Every exchange persists as a JournalEntry pair (user + assistant) under a sessionId.

### 4b. Daily review mode
Pulls today's tasks from the existing Room DB (completed / pending / overdue counts + titles),
formats them into the prompt, asks the model for three gentle reflection prompts about the day.
Runs once per calendar day (DataStore flag), reachable from the Mind tab header.

### 4c. Model manager
First visit shows a download sheet: size, progress %, Wi-Fi-only toggle, pause/resume,
storage used, delete-model button. After download, load happens once and is cached;
backgrounding the app for > 60s unloads the model (RAM guardrail), transparently reloading
on next session.

## 5. Privacy & Safety

- Inherited: biometric/PIN App Lock already gates the whole app.
- New: Mind tab respects the lock like every other screen (no extra work needed).
- All journal data stays in the existing Room DB → included in existing JSON backup/export.
- Zero telemetry around Mind; Xybrid telemetry remains off unless user opts in globally.
- Model download is the ONLY network call, explicit and cancellable.

## 6. Error Handling

| Failure | Behavior |
|---|---|
| Download fails/interrupted | Resumable retry banner; app fully usable without Mind |
| Out-of-disk (< 1 GB free) | Pre-download check, clear message |
| Model OOM on 2 GB device | Catch, unload, suggest smaller fallback model |
| Inference error mid-stream | Partial answer saved, inline retry chip |
| First token > 8 s | Progress indicator with cancel |

Errors surface via the SDK's sealed `XybridException` types mapped to one `MindError` enum —
exhaustive `when`, no swallowed failures.

## 7. Testing

- Unit: UseCase + prompt-builder against fake engine; DAO tests (in-memory Room).
- ViewModel tests: state transitions (idle → downloading → ready → reflecting).
- Instrumented: `MindScreenSmokeTest` (open tab, fake-engine reflection renders, entry saved)
  — emulator only, honoring YATA's destructive-test guards (`-PdisposableDevice`).
- Phase 0 desktop gate: validate metadata + GGUF via xybrid-cli before any Android work.

## 8. Phases

0. **Validate model** — desktop `from_directory()` run of hand-written metadata + LFM2 GGUF. *Gate.*
1. **Data layer** — entity, DAO, migration, repository wiring.
2. **UI with fake engine** — MindScreen, ViewModel, nav entry, feature flag.
3. **Real runtime** — XybridRuntime, downloader sheet, streaming wiring.
4. **Daily review** — task grounding + DataStore daily flag.
5. **Polish** — low-RAM guardrails, smoke test, CHANGELOG entry.
Stretch: upstream PR (registry entry + docs) to xybrid-ai/xybrid.
