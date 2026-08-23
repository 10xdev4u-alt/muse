# ADR-004: Only XybridRuntime imports ai.xybrid.*

Date: 2026-08-23 · Status: accepted
Implemented by: #14 (seam), #20 (runtime), #23 (error mapping), #21/#57 (acquisition seam)

## Decision

Exactly one class touches the Xybrid Kotlin SDK: `data/mind/XybridRuntime.kt`.
Everything upstream depends on interfaces:

| Consumer | Depends on | Never sees |
|---|---|---|
| MindViewModel | ReflectionEngine, ModelAcquisition, JournalRepository | SDK types |
| UI | StateFlows of plain data | SDK types |
| Tests | fakes of those interfaces | SDK AAR |

Supporting translations live beside the runtime: `xybridErrorToMindError`
(#23) converts the sealed error hierarchy; ModelDownloader speaks HTTP and
exposes only DownloadState.

## Pinned API surface (from research #4, verified at compile in #20)

`Xybrid.init`, `XybridModelLoader.fromDirectory().load()`,
`streamTokens(envelope): Flow<XybridStreamToken>` (extension — import
explicitly), `Envelope.text()`, `unload()`, sealed `XybridError`.

## Consequences

- Swapping real↔fake is one Hilt binding line (#20 proved it).
- ViewModel tests need no device, no AAR, no model bytes.
- Upgrading xybrid-kotlin versions touches exactly one file plus this ADR.
- Cost: a thin interface layer that must be kept honest when new SDK
  capabilities arrive — additions go through this ADR first.
