# Example Configuration

This directory contains example configuration files for awelauncher.

## Installation

Copy these files to your config directory:

```bash
mkdir -p ~/.config/awelauncher/themes
cp examples/config/config.yaml ~/.config/awelauncher/
cp examples/config/themes/* ~/.config/awelauncher/themes/
```

## Configuration

### config.yaml

- `general.theme`: Theme name to load, or "auto" for base16 system theme
- `window.width`, `window.height`: Window dimensions

### Theme Files

Theme files define colors and layout properties:

**Colors:**
- `bg`: Background color
- `fg`: Foreground/text color
- `accent`: Accent color for highlights
- `selected`: Selected item background
- `muted`: Muted/secondary text color

**Layout:**
- `radius`: Corner radius (px)
- `padding`: Window padding (px)
- `rowHeight`: Result row height (px)
- `fontSize`: Text size (px)
- `iconSize`: Icon size (px)
- `opacity`: Window transparency (0.0 - 1.0)

## Base16 System Theme

Set `theme: "auto"` to automatically use your base16-shell colors.
Requires `BASE16_COLOR_*_HEX` environment variables.
