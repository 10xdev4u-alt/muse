# ADR-003: Journal schema is chat-shaped, not document-shaped

Date: 2026-08-23 · Status: accepted
Implemented by: #9 (entity/DAO), #10 (migration v32), #11 (tests), #12 (repository)

## Context

Journaling apps typically store one row per day/document. Muse's Mind tab
streams LLM reflections next to the user's writing, which forces a choice:
model that as a mutable document with two text fields, or as an exchange of
messages.

## Decision

`journal_entries` stores one row per message: `role` TEXT ('user' |
'assistant'), `sessionId` grouping, indexed for session queries. A reflection
exchange is two rows sharing a sessionId.

## Why

- **Streaming maps to storage.** The assistant row starts empty via
  beginAssistantReply() and its body is replaced as tokens arrive
  (updateBody). No second "streaming buffer" state anywhere in the app; what
  is on screen is literally what is in Room.
- **Crash semantics come free.** Kill mid-stream leaves a visible empty or
  partial assistant reply — honest, recoverable, deletable — instead of a
  corrupted document or a lost reply.
- **Voice later needs nothing new.** A future voice input (#voice path) writes
  a user row exactly like typed input.
- **Daily review is just sessions.** Review outputs are assistant entries like
  any other (#28), so history/search/export treat them uniformly.
- **No TypeConverters in v32.** Role stays TEXT; the enum lives at the domain
  boundary. One less migration hazard.

## Rejected alternatives

- *One document row per session* (body + reflection columns): simpler count,
  but streaming rewrites the whole row per token batch and crash states get
  murky.
- *JSON blob column*: flexible, unqueryable, and FTS integration (#17 search)
  would need projection hacks.

## Consequences

- Session list = GROUP BY sessionId ORDER BY MAX(createdAt) (#11 covers the
  ranking behavior in tests).
- Mood tags ride user rows only (nullable today; UI wires them post-v0.1).
- If Mind ever needs multi-model turns (e.g., a reviewer persona), it appends
  rows — schema unchanged.
