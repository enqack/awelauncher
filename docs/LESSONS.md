# 🧠 Lessons Learned & Gotchas

A running list of non-obvious technical hurdles and their solutions encountered
during the development of `awelauncher`.

## 📦 Nix & Installation

### 1. The "Vanishing Asset" Problem

**Gotcha**: Assets (like `logo.png`) that exist in the source tree are not
available to the application when installed via Nix or Flakes because they
aren't automatically copied into the Nix store. **Solution**:

- **Embed as Resources**: Add assets to `RESOURCES` in `CMakeLists.txt` via
  `qt_add_qml_module`.
- **System Pixmaps**: Explicitly `install()` assets to `share/pixmaps` in
  `CMakeLists.txt`.
- **Patching Desktop Files**: Use `substituteInPlace` in `flake.nix` to replace
  generic icon paths with the absolute store path of the icon during
  `postInstall`.

### 2. SVG Rendering

**Gotcha**: System icons (magnifying glass, etc.) often fail to render in
minimal Nix environments. **Solution**: Ensure `qt6.qtsvg` is in the
`buildInputs` of the Nix expression.

## 🖼️ Icon Management

### 3. Persistent Icon Cache (Stale Image)

**Gotcha**: Updating `assets/logo.png` had no effect in the app because the
`IconProvider` cached icons based on the filename but didn't detect internal
content changes. **Solution**: Include the file's `lastModified()` timestamp in
the `QCryptographicHash` cache key.

### 4. Alpha Transparency in AI Assets

**Gotcha**: AI image generators often draw a checkerboard pattern when asked for
"transparency." **Solution**: Generate the asset with a pure white background
and post-process using ImageMagick:

```bash
convert input.png -fuzz 10% -transparent white assets/logo.png
```

## 🛠️ Qt & C++ Implementation

### 5. Double-Colon Path Bug (Resource Resolution)

**Gotcha**: `QFile::exists()` checks for resources using the `:` prefix, but the
internal QML path starts with `qrc:/`. Concatenating these incorrectly (e.g.,
`qrc` + `:/...` vs `qrc:` + `:/...`) leads to breakage. **Solution**:

- Use `qrc:/` for QML `source` properties.
- Use `:/` for `QFile` operations.
- Implement a **Fail-Safe Resource Resolver** that uses `QDirIterator` to scan
  `:` in subdirectories if specific paths fail.

### 6. Incomplete Types in `main.cpp`

**Gotcha**: Using `QIcon` methods on `QGuiApplication` requires the explicit
`#include <QIcon>`, even if `QGuiApplication` (or other components) is already
included, to avoid "incomplete type" errors.

## 🏃 Execution & Environment

### 7. PATH Resolution in Nix/Flake

**Gotcha**: When running inside a Nix flake, the environment may not inherit the
user's full desktop `PATH`, causing tools like `ssh-add` or specific terminals
to be "missing." **Solution**: Design `TerminalUtils` and providers to search
multiple candidate paths and provide early `qWarning` debug info for candidate
discovery.

## 🖥️ Terminal Integration

### 8. Terminal Command Wrapping (`-e` vs `--`)

**Gotcha**: Most terminals accept `-e` for executing a command, but their
behavior varies:

- **Standard (xterm/konsole)**: Usually takes `-e "command string"`.
- **WezTerm**: Strongly prefers `wezterm start -- command args`. Passing a
  complex shell script with `-e` often fails or requires nested escaping.
- **xdg-terminal-exec**: Expects the command and its arguments as a list, NOT as
  a single shell-interpreted string. **Solution**: Use a central
  `TerminalUtils::wrapCommand` helper to handle these per-binary quirks and
  always use `sh -c` as a bridge for complex "hold open" scripts.

### 9. The "Hold Open" Trick

**Gotcha**: When launching a CLI tool (like `top` or `ssh`), the terminal closes
instantly when the process ends, preventing the user from seeing errors.
**Solution**: Wrap the command in a shell script:
`sh -c "cmd; echo 'Press Enter to close...'; read"`.

## 🌐 Wayland & UI

### 10. Floating & Centering (Niri/LayerShell)

**Gotcha**: LayerShell applications are normally anchored to edges. For a
"launcher" feel on Wayland (specifically Niri), the application needs specific
LayerShell hints to float. **Solution**: Ensure `LayerShellQt::Interface` has
all anchors disabled to allow the compositor to handle the window as a
floating/centered popup.

### 11. QML Component Scope

**Gotcha**: Refactoring large QML files into components (e.g., `SearchBar.qml`)
can break access to C++ `contextProperty` values if the component isn't properly
re-linked or if it relies on parent-scope variables that are now isolated.
**Solution**: Pass dynamic values as `properties` or ensure the component is
registered in the same `QQmlContext`.

## 📂 Desktop File Parsing (XDG)

### 12. XDG %-Codes in `Exec`

**Gotcha**: `.desktop` files frequently contain codes like `%u`, `%F`, or `%i`
in their `Exec` line. Executing these strings directly in C++ leads to "Command
not found" or syntax errors. **Solution**: Use a regex to strip these codes:
`exec.remove(QRegularExpression(" %[%a-zA-Z]"))`.

### 13. Binaries Missing but Entry Exists

**Gotcha**: An application might have a `.desktop` file installed but the binary
itself might be missing (e.g., from a partial uninstall). **Solution**: Respect
the `TryExec` key in the desktop file. Use `QStandardPaths::findExecutable` to
verify the binary exists before showing the app in the launcher.

## 🏗️ Build System (CMake)

### 14. Wayland Protocol Generation

**Gotcha**: Using unstable or custom Wayland protocols (like
`wlr-foreign-toplevel`) requires generating the client headers and private code
at build time. **Solution**: Use `pkg_check_modules` to find `wayland-scanner`
and add `add_custom_command` rules in `CMakeLists.txt` to automate header/source
generation before building the main executable.

---

## 📝 Documentation Quality

### 15. Markdown Linting in Nix Sandboxes

**Gotcha**: Running `markdownlint` inside a `nix flake check` environment often
fails to find `.markdownlint.json` because Nix Flakes only include git-tracked
files by default. Untracked config files result in `ENOENT`.

**Solution**:

- **Dynamic Config**: In `flake.nix`, use `pkgs.writeText` and `builtins.toJSON`
  to generate the configuration file on the fly.
- **Prettier Sync**: `markdownlint` and `prettier` sometimes disagree on
  list-marker spacing (MD030). If using `prettier` for formatting, it's often
  best to disable `MD030` in the linter to avoid "Formatting vs Linting" wars.
- **Language Tags**: `MD040` requires all fenced code blocks to have a language
  specified (e.g., ` ```bash `). Even plain text blocks should be tagged as
  ` ```text ` or ` ```plain `.

## 🏎️ Performance & Concurrency

### 16. Speed Exposes Race Conditions (Startup Timing)

**Gotcha**: Optimizing startup time (e.g., via a daemon) can break functionality
that accidentally relied on slow startup. In our case, the `WindowProvider`
previously had enough time to initialize and receive Wayland events while the
QML engine was slowly loading. When we made startup "instant" (~7ms), the
provider queried the window list before the compositor had sent any events,
resulting in an empty list.

**Solution**:

- **Never assume initial state is complete** in async protocols (Wayland).
- **Always connect signals** (e.g., `windowsChanged`) to UI refresh logic to
  handle data arriving _after_ initialization.
- **Lazy Loading**: Don't initialize heavy UI until needed but ensure data
  providers start listening immediately.

### 17. Background Daemons vs. Debugging

**Gotcha**: When debugging a client/server app, running the client with
`--debug` flags might connect to a SILENT background daemon (from a previous
run). Users (and developers) will see "command sent" and then nothing, assuming
the app is broken, when it's just the background process swallowing logs or
validation errors.

**Solution**:

- Implement a `kill` or `restart` command in the CLI.
- Ensure the client prints a clear "Connected to existing daemon (PID: X)"
  message.
- "Fail Loudly" if the daemon is unresponsive or version-mismatched.

### 18. Custom Wayland Connections & Qt Event Loop

**Gotcha**: If you use `wl_display_connect(nullptr)` to create a separate
Wayland connection (isolating your logic from Qt's internal connection), **Qt
will not pump events for you**. Calls to `wl_display_roundtrip` specifically
block and pump, but once you return to the main loop, your connection goes
silent.

**Solution**:

- Use `wl_display_get_fd()` to get the file descriptor.
- Wrap it in a `QSocketNotifier` monitoring `QSocketNotifier::Read`.
- Connect the notifier to a slot that calls `wl_display_dispatch()`.

---

Last updated: 2025-12-27
