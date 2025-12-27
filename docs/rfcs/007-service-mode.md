# RFC 007: Service Mode (Daemon)

| Status      | Proposed              |
| :---------- | :-------------------- |
| **Author**  | System Engineer Agent |
| **Date**    | 2025-12-26            |
| **Version** | 0.5.0                 |

## Summary

Introduce a background daemon mode (`awelaunch --daemon`) to eliminate startup
latency and allow external control via a Unix socket.

## Motivation: Deep Dive

While `awelauncher` is fast, the transition from "fast" to "instant" requires
the process to be resident.

1. **The 50ms Threshold**: Human perception begins to detect "lag" at ~50ms. A
   cold start involves binary loading, library linking (Qt is large), asset
   decoding, and config parsing. A daemon cuts this to < 10ms.
2. **Asset Warmth**: Icons and fonts are the most expensive items to load. A
   daemon keeps the `IconProvider` cache in RAM, eliminating disk I/O during
   the first search.
3. **Cross-Context State**: A daemon can maintain state that is usually lost on
   exit, such as the `activeSet` or partially typed queries.
4. **External Control (Automation)**: A daemon allows the user to script the
   launcher (e.g., an external keybind to switch the active set or inject
   results).

## Evaluation of Options & Final Decisions

### 1. IPC Mechanism: Unix Domain Sockets (Final)

**Decision**: **Unix Domain Sockets** using the **JSON** protocol. This provides
the best balance of speed, cross-language compatibility, and simplicity. It
allows the launcher to be controlled by anything from a C target to a simple
`socat` command in a bash script.

### 2. Lifecycle: Explicit Daemon (Final)

**Decision**: **Option A: Explicit Daemon**. The user is responsible for
starting the daemon (e.g., in their WM startup script or a systemd service).

- **Binary**: `awelaunch --daemon`
- **Execution**: The main process detaches from the terminal and creates the
  socket.
- **Client**: A secondary execution of `awelaunch` (without `--daemon`) detects
  the socket and sends the command.

### 3. Advanced Integration (Tier 3+)

#### Systemd Socket Activation

By supporting `SD_LISTEN_FDS`, we can allow `systemd` to manage the socket.
`awelauncher` won't even start until the first time the shortcut is hit,
ensuring zero background RAM usage until needed, while still being "ready" via
the pre-opened socket.

#### Secure Remote Management (QUIC/TLS)

For system engineers managing distributed clusters or remote dev-boxes:

- **Exotic IPC**: Support for **QUIC** (via HTTP/3 or raw UDP) would allow
  secure, low-latency control of the launcher over the network.
- **Use Case**: Triggering a launcher on a remote build-server or observability
  dashboard securely without SSH tunneling overhead.
- **Security**: Mutual TLS (mTLS) to ensure only authorized engineer keys can
  trigger UI actions.

## Detailed Design

### IPC Message Schema (Draft v1)

To ensure the daemon is "engineer-friendly," the messaging protocol follows a
strict, versioned JSON schema.

#### 1. Command Envelope (Client -> Daemon)

Every message must include a `version` and an `action`.

```json
{
  "version": 1,
  "action": "show | hide | toggle | reload | query | status",
  "payload": { ... }
}
```

##### Example: `show`

```json
{
  "version": 1,
  "action": "show",
  "payload": {
    "set": "dev",
    "prompt": "Admin > ",
    "query": "top",
    "monitor": "follow-mouse"
  }
}
```

##### Example: `query` (Headless search)

```json
{
  "version": 1,
  "action": "query",
  "payload": {
    "text": "fi",
    "limit": 5
  }
}
```

#### 2. Response Envelope (Daemon -> Client)

Responses indicate success/failure and return data for headless queries.

```json
{
  "status": "ok | error",
  "message": "Optional human-readable info",
  "data": { ... }
}
```

##### Example: `query` Results

```json
{
  "status": "ok",
  "data": {
    "count": 2,
    "items": [
      { "id": "firefox.desktop", "primary": "Firefox", "score": 0.95 },
      { "id": "filezilla.desktop", "primary": "FileZilla", "score": 0.72 }
    ]
  }
}
```

### Resource Management (The "Bloat" Problem)

To prevent the daemon from consuming excessive RAM over time:

- **Pruning**: When hidden for > 15 minutes, the daemon calls
  `engine.trimComponentCache()` and clears the `IconProvider` RAM cache.
- **Lazy Load**: Providers are only instantiated when first requested.

### 3. Client Binary (`awelaunchctl`)

A lightweight C client (or just the main binary in client mode) to talk to the
daemon.

```bash
# Start daemon (usually in autostart)
awelaunch --daemon

# Trigger it
awelaunch --show drun
```

## Technical Implementation

- **`QtSingleApplication`** or custom `QLocalServer/QLocalSocket` logic to
  detect existing instances.
- **`LauncherController`**: Update to support a non-exiting lifecycle.
- **Wayland LayerShell**: Ensure the window can be reliably mapped/unmapped
  without focus issues.

## Open Questions

- **Memory Usage**: How do we prevent providers (especially those with many
  icons) from bloating memory over time?
  - _Proposed_: Implement a cache TTL and clear unused textures when hidden.
