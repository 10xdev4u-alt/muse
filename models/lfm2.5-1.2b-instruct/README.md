# lfm2.5-1.2b-instruct

Muse's reflection model. Weights are NOT in this repo; the app downloads the
GGUF once and stores it beside `model_metadata.json`.

## Provenance

- Source: <https://huggingface.co/LiquidAI/LFM2.5-1.2B-Instruct-GGUF>
- File: `LFM2.5-1.2B-Instruct-Q4_K_M.gguf`
- Size: 730.9 MB (verified 2026-08-23 via HF API)
- License: LFM Open License v1.0 (weights only, via that repo)

## Why this one

Same 1.2B class as LFM2 but a newer generation trained for instruction
following. The Xybrid registry already ships `lfm2.5-350m`, so the engine's
llama.cpp backend is known to run this architecture family. Q4_K_M keeps RAM
under the 1–2 GB budget with context capped at 2048.

Fallbacks if validation fails, in order: `LiquidAI/LFM2-1.2B-GGUF` (Q4_K_M),
registry `qwen3.5-0.8b`. Decision recorded in ADR-002.
