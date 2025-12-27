# RFC 003: Monitor Destinations (Placement)

| Status      | Proposed              |
| :---------- | :-------------------- |
| **Author**  | System Engineer Agent |
| **Date**    | 2025-12-26            |
| **Version** | 0.5.0                 |

## Summary

Define a mechanism to control which monitor (output) the `awelauncher` window
appears on. This includes explicit selection via CLI/config and dynamic
behaviors like "follow mouse" or "follow focus".

## Motivation

In multi-head setups, a launcher that always appears on the "primary" or
"default" monitor is frustrating. Users expect the launcher to appear where
their attention is (the active monitor) or where they explicitly configure it
(e.g., a dedicated dashboard screen).

## Detailed Design

### 1. Behavior Modes

We will implement four primary placement strategies:

1. **`follow-mouse` (Dynamic)**:
   - The launcher appears on the monitor containing the mouse cursor at the
     instant of execution.
   - _Wayland Implementation_: Detect current `wl_output` via cursor position
     or use Qt's `QGuiApplication::screenAt(QCursor::pos())`.
2. **`follow-focus` (Dynamic - Default)**:
   - The launcher appears on the monitor containing the currently focused
     window.
   - _Note_: This usually aligns with the compositor's default LayerShell
     behavior, but being explicit allows for consistency across compositors.
3. **Explicit Output Name**:
   - Users specify a string (e.g., `DP-1`, `HDMI-A-1`).
   - The launcher searches for an output with this name.
4. **Monitor ID**:
   - Numerical index (0, 1, 2) based on the order detected by the session.

### 2. CLI Contract

New flags to control placement:

```bash
awelaunch --monitor follow-mouse
awelaunch --monitor follow-focus
awelaunch --output DP-2
```

- `--monitor <id|behavior>`: Sets the strategy.
- `--output <name>`: Shorthand for selecting a specific output by name.

### 3. Config Schema

Update `config.yaml` to allow global and per-set defaults:

```yaml
general:
  monitor: "follow-focus" # default

sets:
  dashboard:
    output: "HDMI-A-1" # Force specific screen for this set
    anchor: "top"
```

### 4. Technical Implementation (Qt + LayerShell)

- **Output Discovery**:
  - Use `QGuiApplication::screens()` to enumerate available outputs.
  - Bridge output names via `QScreen::name()`.
- **LayerShell Integration**:
  - Before the QML engine loads/shows the window, call
    `LayerShellQt::Window::setScreen(screen)`.
  - _Caveat_: This must happen early in the window lifecycle for LayerShell to
    respect the destination.

## Open Questions

- **Relative Placement**: Should we support "Left of Focus"? (Deferred: Out of
  scope for v0.5).
- **Fallback**: If `DP-2` is disconnected, fallback to `follow-focus`.

## Roadmap

1. Implement `OutputUtils` to wrap `QScreen` management.
2. Update `Config` and `main.cpp` to parse monitor flags.
3. Modify `LauncherController` to apply screen selection before showing the
   window.
4. Add placement metadata to `LESSONS.md`.
