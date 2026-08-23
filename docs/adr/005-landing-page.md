# ADR-005: Landing page philosophy

Date: 2026-08-23 · Status: accepted
Implemented by: #34 (page), #35 (deploy)

## Decision

One static HTML file, zero JavaScript, zero frameworks, deployed to GitHub
Pages from main via Actions.

## Why

- The product's whole pitch is "runs on your phone with no internet" — a
  landing page that requires JS to render would argue against its own app.
- Zero-JS means Lighthouse perf 100 by construction, works in any browser,
  and there is nothing to maintain except words and CSS.
- GitHub Pages is free hosting for a free product; custom infra buys nothing
  at this scale.
- Dark-first visual language mirrors the app; copy follows the same
  plain-writing rules as the README (no puffery).

## Consequences

- No analytics on the page at all. Traffic curiosity is not worth adding a
  tracker to an anti-tracking product.
- When facts change (model size, phases), the page changes in the same PR
  discipline as code.
