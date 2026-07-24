---
name: ce-map-codebase
description: Map a repository's current stack, integrations, architecture, structure, conventions, testing, and risks into docs/codebase/. Use before ce-plan or ce-work on unfamiliar or significantly changed codebases, for brownfield onboarding, before refactors, or when durable CE planning context is stale or missing.
---

# CE Map Codebase

## Objective

Create or refresh durable codebase orientation documents under
`docs/codebase/` so future CE skills can ground plans, implementation, debug
work, reviews, and captured learnings in the current repo.

Primary output:

- `docs/codebase/STACK.md`
- `docs/codebase/INTEGRATIONS.md`
- `docs/codebase/ARCHITECTURE.md`
- `docs/codebase/STRUCTURE.md`
- `docs/codebase/CONVENTIONS.md`
- `docs/codebase/TESTING.md`
- `docs/codebase/CONCERNS.md`

These files are reference material, not progress state. Do not mutate
`docs/plans/` or `docs/solutions/` while mapping unless the user separately asks
for planning or compounding.

## Arguments

Recognize these optional argument patterns:

- `--fast`: map fewer focus areas. Default fast focus is `tech+arch`.
- `--focus tech|arch|quality|concerns|tech+arch`: restrict focus.
- `--refresh`: overwrite existing `docs/codebase/` documents without asking.
- `--paths <path[,path...]>`: restrict exploration to safe repo-relative path
  prefixes.
- Any other text is a human focus area hint.

Reject `--paths` values that contain `..`, start with `/`, or contain shell
metacharacters such as `;`, backticks, `$`, `&`, `|`, `<`, or `>`. If every path
is invalid, warn and fall back to a whole-repo scan.

## CE Context

Before scanning implementation files, gather lightweight CE context:

1. Read `CONCEPTS.md` if it exists.
2. List `docs/solutions/**/*.md` and read only files whose titles, tags, or
   paths appear relevant to the mapped area.
3. List repo-local skills under `skills/*/SKILL.md`, `.agents/skills/*/SKILL.md`,
   `.codex/skills/*/SKILL.md`, and `.claude/skills/*/SKILL.md` when those paths
   exist. Read the `SKILL.md` files as a lightweight index.
4. Do not bulk-load large `AGENTS.md`, `CLAUDE.md`, build outputs, dependency
   trees, or generated artifacts.

Surface CE-specific constraints in the map:

- `ce-plan` uses repo-relative paths, existing patterns, and test scenarios.
- `ce-work` uses plan file references, patterns to follow, and verification
  gates.
- `ce-debug` benefits from architecture, integration, and concern notes.
- `ce-code-review` benefits from conventions, testing patterns, and risks.
- `ce-compound` stores durable learnings in `docs/solutions/`; link relevant
  learnings from codebase docs instead of duplicating whole solution content.

## Workflow

1. Resolve the repo root with `git rev-parse --show-toplevel` when available.
2. Parse arguments and decide focus:
   - Full map: `tech`, `arch`, `quality`, and `concerns`.
   - Fast default: `tech+arch`.
   - Explicit `--focus`: only the requested focus or pair.
3. If `docs/codebase/` already exists and `--refresh` was not provided, ask
   whether to refresh, skip, or map only a specific focus.
4. Create `docs/codebase/`.
5. Load the mapper prompt in
   `references/agents/ce-codebase-mapper.md`.
6. If the harness supports isolated subagents, dispatch one mapper per focus.
   Otherwise run the same mapper instructions inline, focus by focus. Keep
   write ownership clear so two workers do not edit the same document.
7. Verify expected documents exist and report line counts.
8. Offer CE next steps:
   - Run `ce-plan` when the user wants an implementation plan grounded in the
     map.
   - Run `ce-work` when there is already an implementation-ready plan or clear
     work prompt.
   - Run `ce-debug` when the map exposed a failing behavior to diagnose.
   - Run `ce-compound` only when the mapping revealed a durable project learning
     worth capturing in `docs/solutions/`.
   - Run `ce-commit` only if the user wants these docs committed.

## Focus Outputs

- `tech`: write `STACK.md` and `INTEGRATIONS.md`.
- `arch`: write `ARCHITECTURE.md` and `STRUCTURE.md`.
- `quality`: write `CONVENTIONS.md` and `TESTING.md`.
- `concerns`: write `CONCERNS.md`.

## Safety Rules

- Never read or quote secret-bearing files: `.env`, `.env.*`, `*.env`,
  `credentials.*`, `secrets.*`, `*secret*`, `*credential*`, private keys,
  package manager auth files, cloud credential JSON files, or ignored files that
  appear to contain secrets.
- Note secret file existence only, for example: "`.env` present - environment
  configuration exists."
- Prefer `rg` and `rg --files` for discovery. Fall back to `grep` or `find` only
  when needed.
- Use structured file reads and writes. In Codex, use `apply_patch` for manual
  edits; do not create documents with heredocs.
- Write current state only. Avoid "was", "used to", migration diaries, and
  stale speculation.
- Use repo-relative paths in every finding.
- Do not commit by default.

## Success Criteria

- `docs/codebase/` exists.
- All expected documents for the selected focus exist.
- Documents follow the mapper templates.
- Findings include concrete repo-relative paths.
- CE docs, skills, and solution references are reflected where relevant.
- The final response reports only what was written, verification status, and
  suggested CE next step.
