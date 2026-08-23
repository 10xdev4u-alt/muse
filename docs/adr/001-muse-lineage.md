# ADR-001: Muse's lineage — YATA base, rewritten history, MIT

Date: 2026-08-23 · Status: accepted

## Decision

Muse's task-manager foundation is a snapshot of rjwarrier/yata imported as a
fresh initial commit under 10xdev4u-alt authorship — no fork relationship,
no upstream git history. Muse ships its own MIT license.

## Why

- Upstream has no LICENSE file; unlicensed code defaults to all-rights-
  reserved, so a fork or vendored copy in a public repo would have been
  legally murky. The upstream author resolved this directly: he asked us to
  clone, rewrite the commits as our own, and continue development with our
  own license.
- Rewritten history keeps provenance honest *here*: commit zero is "bootstrap
  muse from yata base", and this ADR records what that means.
- MIT maximizes downstream freedom consistent with how we benefited.

## Obligations we take on

- Credit YATA and rjwarrier visibly (README Credits section) — kept even as
  the package renames to dev.tenx.muse (#30).
- Track behavioral divergence honestly: every change on top of the bootstrap
  commit is ordinary Muse history, reviewed through the same loop.
