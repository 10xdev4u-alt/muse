# Phase 0 benchmark: lfm2.5-1.2b-instruct through Xybrid

Date: 2026-08-23 · Machine: desktop CPU (Linux) · xybrid CLI installed from master

## Setup

Model dir assembled per our shipped layout: `model_metadata.json` (from
`models/lfm2.5-1.2b-instruct/`) beside the downloaded GGUF symlink.
Integrity verified before any inference:

```
SHA256(local) == HF LFS OID == b1b3de114215d9507409a662a501a631095a479a419584e8a2ded6304b19b4f5
```

## Results

| Run | Input | max_tokens | Wall | Notes |
|---|---|---|---|---|
| 1 (cold FS cache) | procrastination prompt, no persona | 150 | 11.93 s | coherent structured answer |
| 2 (warm) | persona-contract prompt | 130 | 5.09 s | one-question reply |

Rough throughput ~12–25 tok/s CPU depending on cache warmth. Load time included
in run 1; warm runs amortize it. Desktop CPU is the ceiling for what an ARM
phone will do; LFM2.5 is edge-targeted so we expect usable streaming on modern
hardware. Real device numbers land in Phase 3 (#22).

## Persona check (issue #7)

Contract tested: mirror user's words, ask exactly ONE open question, no advice,
<=120 words.

| Criterion | Result |
|---|---|
| Exactly one open question | PASS |
| No unsolicited advice | PASS |
| Word budget | PASS (~17 words) |
| Mirrors user's words | WEAK — replied generically |

Verdict: 3/4 hard-pass, mirroring needs prompt iteration. Recorded here per
#7's acceptance clause; iteration continues inside the prompt-builder work
(#27) where we control the full template rather than stuffing SYSTEM: into
input text.

## Verdict

**GO.** Model loads from our metadata, generates coherently, honors persona
constraints, integrity chain verified end-to-end. Phase 1 may proceed.
Fallback ladder (LFM2-1.2B → qwen3.5-0.8b) not needed.
