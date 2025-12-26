# Spec: Systems Utility Providers

## Goal

Implement lightweight, high-value providers that replace common one-off shell
scripts with robust, native C++ implementations.

---

## 1. Top Provider (`top`)

A snapshot view of system resource usage.

### Top Functionality

- **List**: Shows top N processes sorted by CPU usage.
- **Item Format**: `[CPU%] [MEM%] {Process Name} (PID)`
- **Behavior**: Takes a snapshot on open. No live updates (to prevent fuzzy
  search jitter). User re-opens to refresh.
- **Action**: `Enter` sends `SIGTERM`.

### Top Configuration

```yaml
top:
  limit: 10 # Number of processes to show
  sort: "cpu" # "cpu" or "memory"
```

---

## 2. Kill Provider (`kill`)

A targeted process killer (like `pkill` but visual).

### Kill Functionality

- **List**: All user-owned processes.
- **Matching**: Fuzzy match on process name and command line arguments.
- **Actions**:
  - `Enter`: `SIGTERM`
  - `Shift+Enter`: `SIGKILL`
- **Icons**: Attempts to map process name to a known `drun` icon if possible,
  otherwise generic executable icon.

### Kill Configuration

```yaml
kill:
  show_system: false # If true, shows root processes (requires permissions to kill)
```

---

## 3. Todo Provider (`todo`)

A simple task manager.

### Todo Functionality

1. **List Tasks**: Shows tasks from a simple text file (`~/.todo.txt`).
2. **Add/Complete**: Inline actions to add or mark tasks as done.

### Todo Configuration

```yaml
todo:
  file: "~/.todo.txt" # Path to the todo file
```

---

## 4. SSH Provider (`ssh`)

Instant access to remote hosts defined in SSH config.

### SSH Functionality

- **Sources**:
  1. `~/.ssh/config` (Host blocks)
  2. `~/.ssh/known_hosts` (Deduplicated hostnames)
- **List Items**: All unique hostnames/aliases.
- **Item Format**: `Alias -> Hostname (User)`
- **Action**: Launches a new terminal window executing `ssh <host>`.

### Terminal Launcher Priority

1. `xdg-terminal-exec` (if available on path).
2. Config: `ssh.terminal` in `config.yaml`.
3. Fallback: `$TERM` environment variable.

### SSH Configuration

```yaml
ssh:
  terminal: "" # Override command (e.g. "foot -e")
  parse_known_hosts: true
```

---

## Implementation Details

- **Performance**: Use direct `/proc` filesystem parsing for process lists
  (avoid shelling out to `ps`).
- **SSH Parsing**: Simple regex parsing for `Host` headers in `ssh_config`.
