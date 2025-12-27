# RFC 004: Pinned Results & Aliases

| Status      | Proposed              |
| :---------- | :-------------------- |
| **Author**  | System Engineer Agent |
| **Date**    | 2025-12-26            |
| **Version** | 0.5.0                 |

## Summary

Implement a mechanism for users to explicitly pin items to the top of search
results and define custom aliases for commands or applications.

## Motivation

While MRU (Most Recently Used) scoring handles frequent actions, some items
should always be at the top regardless of usage (Pins). Additionally, some
commands are easier to remember by a short nickname than their full name
(Aliases).

## Detailed Design

### 1. Pinned Results

Pins are items that are forcibly sorted to the top of the list if they match the
query at all.

- **Configuration**:

  ```yaml
  general:
    pins:
      - id: "firefox.desktop"
      - id: "terminal:top"
  ```

- **Behavior**:
  - If a pinned item's `primary` or `id` matches the query (even if it's not the
    highest fuzzy score), it is moved to the top.
  - Pins are sorted amongst themselves by their definition order in config.

### 2. Aliases

Aliases allow a user to trigger an item by a shorthand name.

- **Configuration**:

  ```yaml
  general:
    aliases:
      - name: "ff"
        target: "firefox.desktop"
      - name: "sz"
        target: "run:ssh -p 2222 root@server"
  ```

- **Behavior**:
  - If the query strictly matches an alias `name`, the target item is injected
    as the first result.
  - Partial matches on aliases (e.g. typing "f" for alias "ff") will still rank
    the target item higher but not necessarily at the top.

### 3. Unified Storage

Both Pins and Aliases can be defined in `config.yaml` globally or within `sets`.

```yaml
sets:
  dev:
    pins: ["gh.desktop"]
    aliases:
      - name: "gc"
        target: "git-commit-helper --amend"
```

## Technical Implementation

- **Config**: Update `Config.h/cpp` to parse `pins` (vector of strings) and
  `aliases` (map of string to string).
- **Model**:
  - `LauncherModel` will check the `pins` list during the sorting phase.
  - A new `AliasProvider` or integration into `Aggregator` will handle synthetic
    results for aliases.
- **Matching**: Alias matches take absolute precedence over fuzzy matches.

## Open Questions

- **Conflict Resolution**: What if an alias name matches a real binary?
  - _Proposed_: Alias takes precedence. The real binary will still appear
    further down the list.
