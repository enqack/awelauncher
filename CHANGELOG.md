# Changelog

All notable changes to this project will be documented in this file.

## [0.4.4] - 2025-12-26

### Changed

- **Version**: Bumped version to 0.4.4
- **Documentation**: Updated README.md

## [0.4.3] - 2025-12-26

### Fixed

- **Nix Build**: Fixed build failure by adding missing `FilterUtils` and
  `TerminalUtils` to `CMakeLists.txt`.
- **SSH Launcher**: Fixed `xdg-terminal-exec` and `wezterm` launching issues by
  correctly handling arguments and flags (now allows complex commands via
  `sh -c`).
- **Dev Tools**: Updated `bump_version.sh` to track `CHANGELOG.md` version
  consistency.

## [0.4.2] - 2025-12-26

### Fixed

- **CLI Overrides**: Fixed `--margin`, `--width`, and `--height` flags being
  ignored (overrides now correctly map to integer config values).
- **Margins**: Fixed window margin application on Layer Shell compositors by
  updating QML property assignment syntax.
- **Focus Border**: Added visual focus indicator (accent-colored border) for
  active state to mimic window manager decoration.

## [0.4.1] - 2025-12-26

### Fixed

- **Anchoring**: Corrected anchoring logic for top/bottom placement.
- **Docs**: Updated README and helper docs.

## [0.4.0] - 2025-12-26

### Added

- **New Providers**:
  - **Top Process (`top`)**: View and monitor processes, sort by CPU/Memory.
  - **Process Killer (`kill`)**: Fuzzy find and kill processes.
  - **SSH (`ssh`)**: Launch SSH sessions from `~/.ssh/config`.
- **Provider Sets**: named configurations grouping multiple providers with
  filters and custom layouts (e.g., `--set dev`).
- **Terminal Integration**:
  - **Shift+Enter**: Force launch in terminal.
  - **Ctrl+Enter**: Force launch in terminal and hold open (cmd; read).
- **Visual Polish**:
  - New empty state overlays.
  - Dynamic footer legend based on context.
  - Improved icon rendering.
- **CLI**: added `--overlay` flag to force overlay layer.

### Changed

- Refactored `LauncherController` to support action flags (Force/Hold Terminal).
- Updated internal architecture to support arbitrary provider composition.

### Fixed

- Portal registration warnings on NixOS/Wayland.
- Icon sizing consistency in SearchBar.

## [0.3.0] - 2025-11-01

### Added

- Window switcher mode.
- Monitor moving actions.
- Theme engine improvements.

## [0.2.0] - 2025-10-15

### Added

- Anchoring support.
- Margin support.
- CLI geometry overrides.

## [0.1.0] - 2025-09-01

### Added

- Initial release.
- drun/run modes.
- Fuzzy search.
