# RFC-014: Visual Polish & UX Details

## Status

- **Date**: 2025-12-26
- **Status**: Draft
- **Target Version**: v1.0.0

## Goal

Enhance the user experience by providing clear context awareness ("Where am I?")
and useful fallback actions ("What now?") when results are empty.

---

## 1. Context Awareness (Breadcrumbs)

With the introduction of **Provider Sets**, it is critical to indicates the
active context.

### Features

- **Prompt Icon**: Display a specific icon in the search bar (left of layout)
  representing the active Set or Provider.
  - _Config_: `sets.<name>.icon` (e.g. `icon: "utilities-terminal"` for Dev
    set).
- **Dynamic Placeholder**: Update the search input placeholder text to match the
  context.
  - _Config_: `sets.<name>.prompt` (already in Sets spec, but formalized here).
- **Right-Side Badge**: Optional badge on the right side of the input bar
  showing the current mode name (e.g. `[DEV]`).

### Config Schema Extension

```yaml
sets:
  dev:
    icon: "applications-development"
    prompt: "Code > "
    name: "Dev Mode" # displayed in badge?
```

---

## 2. Empty States

Improve the "No Results" experience from a blank list to a helpful state.

### Visuals

- **Icon/Message**: When the model is empty and query is non-empty, display a
  centered text/icon in the ListView area.
  - _Text_: "No matches found" (configurable).
  - _Icon_: `face-sad` or generic search icon.

### Smart Fallbacks (The "Actionable Empty State")

Instead of just showing "Nothing", offer to **do** something with the unknown
query.

- **Run in Terminal**: If `drun` finds nothing, insert a synthetic result:
  - _Text_: `Run '{query}' in terminal`
  - _Action_: Spawns terminal with command.
- **Web Search**: (Future)
  - _Text_: `Search google for '{query}'`

### Configuration

```yaml
general:
  empty_state:
    shown: true
    icon: "search-symbolic" # or "face-sad-symbolic" at 64px
    text: "No results"

  fallbacks:
    run_command: true # If true, adds "Run..." item at bottom or when empty
```

---

## Implementation Notes

- **QML**:
  - `LauncherRoot.qml` needs a `State` or simple `visible` logic for the
    EmptyPlaceholder component.
  - Search bar needs a new `Image` element for the Context Icon.
- **C++**:
  - `LauncherModel` needs to emit a `count` signal (standard) or `empty` state.
  - Fallbacks might be implemented as a special "FallbackProvider" that only
    returns items when the aggregator has 0 results.
