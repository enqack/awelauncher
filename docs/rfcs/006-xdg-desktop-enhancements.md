# RFC 006: Enhanced XDG Desktop Support

| Status      | Proposed              |
| :---------- | :-------------------- |
| **Author**  | System Engineer Agent |
| **Date**    | 2025-12-26            |
| **Version** | 0.5.0                 |

## Summary

Bring `awelauncher` to full XDG compliance by respecting advanced `.desktop`
keys and supporting "Desktop Actions" (e.g., "New Private Window" for
browsers).

## Motivation

Many modern applications hide themselves (`NoDisplay=true`) or provide useful
contextual keywords that help with search but aren't in the app name. Proper
XDG support makes the launcher feel like a first-class citizen in the Linux
desktop ecosystem.

## Detailed Design

### 1. Visibility Rules

- **Respect `Hidden`/`NoDisplay`/`OnlyShowIn`**:
  - Filter out items that should not be visible in a general launcher.
  - Check `XDG_CURRENT_DESKTOP` to handle environment-specific entries.
- **`TryExec` Check**: (Already partially implemented, but will be enforced).

### 2. Rich Search Data

- **Keywords**: Include the `Keywords` key in the fuzzy matching pool.
- **Categories**: Allow searching by category (e.g., typing "media" shows VLC).

### 3. Desktop Actions

Applications can define multiple "Actions" (e.g., Firefox has "New Window", "New
Private Window").

- **UI Implementation**:
  - Actions appear as child items or secondary results under the main app.
  - _Search Syntax_: Typing `firefox:private` could jump straight to the action.
- **Item Schema**:
  - New role: `parent_id` to link actions to their main app.

## Technical Implementation

- **`DesktopFileLoader`**:
  - Update to parse `Keywords`, `Actions`, and `NoDisplay`.
  - Handle the `[Desktop Action <Name>]` sections.
- **Matching**:
  - Weighted search: Match in `Name` > `Keywords` > `Exec`.

## Open Questions

- **UI Clutter**: Should all actions be shown by default?
  - _Proposed_: No. They should only show if explicitly searched for or if the
    user triggers a "Show Actions" keybind (e.g. `Tab`).
