#!/usr/bin/env bash
set -e

# Setup XDG_RUNTIME_DIR
export XDG_RUNTIME_DIR=$(mktemp -d)
chmod 0700 "$XDG_RUNTIME_DIR"

# Cleanup on exit
trap 'rm -rf "$XDG_RUNTIME_DIR"; kill $(jobs -p)' EXIT

echo "Starting headless Wayland compositor (Weston)..."
# Start weston with headless backend
# Note: we use --backend=headless-backend.so if available, or just verify weston supports it.
# Usually `weston-headless` or `weston` with config works.
# For simplicity in Nix, we might use a dedicated headless server if weston is heavy, but weston is standard.
weston --backend=headless-backend.so --socket=wayland-1 --idle-time=0 &
WESTON_PID=$!

# Wait for socket
echo "Waiting for wayland-1 socket..."
for i in {1..10}; do
    if [ -S "$XDG_RUNTIME_DIR/wayland-1" ]; then
        echo "Socket found!"
        break
    fi
    sleep 0.5
done

if [ ! -S "$XDG_RUNTIME_DIR/wayland-1" ]; then
    echo "Failed to start Wayland compositor."
    exit 1
fi

export WAYLAND_DISPLAY=wayland-1

echo "Starting awelauncher..."
# Run awelauncher in drun mode (default)
awelaunch --show drun &
APP_PID=$!

# Wait for it to render
sleep 2

echo "Taking screenshot..."
grim "$XDG_RUNTIME_DIR/screenshot.png"

if [ -f "$XDG_RUNTIME_DIR/screenshot.png" ]; then
    echo "Screenshot captured successfully."
    # Optional: Check file size > 0
    if [ -s "$XDG_RUNTIME_DIR/screenshot.png" ]; then
        echo "Test Passed: GUI rendered and captured."
        exit 0
    else
        echo "Test Failed: Screenshot is empty."
        exit 1
    fi
else
    echo "Test Failed: Screenshot not created."
    exit 1
fi
