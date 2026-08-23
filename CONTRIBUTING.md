# Contribution template — Muse

## The loop (how every change lands)

1. Pick an issue (or raise one first if the idea isn't tracked yet).
2. Branch: `feat/<issue#>-<slug>`, `fix/<issue#>-<slug>`, `research/<issue#>-<slug>`.
3. Research inside the PR description before code: what exists, what was evaluated, why this approach.
4. Implement small. One issue per PR.
5. Validate locally: `./gradlew :app:compileDebugKotlin -q` plus relevant unit tests must pass.
6. Commit conventional style, subject <= 6 words (`feat: add journal entry entity`).
   Co-author trailer for AI-assisted work:
   `Co-authored-by: the-ai-developer <the-ai-developer@users.noreply.github.com>`
7. Push, open PR using the template, request review from `10xdev4u-alt` or `the-ai-developer`.
8. Merge (squash), delete local and remote branch, prune stale branches, move on.

## Ground rules

- No direct pushes to `main` except repo bootstrap.
- Every user-visible change updates `CHANGELOG.md` under `[Unreleased]` in the same commit.
- Never run instrumented tests against a personal device. Emulator or disposable device only,
  with `-PdisposableDevice`. The build enforces this; do not work around it.
- Docs follow plain-writing style: no puffery, no filler, say what the thing does.
