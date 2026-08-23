# Journey log — 2026-08-23: repo birth to Phase 0 gate

## What happened

Same-day arc: cloned and studied xybrid (~147K lines Rust) and yata (~60K lines
Kotlin), specced Muse, stood up the repo with an issues-first constitution,
then executed the first three PR loops and the Phase 0 model gate.

## Loop results

| PR | Issue | Outcome |
|---|---|---|
| #38 | #1 CI workflow | merged; compile+tests green on runner in ~4m |
| #39 | #5 model metadata | merged; HF API verified size before writing schema |
| #40 | #4 SDK audit | merged; 9/9 APIs present, 2 gaps documented |

## Phase 0 gate (#6, #7)

LFM2.5-1.2B-Instruct Q4_K_M ran through our own `model_metadata.json` on
desktop CPU: coherent generation, persona contract 3/4 hard-pass (mirroring
weak, iteration queued for #27), SHA256 matched HuggingFace LFS OID exactly.
**Verdict: GO.** Details: docs/research/006-phase0-benchmark.md

## Lessons

- The spec picked LFM2-1.2B; querying the HF API directly revealed LFM2.5 at
  the same RAM cost. Verify against source data, not model cards.
- A backgrounded download died when its shell timed out. Detached processes
  (`setsid`) plus resumable `curl -C -` fixed it — same pattern our Android
  downloader (#19) must handle.
- Merging PR #38 while its checks were still pending worked out (green after),
  but that was luck dressed up as speed. Branch protection comes with scale.

## Metrics

Issues raised: 37 · Issues closed: 7 (#1 #4 #5 #6 #7 + ADRs pending) ·
PRs merged: 3 · CI runs green: all · Commits: conventional, <=6-word subjects,
co-authored with the-ai-developer throughout.
