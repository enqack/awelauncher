# Spec: Provider Sets Configuration

## Goal

Allow users to define named "Sets" of providers/modes. Instead of just running
individual modes (`--show drun`), a user can switch contexts (`--set dev`,
`--set media`) which might combine multiple providers or apply specific filters.

## Usage

```bash
awelaunch --set <name>
```

If no arguments are provided, `awelaunch` defaults to the `drun` provider
(behavior consistent with previous versions).

## Configuration Schema

A new top-level `sets` dictionary in `config.yaml`.

```yaml
general:
  theme: "auto"

sets:
  # Example 1: Developer Set
  # Combines multiple providers into one unified list.
  dev:
    providers: [run, drun, window]
    prompt: "Dev 🚀 > "

  # Example 2: Media Set
  # Only shows windows, specific app filters, unique layout.
  media:
    providers: [window]
    prompt: "Focus > "

    # [Decision] Per-Set Layout Overrides
    layout:
      width: 600
      height: 400
      anchor: center

    # [Decision] Filter Syntax (Include/Exclude)
    filter:
      # Whitelist: Item MUST match at least one rule here (if present)
      include:
        app_id: ["/spotify/", "mpv"] # Regex /regex/ or substring
        title: ["VLC"]

      # Blacklist: Item matching ANY rule here is dropped (takes precedence)
      exclude:
        title: ["/Picture-in-Picture/"]
```

## Behavior Specifications

### 1. Unified Results

When a set defines multiple providers (e.g. `[run, window]`), results are
**unified** into a single flat list.

- **Implication:** `LauncherModel` aggregates results from multiple providers
  simultaneously.
- **Ranking:** Results are scored and sorted globally (MRU + Fuzzy Score),
  regardless of origin provider.

### 2. Filtering Logic

Filters are first-class citizens defined under a `filter` key.

- **Syntax:**
  - Strings wrapped in `/.../` are treated as **Regex** (e.g., `"/^Spotify$/"`).
  - Normal strings are treated as **Substring** match.
- **Precedence:**
  1.  `exclude` rules are checked first. If matched -> Drop.
  2.  If `include` rules exist, check if matched. If not matched -> Drop.
  3.  Otherwise -> Keep.

### 3. Layout Overrides

Any key valid in the global `layout` or `window` config sections can be
overridden inside a set. This allows "mini-launchers" or "full-screen
dashboards" based on context.

## Implementation Plan

1.  **Config Schema**: Update `Config.cpp` to parse the `sets` dictionary and
    its sub-structures (`filter`, `layout`).
2.  **Core Architecture**:
    - Refactor `LauncherModel` to support multiple active providers
      (`std::vector<Provider*>`).
    - Implement "Aggregator" logic in Model to merge and sort results from all
      sources.
3.  **Controller**: Update `--set` CLI handling to:
    - Load the specified set config.
    - Instantiate/Activate the correct group of providers.
    - Apply layout overrides.
