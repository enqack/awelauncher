# RFC 008: Context Actions

| Status      | Proposed              |
| :---------- | :-------------------- |
| **Author**  | System Engineer Agent |
| **Date**    | 2025-12-26            |
| **Version** | 0.5.0                 |

## Summary

Transform `awelauncher` from a simple "launcher" into a "workflow tool" by
adding support for secondary actions on items (e.g. "Run in Terminal", "Open
Config", "Kill Parent").

## Motivation

Selecting an item shouldn't just do one thing. Sometimes you want to launch
an app in a terminal to see its logs, or move a window to a specific
workspace without switching to it. Context actions provide this power without
cluttering the primary interface.

## Detailed Design

### 1. Interaction Model

- **Primary Action**: `Return` / `Click`.
- **Secondary Actions List**: Triggered by `Tab` or `Right Click`.
  - When triggered, the result list is replaced by a temporary "Action List" for
    the selected item.
- **Direct Hotkeys**:
  - `Shift+Return`: Run in Terminal (Global override).
  - `Ctrl+O`: Open location/config.

### 2. Provider-Specific Actions

Providers define their own actions:

- **drun**: Run in Terminal, Edit .desktop file.
- **window**: Close, Move to Workspace, Fullscreen, Maximize.
- **top/kill**: Stop (SIGSTOP), Continue (SIGCONT), Kill (SIGKILL).
- **ssh**: Connect as Root, Port Forwarding.

### 3. Action Schema

```cpp
struct LauncherAction {
    QString name;
    QString icon;
    std::function<void()> callback;
};
```

## Technical Implementation

- **`LauncherController`**: New state `Mode::ActionList`.
- **QML**: `LauncherRoot.qml` needs a transition to show the action list.
- **Models**: `LauncherModel` can be re-populated dynamically or we use a
  separate `ActionModel`.

## Open Questions

- **Discovery**: How do users know an item has actions?
  - _Proposed_: A small hint/icon in the [Footer.qml](file:///home/sysop/Projects/awelauncher/src/qml/Footer.qml) (e.g., "[Tab] Actions").
