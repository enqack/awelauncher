# awelauncher Roadmap

Based on the [Strategic Analysis](https://github.com/enqack/awelauncher/issues/...) and ecosystem review.

**Identity:**
> **awelauncher** = the fast, Wayland-native launcher with clean architecture, deterministic behavior, and modular growth — a "systems engineer’s launcher."

---

## Tier 1: Quality & Workflow Wins (MVP Candidates)
*Focus: Sharpening the tool without changing its identity.*

- [ ] **Provider Sets**
  - Define different workflows (e.g., `--set dev`, `--set web`) with distinct provider lists/ordering.
  - *Goal:* Eliminate config-switching friction.
- [ ] **Pinned Results & Aliases**
  - Allow explicit pinning of items to the top of results.
  - *Goal:* Muscle memory for frequent actions.
- [ ] **Smarter Scoring**
  - Hybrid fuzzy: Prefix > Subsequence > Acronym.
  - *Goal:* More intuitive ranking.
- [ ] **Enhanced .desktop Handling**
  - Respect `Hidden`/`NoDisplay`, parse keywords/categories, support Actions.
  - *Goal:* Full XDG compliance and cleaner results.
- [ ] **Unified History**
  - Extend MRU to `run` and `dmenu` modes.
  - *Goal:* Consistent recall across modes.

## Tier 2: Power User Features
*Focus: Structured extensibility.*

- [ ] **Service Mode (Daemon) + Socket Control**
  - Keep instance resident for < 10ms response.
  - Allow external control (e.g., `awelaunchctl`).
- [ ] **Provider/Context Actions**
  - Right-click/Keybind actions for items (e.g., "Open in Terminal", "Move Window").
  - *Goal:* Transform from "picker" to "workflow tool".
- [ ] **Clipboard Provider**
  - Search/paste history.
- [ ] **Calculator / Unit Conv**
  - Built-in utility provider.
- [ ] **Input Modes**
  - Emoji picker, File picker.

## Tier 3: Ecosystem Platform
*Focus: Long-term stability protocol.*

- [ ] **External Provider Protocol**
  - JSON-over-stdin/stdout contract for plugins.
- [ ] **Scripting API**
  - Stable CLI for querying/controlling the launcher.
- [ ] **Cross-Compositor Abstraction**
  - Hyprland/Sway specific enhancements behind a stable interface.

---

## Prioritized Next Steps (MVP)

1. **Provider Sets**: The highest leverage for "pro" feel.
2. **Service Mode**: The ultimate performance feature.
3. **Context Actions**: The biggest workflow enhancer.
