---
name: context-handoff
description: Capture a durable, evidence-based continuation note for an unfinished DarkTableNext refactor or investigation. Use when pausing a long task, moving work to another session or agent, documenting a blocker, or preserving exact two-repository GeoControls state and next actions.
---

# Context Handoff

Create a compact operational record that lets the next session continue without rediscovering
decisions or mistaking planned work for completed work.

## Collect evidence

Read root `AGENTS.md`, then inspect:

```sh
git status --short --branch
git log -5 --oneline
git diff --stat
git diff
git diff --cached
```

Record the current goal, relevant product boundary documents, files actually changed, and the
latest build/test output. If GeoControls is involved, collect the same Git state in its real
checkout plus the host dependency mode, path, pinned commit, and whether that commit was pushed.

Do not include secrets, tokens, private URLs, huge patches, or generated build output.

## Write the handoff

Use these sections in this order:

1. **Goal** — the concrete outcome and current scope.
2. **Product/architecture boundary** — decisions that must not be reopened accidentally.
3. **Current state** — branch, HEAD, dirty files, dependency mode, and active checkout.
4. **Completed** — only work verified in the repository.
5. **Validation** — exact commands, results, and environment limitations.
6. **Open issues or blockers** — evidence, failed attempts, and what would unblock them.
7. **Next action** — the first exact file/command/change to execute.
8. **Remaining sequence** — ordered steps to reach the goal.
9. **Do not do** — known destructive, stale, or architecturally invalid paths.

When no output path is requested, return the handoff in the conversation. When a repository file
is explicitly requested, use the requested path; otherwise prefer a clearly temporary or
task-specific name rather than silently overwriting roadmap documents.

## Accuracy rules

- Distinguish “implemented,” “validated,” “planned,” and “assumed.”
- Include commit hashes only after verifying them.
- Say whether tests were not run, failed, or passed.
- Identify generated and ignored files so the next session does not stage them.
- For linked repositories, never collapse two dirty worktrees into one Git state.
- End with an executable next step, not “continue investigating.”

Creating a handoff does not authorize a commit, push, cleanup, reset, dependency update, or other
state change beyond the requested note.
