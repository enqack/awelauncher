# RFC-011: awelaunchctl - Dedicated Daemon Controller

## Status
- **Date**: 2025-12-26
- **Status**: Draft
- **Target Version**: v1.0.0

## Context
With the introduction of **Service Mode (Daemon)** in v0.4.4 ([RFC-007](file:///home/sysop/Projects/awelauncher/docs/rfcs/007-service-mode.md)), users can control the launcher via a Unix Domain Socket and JSON. While the `awelaunch` binary can act as a client, there is a need for a more ergonomic, specialized tool for scripting, WM keybindings, and advanced state inspection.

`awelaunchctl` will serve as the dedicated "control plane" utility for the launcher.

## Proposed Design

### 1. Goals
- **Ergonomics**: Replace manual JSON creation with natural CLI commands.
- **Pipe-friendliness**: Output raw JSON or formatted text for use in shell scripts.
- **Diagnostics**: Provide tools to inspect daemon health, memory usage, and registration state.
- **Zero Overhead**: Minimal binary size (likely a small C++ utility or even a POSIX-compliant shell script).

### 2. Command Structure

```bash
awelaunchctl <command> [options]
```

#### Commands:
- `show`: Trigger the launcher to show.
  - `--set <name>`: Load a specific provider set.
  - `--mode <mode>`: Load a specific provider mode (run, drun, window).
  - `--query <text>`: Prefill the search box.
- `hide`: Hide the running launcher.
- `toggle`: Toggle visibility.
- `query <text>`: Perform a headless search and print results.
  - `--limit <n>`: Limit results.
  - `--format <json|text>`: Choose output format.
- `reload`: Tell the daemon to reload its configuration.
- `status`: Display daemon status (visibility, uptime, memory, version).
- `quit`: Stop the daemon process.

### 3. Implementation Plan
- **Phase 1**: Implement as a C++ utility using `QLocalSocket` for consistency.
- **Phase 2**: Add support for shell completions (bash/zsh/fish).
- **Phase 3**: Optional "REPL" mode for interactive exploration of the IPC API.

## Alternatives Considered
1. **Keeping `awelaunch` as the client**: This works but makes the primary binary larger and its CLI interface more cluttered.
2. **Bash Wrapper**: A simple `bash` script using `socat` or `printf > /dev/ux/...`. This is lightweight but lacks robust error handling and discovery.

## Security Considerations
`awelaunchctl` respects the same security boundaires as the Unix Domain Socket (file permissions on `/tmp/awelauncher.sock`).
