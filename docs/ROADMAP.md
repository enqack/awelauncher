# awelauncher Roadmap

Based on the
[Strategic Analysis](https://github.com/enqack/awelauncher/issues/...) and
ecosystem review.

**Identity:**

> **awelauncher** = the fast, Wayland-native launcher with clean architecture,
> deterministic behavior, and modular growth — a "systems engineer’s launcher."

---

## Tier 1: Quality & Workflow Wins (MVP Candidates)

_Focus: Sharpening the tool without changing its identity._

- [x] **Provider Sets**
  - Define different workflows (e.g., `--set dev`, `--set web`) with distinct
    provider lists/ordering.
  - _Goal:_ Eliminate config-switching friction.
  - _Status:_ Shipped in v0.4.0.
- [ ] **Pinned Results & Aliases**
  - Allow explicit pinning of items to the top of results.
  - _Goal:_ Muscle memory for frequent actions.
- [ ] **Smarter Scoring**
  - Hybrid fuzzy: Prefix > Subsequence > Acronym.
  - _Goal:_ More intuitive ranking.
- [ ] **Enhanced .desktop Handling**
  - Respect `Hidden`/`NoDisplay`, parse keywords/categories, support Actions.
  - _Goal:_ Full XDG compliance and cleaner results.
- [ ] **Unified History**
  - Extend MRU to `run` and `dmenu` modes.
  - _Goal:_ Consistent recall across modes.

## Tier 2: Power User Features

_Focus: Structured extensibility._

- [ ] **Service Mode (Daemon) + Socket Control**
  - Keep instance resident for < 10ms response.
  - Allow external control (e.g., `awelaunchctl`).
- [ ] **Provider/Context Actions**
  - Right-click/Keybind actions for items (e.g., "Open in Terminal", "Move
    Window").
  - _Goal:_ Transform from "picker" to "workflow tool".
- [ ] **Clipboard Provider**
  - Search/paste history.
- [ ] **Calculator / Unit Conv**
  - Built-in utility provider.
- [x] Draft RFC-003: Monitor Destinations <!-- id: 73 -->
    - [x] Research LayerShell output selection <!-- id: 74 -->
    - [x] Write RFC-003 specification <!-- id: 75 -->
    - [x] Update ROADMAP.md with placement feature <!-- id: 76 -->
- [ ] **Emoji Picker / Input Modes**
  - File picker, emoji selection.

## Tier 3: Ecosystem Platform

_Focus: Long-term stability protocol._

- [ ] **External Provider Protocol**
  - JSON-over-stdin/stdout contract for plugins.
- [ ] **Scripting API**
  - Stable CLI for querying/controlling the launcher.
- [ ] **Cross-Compositor Abstraction**
  - Hyprland/Sway/Niri/Mangowc specific enhancements behind a stable interface.

---

## Prioritized Next Steps (MVP)

1. **Provider Sets**: The highest leverage for "pro" feel.
2. **Service Mode**: The ultimate performance feature.
3. **Context Actions**: The biggest workflow enhancer.
