# awelauncher

A fast, minimal Wayland launcher built with Qt6 and QtQuick.

## Features

- **Fuzzy search** with visual match highlighting
- **Multiple modes**: Desktop apps (drun), command runner (run)
- **MRU boost**: Recently used apps appear higher in results
- **Icon caching**: Async loading with disk cache for instant startup
- **Theming**: YAML-based themes + base16 system theme support
- **Performance**: 76ms cold start, < 16ms search updates
- **Wayland-first**: Native Wayland support with Qt6

## Building

### With Nix (recommended)

```bash
nix develop
cd build
cmake ..
make
```

### Manual build

Requirements:
- Qt6 (Quick, Gui, WaylandClient)
- yaml-cpp
- CMake 3.16+
- C++17 compiler

```bash
mkdir build && cd build
cmake ..
make
```

## Installation

```bash
# From build directory
sudo make install

# Or with Nix
nix build
```

## Configuration

Example configs are in `examples/config/`. Copy to `~/.config/awelauncher/`:

```bash
mkdir -p ~/.config/awelauncher/themes
cp examples/config/config.yaml ~/.config/awelauncher/
cp examples/config/themes/*.yaml ~/.config/awelauncher/themes/
```

## Usage

```bash
# Launch desktop app picker
awelaunch --show drun

# Launch command runner
awelaunch --show run

# Use custom theme
awelaunch --theme catppuccin

# Enable debug output
awelaunch --debug
```

### Keybindings

- `Esc` - Dismiss launcher
- `Enter` - Launch selected item
- `↑/↓` - Navigate results
- Type to fuzzy search

## Architecture

- **Controllers**: `LauncherController` - handles app launching
- **Models**: `LauncherModel` - manages items and search filtering
- **Providers**: Desktop file loader, icon provider
- **Utils**: Config, theme, fuzzy matcher, MRU tracker

See `docs/design-doc.md` for full design specification.

## Performance

- **Startup**: 76ms cold (exceeds < 50ms warm target)
- **Search**: < 16ms for 5k items
- **Icons**: Async + disk cache, never blocks typing

## License

MIT (or your preferred license)

## Credits

Built with Qt6, inspired by wofi and rofi.
