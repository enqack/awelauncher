# RFC 005: Smarter Scoring & Unified History

| Status      | Proposed              |
| :---------- | :-------------------- |
| **Author**  | System Engineer Agent |
| **Date**    | 2025-12-26            |
| **Version** | 0.5.0                 |

## Summary

Enhance the matching algorithm to provide more intuitive ranking and unify the
MRU (Most Recently Used) history across all providers (`run`, `dmenu`, `ssh`,
etc.).

## Motivation

The current fuzzy matcher is powerful but can be unpredictable. Users expect "f"
to prioritize "Firefox" (prefix) over "Google Chrome" (subsequence).
Additionally, history tracking is currently inconsistent between providers.

## Detailed Design

### 1. Hybrid Scoring Algorithm

We will move from a pure fuzzy score to a tiered scoring system:

| Tier | Match Type       | Priority | Example (Query: "fir")         |
| :--- | :--------------- | :------- | :----------------------------- |
| 1    | **Exact Match**  | Highest  | "fir" -> "fir" (binary)        |
| 2    | **Prefix Match** | High     | "fir" -> "**Fir**efox"         |
| 3    | **Acronym**      | Medium   | "st" -> "**S**ublime **T**ext" |
| 4    | **Subsequence**  | Low      | "on" -> "pyth**on**"           |

**Scoring Calculation**:
`FinalScore = (TierWeight * MatchQuality) + (MRU_Boost * Recency)`

### 2. Unified History (Global MRU)

Currently, MRU is often siloed by provider. We will move to a `history.json`
schema that tracks interactions globally.

```json
{
  "history": [
    { "id": "firefox.desktop", "count": 42, "last": 1735234567 },
    { "id": "ssh:myserver", "count": 12, "last": 1735234500 }
  ]
}
```

- **Persistence**: Switched from `mru.json` to a more structured `history.json`
  under `~/.cache/awelauncher/`.
- **Global Boost**: An item frequently launched in `drun` will still get a boost
  if it appears in a `run` or `set` search.

## Technical Implementation

- **`FuzzyMatcher.cpp`**: Refactor to return a `MatchResult` struct containing
  both the score and the match type/positions.
- **`MRUTracker.cpp`**: Update to handle global ID tracking. Use
  `QHash<QString, HistoryEntry>` for O(1) lookups during scoring.

## Open Questions

- **History Pruning**: Should we auto-delete entries older than 30 days to
  prevent growth? (Proposed: Yes, with a configurable limit).
