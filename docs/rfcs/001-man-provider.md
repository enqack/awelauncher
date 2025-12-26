# RFC 001: Man/Apropos Provider

| Status      | Proposed              |
| :---------- | :-------------------- |
| **Author**  | System Engineer Agent |
| **Date**    | 2025-12-26            |
| **Version** | 0.4.5                 |

## Summary

Implement a provider to index, search, and view system manual pages using
`apropos` and `man`, reinforcing `awelauncher`'s identity as a "system
engineer's launcher".

## Motivation

- **Speed**: "I know what I want, not what it's called." `apropos` provides
  keyword-based discovery which is often faster than web search.
- **Integration**: Leverages the dmenu-compliance and terminal launching
  capabilities already present.
- **Completeness**: A launcher for a Linux system without man page access feels
  incomplete.

## Detailed Design

### 1. The `apropos` Indexer

The provider will rely on the system's existing manual database via the
`apropos` command.

**Command:**

```bash
apropos -r .
# OR for specific query
apropos <query>
```

_Note: `apropos .` dumps the entire DB. `apropos <query>` utilizes the optimized
search._

**Parsing Strategy:** Standard `apropos` output format:

```text
name (section) - description
```

Example: `ls (1) - list directory contents`

Regex: `^(\S+)\s+\((.+?)\)\s+-\s+(.+)$`

**Item Schema:**

- **ID**: `man:<section>:<name>` (e.g., `man:1:ls`)
- **Primary**: `<name> (<section>)`
- **Secondary**: `<description>`
- **Icon**: `system-help` (fallback: `utilities-terminal`)
- **Exec**: `man <section> <name>`
- **Terminal**: `true` (Launch in terminal)

### 2. Provider Modes

We can expose two operational modes or "sub-providers":

1.  **Direct Search (`man`)**:
    - Users types `man <query>`.
    - Provider runs `apropos <query>`.
    - Returns matches.
    - **Pros**: extremely fast, uses system index optimization.
    - **Cons**: Requires prefix triggering or explicit mode.

2.  **Global Index (`dman` / `docs`)**:
    - Runs `apropos .` (dump all) at startup (async).
    - Caches results in memory.
    - Participating in the global fuzzy search.
    - **Pros**: Instant fuzzy find.
    - **Cons**: ~10k+ items might pollute the global namespace. Best used in a
      dedicated "Docs" Provider Set.

### 3. Graceful Degradation

If `apropos` returns "nothing appropriate" (empty DB):

- The provider must return a single **Actions Item**:
  - **Primary**: "Manual database empty"
  - **Secondary**: "Run 'sudo mandb' to index system documentation"
  - **Exec**: `sudo mandb` (Terminal=true)

### 4. Caching

- `apropos` is fast, but spawning a process on every keystroke
  (`apropos <query>`) is bad for the `< 16ms` latency target.
- **Strategy**:
  - **Debounce**: 150ms?
  - **Cache**: LRU cache of query -> results?
  - **Preferred**: Async process execution. The UI shows stale/empty results
    until the `QProcess` returns.

## Open Questions

- **Formatting**: Should we try to render man pages as HTML/Markdown in a
  preview window? (Deferred to v0.6+).
- **Updates**: How to detect if `mandb` has run? (inotify on `/var/cache/man`?
  Simply TTL 1 hour?)

## Roadmap

1. Implement `ManProvider` class wrapping `QProcess` calls to `apropos`.
2. Add regex parsing logic.
3. Integrate into `LauncherController`.
4. Add "empty DB" detection.
