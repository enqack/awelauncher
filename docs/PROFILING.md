# Performance Profiling Results

## Profiling Infrastructure

Added startup timing instrumentation to measure performance bottlenecks.

### Profile Points

- **App init**: QGuiApplication creation and CLI parsing
- **Config loaded**: YAML config file parsing
- **Theme loaded**: Theme YAML parsing and color initialization
- **Items loaded**: Desktop file scanning (drun) or PATH scanning (run)
- **QML loaded**: QML engine initialization and UI rendering

### Usage

```bash
./build/awelaunch --profile
```

Profile output appears as:

```text
[PROFILE] App init : 5 ms
[PROFILE] Config loaded : 8 ms
[PROFILE] Theme loaded : 12 ms
[PROFILE] Items loaded : 145 ms
[PROFILE] QML loaded : 180 ms
```

## Performance Targets (from design-doc.md)

- **Open-to-first-paint**: < 50ms warm
- **Keystroke-to-updated-view**: < 16ms for 5k items

## Next Steps

1. Run profiling on actual system to identify bottlenecks
2. If desktop file scanning is slow (> 100ms):
   - Consider binary cache
   - Parallel scanning
   - Lazy loading
3. If still too slow, consider daemon architecture (`awelaunchd`)

## Optimization Ideas

- **Binary cache**: Serialize parsed items to `~/.cache/awelauncher/items.cache`
- **Parallel scanning**: Use QThreadPool for desktop file parsing
- **Lazy parsing**: Only parse `.desktop` files on-demand
- **Daemon mode**: Pre-load and cache items in background process
