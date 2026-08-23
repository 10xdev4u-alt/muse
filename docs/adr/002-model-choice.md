# ADR-002: Muse's on-device LLM is LFM2.5-1.2B-Instruct (Q4_K_M)

Date: 2026-08-23 · Status: accepted · Benchmark: docs/research/006-phase0-benchmark.md

## Context

Muse needs a reflection model under a 1–2 GB RAM budget, loadable through the
Xybrid Kotlin SDK from a directory we control. The original spec picked
LiquidAI LFM2-1.2B with fallbacks (SmolLM3-1.7B, registry qwen3.5-0.8b).

## Decision

Ship `LiquidAI/LFM2.5-1.2B-Instruct-GGUF`, file
`LFM2.5-1.2B-Instruct-Q4_K_M.gguf` (730.9 MB), context 2048.

## Why

- Same 1.2B class and RAM envelope as the spec pick, but a newer generation
  tuned for instruction following — discovered by querying the HF API rather
  than trusting the spec's model card era.
- Architecture family already proven inside Xybrid's llama.cpp backend; the
  xybrid registry ships lfm2.5 variants.
- Q4_K_M over Q4_0: 35 MB premium for better quantization quality.
- Phase 0 benchmark validated end-to-end: SHA256 matched HF LFS OID before
  inference; coherent generation; persona contract honored except mirroring,
  which is a prompt-template concern (#27), not a model concern.

## Fallback ladder (unchanged, untriggered)

1. LiquidAI/LFM2-1.2B-GGUF Q4_K_M
2. registry qwen3.5-0.8b

## Consequences

- Downloader (#19) pins the recorded SHA256 and verifies before load.
- Metadata lives in-repo at models/lfm2.5-1.2b-instruct/ as the single source;
  the app copies it next to the weights at download time.
- Weights are not redistributed; users fetch them once from HuggingFace under
  the LFM Open License v1.0.
