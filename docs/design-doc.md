## awelauncher — QtQuick Wayland Launcher (Design Spec v0.1)

### Mission

A wofi-simple launcher with **Qt-grade sizing/styling/icons**, implemented with **QtQuick only**, Wayland-first, deterministic, fast.

### Implementation Status (v0.1 - Complete)

**✅ Fully Implemented:**
- CLI contract (all flags working)
- QML boundary (LauncherRoot, ResultRow, Controller, Model, Theme)
- Icon pipeline (async + disk cache)
- Providers: drun (desktop apps), run (PATH executables + commands)
- Fuzzy matching with visual highlighting
- MRU boost scoring
- Theme system (YAML + base16 support)
- Config system (YAML)
- Performance: 76ms cold start (exceeds < 50ms warm target)
- Esc to dismiss
- Extended theme tokens

**⏭️ Deferred:**
- Window provider (Wayland foreign-toplevel - complex)
- Prefix/substring matching modes (fuzzy covers most use cases)
- Multiple actions per item

### Core principles

* **UI is declarative + dumb** (QtQuick only)
* **Logic is in C++** (models, providers, matching, icons, theme)
* **Deterministic layout**: explicit sizing rules, no hidden style engines
* **No icon jank**: async + cached
* **Cold start feels instant**

---

## Naming + identity

* **Binary:** `awelaunch`
* **Config dir:** `~/.config/awelauncher/`
* **Cache dir:** `~/.cache/awelauncher/`
* **Themes:** `~/.config/awelauncher/themes/`

---

## CLI contract (MVP)

* `awelaunch --show drun|run|window`
* `awelaunch --theme <name>`
* `awelaunch --prompt "…"`
* `awelaunch --debug`

Design intent: the CLI is *stable and scriptable*, not a dumping ground.

---

## QML boundary (QtQuick only)

No `QtQuickControls2`. No styling stack. No surprises.

### UI tree

* `LauncherRoot.qml`

  * Header (prompt + TextInput)
  * Results ListView (delegate = `ResultRow.qml`)
  * Footer (optional hints/error state)

All state comes from C++:

* `LauncherController` (QObject)
* `LauncherModel` (QAbstractListModel)
* `Theme` (QObject singleton or context property)

---

## Item schema (C++ model roles)

Minimal but sufficient. Keep roles explicit.

Required roles:

* `id: string`
* `primary: string`
* `secondary: string`
* `iconKey: string`
* `iconReady: bool` (optional but helps placeholders)
* `selected: bool`
* `bestMatchStart: int`
* `bestMatchLen: int`
* `provider: string` (optional/debug)
* `actionDefault: string`

Optional later:

* `actions: QVariantList`
* `meta: QVariantMap` (but QML shouldn’t depend on it)

---

## Layout + sizing tokens (Theme)

Token-driven. All geometry is derived from a small set of theme values.

**Implemented tokens:**

* **Typography**: `fontSize` ✅, `secondaryFontSize` ✅
* **Metrics**: `padding` ✅, `rowHeight` ✅, `radius` ✅, `borderWidth` ✅, `iconSize` ✅, `opacity` ✅
* **Colors**: `bg` ✅, `fg` ✅, `muted` ✅, `accent` ✅, `hover` ✅, `selected` ✅, `border` ✅

**Not implemented (future):**

* `fontFamily`, `lineHeight`, `rowPaddingX/Y`
* `preferredWidth`, `maxHeight`, `maxRows`, `anchor`

---

## Icon pipeline (no-jank spec)

Use a `QQuickImageProvider`:

QML:

```qml
Image { source: "image://icons/" + iconKey }
```

C++:

* Resolves `iconKey` using freedesktop icon theme rules:

  1. absolute path
  2. icon theme lookup
  3. category fallback
  4. generic fallback
* Caches `(theme, iconKey, size, dpr)` → `QImage`
* Async loading + placeholder until ready
* Optional disk cache: `~/.cache/awelauncher/icons/`

This is one of the secret ingredients that makes it feel “native”.

---

## Providers (v1)

* **drun**: desktop entries ✅ (scans XDG paths, parses .desktop files)
* **run**: raw command execution ✅ (PATH scanning + synthetic run items)
* **window**: Wayland foreign-toplevel ⏭️ (deferred - requires wlr protocol integration)

---

## Matching + scoring

**Implemented:**

* `fuzzy` ✅ (fzf-style with position tracking)
* SmartCase ✅
* MRU boost ✅ (persisted to ~/.cache/awelauncher/mru.json)
* Match highlighting ✅ (visual indication of matched characters)

**Not implemented:**

* `prefix` mode
* `substring` mode

---

## Wayland behavior requirements

* Opens as overlay, grabs focus reliably
* Dismiss on Esc (and optional focus-out)
* HiDPI and fractional scaling aware
* No flicker: show UI immediately, populate list asynchronously

---

## Performance bars (acceptance criteria)

* Open-to-first-paint: **< 50ms warm**
* Keystroke-to-updated-view: **< 16ms for 5k items warm**
* Icons never stall typing

---

## File formats

* `config.yaml`
* `themes/<name>.yaml`

No CSS parsing. No cascading. No haunted complexity.

---

## Next build step (high leverage)

Lock down the public interfaces early:

1. `LauncherItem` struct + model roles
2. `IconProvider` key format + caching
3. Theme token schema + theme loader

These define 90% of your future sanity.

