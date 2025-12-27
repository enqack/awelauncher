# RFC-007: XDG Category Support

## Status

- **Status**: Implemented
- **Type**: Feature

## Context

XDG `.desktop` files often contain a `Categories` key (e.g.,
`Categories=Game;ArcadeGame;`). Users expect to be able to search for these
categories to find groups of applications, such as all games or all settings
panels.

## Specification

### 1. Categories Parsing

The `DesktopFileLoader` must parse the `Categories` key from the
`[Desktop Entry]` group.

- The string is semicolon-separated.
- It should be normalized (semicolons replaced with spaces) to allow fuzzy
  matching against individual category names.

### 2. Matching Logic

The `FuzzyMatcher` should include the `categories` field in its scoring
algorithm.

- **Priority**: Lower than `Name` (App Title) and `Keywords` but sufficient to
  surface results when the user explicitly searches for a category.
- **Example**: Searching "Game" should list applications with
  `Categories=...Game...`.

### 3. Data Model

`LauncherItem` struct is updated to include:

```cpp
QString categories;
```

## Implementation Details

- **Loader**: `DesktopFileLoader.cpp` updated to read `Categories`.
- **Model**: `LauncherModel.h` updated struct.
- **Matching**: `LauncherModel.cpp` filter logic updated to check
  `item.categories`.
