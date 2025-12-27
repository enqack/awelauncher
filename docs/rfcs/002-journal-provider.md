# RFC 002: Journal/Logs Provider

| Status      | Proposed              |
| :---------- | :-------------------- |
| **Author**  | System Engineer Agent |
| **Date**    | 2025-12-26            |
| **Version** | 0.4.5                 |

## Summary

Implement a provider to query, filter, and view system logs via `journalctl`,
allowing users to diagnose system state directly from the launcher.

## Motivation

- **Diagnostics**: Quickly check "why did that service fail?" without opening a
  terminal and remembering `journalctl` flags.
- **Visibility**: Surfacing errors or recent logs can proactively highlight
  system issues.
- **Power User Workflow**: Integrate log checking into the "launcher loop" (Run
  -> Check Log -> Fix).

## Detailed Design

### 1. Data Source: `journalctl`

We will use `journalctl` with JSON output for robust parsing.

**Command:**

```bash
journalctl -n 50 --output=json --reverse
# -n 50: Limit to last 50 entries (performance)
# --reverse: Newest first
# --output=json: Structured data
```

**Fields of Interest:**

- `MESSAGE`: The log content.
- `_SYSTEMD_UNIT`: The service name (e.g., `NetworkManager.service`).
- `PRIORITY`: Log level (0-7).
- `__REALTIME_TIMESTAMP`: Time.

**Item Schema:**

- **ID**: `log:<cursor>` (Use journal cursor for uniqueness)
- **Primary**: `MESSAGE` (truncated)
- **Secondary**: `<TIMESTAMP> • <UNIT> • <PRIORITY_LABEL>`
- **Icon**:
  - Priority 0-3 (Emerg-Err): `dialog-error`
  - Priority 4 (Warning): `dialog-warning`
  - Priority 5-7 (Notice-Debug): `dialog-information`
- **Exec**: `journalctl -u <UNIT> -f` (Follow this unit in terminal) OR
  `journalctl -f` if no unit.
- **Terminal**: `true`

### 2. Provider Modes / Filtering

1. **Recent Logs (`logs`)**:
   - Shows global last N logs.
   - Useful for "What just happened?".

2. **Unit Logs (`logs:unit`)**:
   - If user types `logs sshd`, we filter `journalctl -u sshd`.
   - Requires parsing query to construct `journalctl` args?
   - **Optimization**: Maybe just filter the JSON output in memory if N is small
     (100).

3. **Error Stream (`errors`)**:
   - `journalctl -p 3 -n 50` (Show only errors).
   - High signal-to-noise ratio.

### 3. Visual Representation

- **Colors**: Critical/Error should be red/orange. Warning yellow.
- **Layout**: "Secondary" text needs to be concise.
  `14:05 [sshd]: Failed password...`

### 4. Interactions

- **Enter**: Open `journalctl -f` (follow mode) in terminal, starting from
  roughly that point or just tailing.
- **Ctrl+C**: Copy log message to clipboard.

## Performance Considerations

- `journalctl` can be slow on large disks.
- **Strict Limits**: Always use `-n <LIMIT>`.
- **Async**: `QProcess` is mandatory. Do not block the UI thread.
- **Debounce**: Do not query on every character if we are constructing complex
  filters.

## Open Questions

- **Permissions**: `journalctl` usually requires user to be in `systemd-journal`
  group. We should check this capability and warn if missing.
- **Formatting**: JSON output is verbose. C++ parsing overhead? (Should be
  negligible for <100 items).

## Roadmap

1. Implement `JournalProvider` class.
2. JSON parser integration (Qt `QJsonDocument`).
3. Logic to map Priority -> Icon/Color.
4. "Follow Unit" action logic.
