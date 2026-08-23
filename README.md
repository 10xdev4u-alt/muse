# Muse

A journal that talks back. Offline.

Muse is an Android journaling companion with a local LLM built in. You write your
thoughts, a small language model running entirely on your phone reflects them back
with questions instead of advice, and every word stays on the device. It also knows
what your day looked like, because it ships inside a task manager and reads your
real task history for its daily review.

No accounts. No API keys. No network after the model download. Airplane mode is
the intended way to use it.

## How it works

The app bundles [Xybrid](https://github.com/xybrid-ai/xybrid), an on-device
inference runtime, via its Kotlin SDK (`ai.xybrid:xybrid-kotlin`). On first use of
the Mind tab, Muse downloads the LFM2-1.2B instruct model (Q4_K_M GGUF, roughly
700 MB) once from Hugging Face over Wi-Fi, stores it in app-private storage, and
runs all inference through llama.cpp on-device from then on.

Two modes:

- **Free write.** Type a stream of consciousness, tap reflect, and the model
  streams a short reply that mirrors what you wrote and asks one open question.
  Every exchange is saved as a journal entry.
- **Daily review.** Once a day, Muse pulls today's completed, pending, and overdue
  tasks from the Room database and asks the model for three gentle reflection
  prompts grounded in what actually happened.

## Privacy

Biometric or PIN lock gates the whole app. Journal entries live in the existing
Room database and ride its JSON backup and export. The only network call the AI
feature ever makes is the one-time model download, which you start explicitly and
can cancel.

## Building

```bash
./gradlew :app:compileDebugKotlin -q   # fast correctness check
./gradlew :app:assembleDebug -q        # full debug build
./gradlew :app:installDebug -q         # install to a connected device
./gradlew :app:testDebugUnitTest       # JVM unit tests
```

Android Studio Hedgehog or newer, JDK 17, compileSdk 35, minSdk 26.

## Status

Early development. The design spec lives at
`docs/superpowers/specs/2026-08-23-yata-mind-design.md` (named YATA Mind before
the rename). We develop in the open: research produces issues, issues produce PRs,
and every phase lands as a reviewed merge. See `CONTRIBUTING.md` and the issue
tracker for where things stand.

## Credits

Muse's task-manager foundation started from [YATA](https://github.com/rjwarrier/yata)
by rjwarrier, who kindly allowed this rewrite to carry its own history and MIT
license. The inference engine is Xybrid by the xybrid-ai team. Both are worth your
star.

## License

[MIT](LICENSE)
