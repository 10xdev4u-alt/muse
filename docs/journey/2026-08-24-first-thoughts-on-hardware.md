# Journey log — 2026-08-24: first thoughts on real hardware

## The night Muse woke up

One device (SM-M045F, 4GB, MediaTek), one USB cable that kept dying,
five crashes, four fixes, one forged bridge — and then streaming tokens
on a phone in airplane mode.

## Crash autopsy chain

| Crash | Root cause | Fix | PR |
|---|---|---|---|
| Flash-close at launch | Hilt field access before `super.onCreate()` | Move registration after super | #73/#74 |
| Download dialog vanished mid-download | Visibility required state==Idle | Lifecycle-aware visibility + ack | #75/#76 |
| `libxybrid_bolt.so not found` | AAR ships hyphen, glue loads underscore | jniLibs underscore copies | #77 |
| `No implementation found` ×85 | Shipped .so has ZERO Java_ exports (upstream bug, filed as xybrid-ai#530) | **Generated full JNI bridge** from upstream's own C header | #79 |

## The bridge

`native-bridge/gen_bridge.py` parses upstream's authoritative C header +
Kotlin externals → emits 85 JNI trampolines with full ABI conversion.
Compiled against NDK r26 for arm64+v7a (~80KB each). Verified end-to-end:
`JNI_OnLoad → set_binding("kotlin") → model_from_directory → run_stream →
stream_next every ~190ms`. Tokens, on hardware, offline.

## Process lessons

- **Verify the artifact, not the intention**: Gradle repackaged a stale .so
  twice. The fix was pulling the INSTALLED apk back off the phone and
  grepping its strings. Now standard practice.
- **Interactive installers hang shells** — prefer already-installed skills;
  we already had equivalent design doctrine locally.
- **Upstream bugs are contributions in disguise**: xybrid-ai#530 includes
  our full analysis and an offer to PR the generator.

## Landing v2 shipped same night

Observatory theme: self-hosted Syne + JetBrains Mono (66KB), CSS-only
starfield/aurora/grain, word-stagger hero, scroll-driven reveals,
REAL numbers strip, REAL Phase-0 transcript. Zero JS maintained.

## State

App: persona switched to dev-companion, streaming on device.
Landing: live at 10xdev4u-alt.github.io/muse.
Board: 26 issues closed of 39 raised. PRs merged: 23.
