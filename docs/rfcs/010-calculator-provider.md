# RFC 010: Calculator & Unit Conversion

| Status      | Proposed              |
| :---------- | :-------------------- |
| **Author**  | System Engineer Agent |
| **Date**    | 2025-12-26            |
| **Version** | 0.5.0                 |

## Summary

Implement a built-in utility provider that evaluates mathematical expressions
and performs unit conversions (currency, distance, weight, etc.) in real-time.

## Motivation

Users often need quick calculations or conversions without opening a full
calculator app or a browser. `awelauncher` can provide this as a "top-level"
result.

## Detailed Design

### 1. Interception Logic

The `CalcProvider` will attempt to parse the query. If it successfully parses
as a math expression or a conversion request, it returns a synthetic result.

- **Examples**:
  - `5 * 4 + 2` -> `Result: 22`
  - `sqrt(144)` -> `Result: 12`
  - `100 eur to usd` -> `Result: $105.40`
  - `10km to miles` -> `Result: 6.21 miles`

### 2. Implementation Options

#### Option A: `QJSEngine` (Easy)
- Use Qt's built-in JavaScript engine to evaluate math.
- *Pros*: Very flexible, supports functions (`Math.sin`, etc.).
- *Cons*: Heavier than a simple parser.

#### Option B: `libqalculate` (Pro)
- Link against the industry-standard library.
- *Pros*: Powerful, handles units and currency natively.
- *Cons*: Additional heavy dependency.

**Decision**: **Option A** for math, with a simple internal multiplier table
for basic units. If power is needed later, we can move to `libqalculate`.

### 3. Interaction

- **Visuals**: The result is shown as the first item with a `calculator` icon.
- **Action**: `Return` copies the result to the clipboard.

## Technical Implementation

- **`CalcProvider`**: High-priority provider that triggers only on specific
  regex matches (e.g. `^[0-9+*/(). -]+$`).
- **Currency**: Requires an optional background fetch of exchange rates (e.g.
  cached daily).

## Open Questions

- **Ambiguity**: How to distinguish a math query from a search for a file named
  `1+1.txt`?
  - _Proposed_: Calculator results always have a distinct `calculator` icon.
