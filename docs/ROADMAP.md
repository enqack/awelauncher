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

- [x] **Provider Sets** ([RFC-012](file:///home/sysop/Projects/awelauncher/docs/rfcs/012-provider-sets.md))
  - Define different workflows (e.g., `--set dev`, `--set web`) with distinct
    provider lists/ordering.
  - _Goal:_ Eliminate config-switching friction.
  - _Status:_ Shipped in v0.4.0.
- [x] **Pinned Results & Aliases** ([RFC-004](file:///home/sysop/Projects/awelauncher/docs/rfcs/004-pinned-results-aliases.md))
  - Allow explicit pinning of items to the top of results.
  - _Goal:_ Muscle memory for frequent actions.
  - _Status:_ Implemented in v0.4.4.
- [x] **Smarter Scoring** ([RFC-005](file:///home/sysop/Projects/awelauncher/docs/rfcs/005-smarter-scoring.md))
  - Hybrid fuzzy: Prefix > Subsequence > Acronym.
  - _Goal:_ More intuitive ranking.
  - _Status:_ Implemented in v0.4.4.
- [x] **Enhanced .desktop Handling** ([RFC-006](file:///home/sysop/Projects/awelauncher/docs/rfcs/006-xdg-desktop-enhancements.md), [RFC-007](file:///home/sysop/Projects/awelauncher/docs/rfcs/007-xdg-categories.md))
  - Respect `Hidden`/`NoDisplay`, parse keywords/categories, support Actions.
  - _Goal:_ Full XDG compliance and cleaner results.
  - _Status:_ Implemented in v0.4.4.
- [x] **Unified History** ([RFC-005](file:///home/sysop/Projects/awelauncher/docs/rfcs/005-smarter-scoring.md))
  - Extend MRU to `run` and `dmenu` modes.
  - _Goal:_ Consistent recall across modes.
  - _Status:_ Implemented in v0.4.4.

## Tier 2: Power User Features

_Focus: Structured extensibility._

- [x] **Service Mode (Daemon) + Socket Control** ([RFC-007](file:///home/sysop/Projects/awelauncher/docs/rfcs/007-service-mode.md))
  - Keep instance resident for < 10ms response.
  - Allow external control.
  - _Status:_ Shipped core in v0.4.4.
- [ ] **awelaunchctl - Dedicated Client** ([RFC-011](file:///home/sysop/Projects/awelauncher/docs/rfcs/011-awelaunchctl.md))
  - A handy wrapper for sending JSON commands to the daemon without manual JSON/socat.
  - _Goal:_ First-class scripting and automation.
- [ ] **Provider/Context Actions** ([RFC-008](file:///home/sysop/Projects/awelauncher/docs/rfcs/008-context-actions.md))
  - Right-click/Keybind actions for items (e.g., "Open in Terminal", "Move
    Window").
  - _Goal:_ Transform from "picker" to "workflow tool".
- [ ] **Clipboard Provider** ([RFC-009](file:///home/sysop/Projects/awelauncher/docs/rfcs/009-clipboard-provider.md))
  - Search/paste history.
- [ ] **Calculator / Unit Conv** ([RFC-010](file:///home/sysop/Projects/awelauncher/docs/rfcs/010-calculator-provider.md))
  - Built-in utility provider.
- [x] **Systems Utility Providers** ([RFC-013](file:///home/sysop/Projects/awelauncher/docs/rfcs/013-utility-providers.md))
  - Top, Kill, SSH.
  - _Goal:_ Replace shell scripts with native speed.
  - _Status:_ Implemented (Top, Kill, SSH) in v0.4.4.
- [x] **Visual Polish & UX** ([RFC-014](file:///home/sysop/Projects/awelauncher/docs/rfcs/014-visual-polish.md))
  - Context icons, breadcrumbs, smart empty states.
  - _Goal:_ Professional feel and user guidance.
  - _Status:_ Implemented in v0.4.4.
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

## The Road to 1.0.0: "The Complete Systems Launcher"

v1.0.0 represents the transition from a "fast picker" to a "stable system component." Key themes:

1.  **Semantic Stability**: The IPC protocol and plugin API are declared stable.
2.  **`awelaunchctl`**: The definitive way to interact with the launcher from scripts, WM configs, and keybinds.
3.  **Refined Discovery**: Fully compliant XDG handling, smarter scoring, and context-aware actions.
4.  **Operational Safety**: Robust error handling, memory bloat guards, and systemd integration.

---

## Prioritized Next Steps (MVP)

1. **Context Actions**: The biggest workflow enhancer.
2. **awelaunchctl**: Stabilizing the control plane.
3. **Smarter Scoring**: Finishing Tier 1 quality.
