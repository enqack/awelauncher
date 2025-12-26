# awelauncher — QtQuick Wayland Launcher (Design Spec v0.3.1)

## Mission

A wofi-simple launcher with **Qt-grade sizing/styling/icons**, implemented with
**QtQuick only**, Wayland-first, deterministic, fast.

### Implementation Status (v0.4.2 - Current)

**✅ Fully Implemented:**

- **CLI/Metadata**: unified versioning (`VERSION` file), `--version` flag, and
  config/theme overrides.
- **QML boundary**: LauncherRoot, ResultRow, Controller, Model, Theme (QtQuick
  only).
- **Icon pipeline**: async loading + placeholder + disk caching.
- **Providers**:
  - **drun**: desktop apps (XDG parsed).
  - **run**: PATH executables.
  - **window**: window switcher with activation, actions (close, fullscreen,
    etc.).
  - **dmenu**: stdin/stdout pipe mode.
  - **top**: process monitoring (CPU/Memory sorting) (New in 0.4.0).
  - **kill**: process killer (New in 0.4.0).
  - **ssh**: SSH host launcher (New in 0.4.0).
- **Provider Sets**: Group providers, prompts, and filters into named sets (e.g.
  `dev`, `media`) (Tier 1).
- **Monitor Selection**: secondary selection flow (`Ctrl+M`) to move windows
  between outputs.
- **Advanced Positioning**: anchoring (top/bottom/left/right/center), margins,
  and CLI geometry overrides.
- **Matching**: fuzzy with visual highlighting + MRU boost scoring.
- **Build System**: generator-agnostic Taskfile + Nix flake integration.
- **Interaction**: Esc to dismiss, help overlay (`Ctrl+H`).

**⏭️ Deferred:**

- Multiple actions per item menu.
- Prefix/substring matching modes (fuzzy covers most use cases).

**🚧 Planned (v0.5+):**

- **Plugin System**: External provider binary API.
- **Workspace movement** (requires protocol support).
- **Man/Apropos Provider**: System documentation search with `apropos` indexing
  (keyword/summary) and `man` viewer.
- **Journal/Logs Provider**: View system logs (`journalctl`) directly in the
  interface.

### Core principles

- **UI is declarative + dumb** (QtQuick only)
- **Logic is in C++** (models, providers, matching, icons, theme)
- **Deterministic layout**: explicit sizing rules, no hidden style engines
- **No icon jank**: async + cached
- **Cold start feels instant**

---

## Naming + identity

- **Binary:** `awelaunch`
- **Config dir:** `~/.config/awelauncher/`
- **Cache dir:** `~/.cache/awelauncher/`
- **Themes:** `~/.config/awelauncher/themes/`

---

## CLI contract (Stable)

- `awelaunch --show drun|run|window|top|kill|ssh|dmenu`
- `awelaunch --set <provider_set_name>`
- `awelaunch --version`
- `awelaunch --theme <name>`
- `awelaunch --prompt "…"`
- `awelaunch --dmenu` (alias for `--show dmenu`)
- `awelaunch --width <int> --height <int>`
- `awelaunch --anchor <center|top|bottom|left|right>`
- `awelaunch --margin <int>`
- `awelaunch --overlay` (Force overlay layer)
- `awelaunch --debug`

Design intent: the CLI is _stable and scriptable_, not a dumping ground.

---

## QML boundary (QtQuick only)

No `QtQuickControls2`. No styling stack. No surprises.

### UI tree

- `LauncherRoot.qml`
  - Header (prompt + TextInput)
  - Results ListView (delegate = `ResultRow.qml`)
  - Footer (optional hints/error state)

All state comes from C++:

- `LauncherController` (QObject)
- `LauncherModel` (QAbstractListModel)
- `Theme` (QObject singleton or context property)

---

## Item schema (C++ model roles)

Minimal but sufficient. Keep roles explicit.

Required roles:

- `id: string`
- `primary: string`
- `secondary: string`
- `iconKey: string`
- `exec: string` (empty for window mode)
- `terminal: bool`
- `selected: bool`
- `matchPositions: array<int>` (for fuzzy match highlighting)
- `provider: string` (optional/debug)

Optional later:

- `actions: QVariantList`
- `meta: QVariantMap` (but QML shouldn’t depend on it)

---

## Layout + sizing tokens (Theme)

Token-driven. All geometry is derived from a small set of theme values.

**Implemented tokens:**

- **Typography**: `fontSize` ✅, `secondaryFontSize` ✅
- **Metrics**: `padding` ✅, `rowHeight` ✅, `radius` ✅, `borderWidth` ✅,
  `iconSize` ✅, `opacity` ✅
- **Positioning**: `anchor` (v0.2)
- **Colors**: `bg` ✅, `fg` ✅, `muted` ✅, `accent` ✅, `hover` ✅, `selected`
  ✅, `border` ✅

**Not implemented (future):**

- `fontFamily`, `lineHeight`, `rowPaddingX/Y`
- `preferredWidth`, `maxHeight`, `maxRows`, `anchor`

---

## Icon pipeline (no-jank spec)

Use a `QQuickImageProvider`:

QML:

```qml
Image { source: "image://icons/" + iconKey }
```

C++:

- Resolves `iconKey` using freedesktop icon theme rules:
  1. absolute path
  2. icon theme lookup
  3. category fallback
  4. generic fallback

- Caches `(theme, iconKey, size, dpr)` → `QImage`
- Async loading + placeholder until ready
- Optional disk cache: `~/.cache/awelauncher/icons/`

This is one of the secret ingredients that makes it feel “native”.

---

## Providers (v1)

- **drun**: desktop entries ✅ (scans XDG paths, parses .desktop files)
- **run**: raw command execution ✅ (PATH scanning + synthetic run items)
- **window**: Wayland foreign-toplevel ✅
  (wlr-foreign-toplevel-management-unstable-v1)
  - Enumerates all toplevel windows
  - Window actions: activate, close, fullscreen, maximize, minimize
  - **Monitor Selection**: `Ctrl+M` triggers a secondary list of outputs to move
    the window.
  - State tracking: title, app_id, state flags.
  - Tested on Niri, Sway, and Hyprland.
- **dmenu**: stdin provider ✅
  - Reads lines from stdin.
  - Filters based on input.
  - Prints selection to stdout.

---

## Matching + scoring

**Implemented:**

- `fuzzy` ✅ (fzf-style with position tracking)
- SmartCase ✅
- MRU boost ✅ (persisted to ~/.cache/awelauncher/mru.json)
- Match highlighting ✅ (visual indication of matched characters)

**Not implemented:**

- `prefix` mode
- `substring` mode

---

## Wayland behavior requirements

- Opens as overlay, grabs focus reliably
- Dismiss on Esc (and optional focus-out)
- HiDPI and fractional scaling aware
- No flicker: show UI immediately, populate list asynchronously

---

## Performance bars (acceptance criteria)

- Open-to-first-paint: **< 50ms warm**
- Keystroke-to-updated-view: **< 16ms for 5k items warm**
- Icons never stall typing

---

## File formats

- `config.yaml`
- `themes/<name>.yaml`

No CSS parsing. No cascading. No haunted complexity.

---

## Next build step (high leverage)

Lock down the public interfaces early:

1. `LauncherItem` struct + model roles
2. `IconProvider` key format + caching
3. Theme token schema + theme loader

These define 90% of your future sanity.
