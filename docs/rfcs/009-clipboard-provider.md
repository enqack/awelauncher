# RFC 009: Clipboard Provider

| Status      | Proposed              |
| :---------- | :-------------------- |
| **Author**  | System Engineer Agent |
| **Date**    | 2025-12-26            |
| **Version** | 0.5.0                 |

## Summary

Implement a provider that tracks clipboard history and allows users to search,
preview, and re-copy or paste previous entries.

## Motivation

Frequent copy-pasting is a core workflow for developers and power users.
`awelauncher` is well-positioned to be a fast, searchable clipboard manager
without requiring a separate heavy application.

## Detailed Design

### 1. Data Collection (The Collector)

- **Mechanism**: A background thread (or service mode component) listens for
  clipboard changes.
- **Wayland Hurdles**: Browsing the clipboard on Wayland usually requires
  `wl-paste` or a compositor-specific protocol like `wlr-data-control`.
- **Primary Focus**: Text and URLs initially. Image support is deferred.

### 2. Storage

- **Persistence**: Items are stored in `~/.cache/awelauncher/clipboard.json` (or
  a simple sqlite db if volume is high).
- **Trimming**: Keep a maximum of 100 entries by default.

### 3. Search & Interaction

- **Trigger**: `--show clip` or inclusion in a `set`.
- **Item Schema**:
  - `primary`: First line of the snippet.
  - `secondary`: Timestamp + Character count.
  - `meta`: Full content for searching.
- **Actions**:
  - `Return`: Copy to clipboard and hide.
  - `Shift+Return`: Paste directly into focus window (requires `wtype`).

## Technical Implementation

- **`ClipboardProvider`**: Implements the `Provider` interface.
- **Integration**: Works best in **Service Mode (RFC-007)** to ensure no missed
  entries while the launcher is hidden.
- **Security**: Sensitive data (passwords) can be ignored if the application
  marks them (e.g., via `x-kde-passwordManagerHint`).

## Open Questions

- **Privacy**: Should there be a "Clear History" action or a "Private Mode"?
- **Display**: How to show multi-line snippets cleanly in a single-row result?
  - _Proposed_: Show first 50 chars, use `secondary` for type/time.
