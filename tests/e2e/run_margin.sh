#!/usr/bin/env bash
set -e

# Setup XDG_RUNTIME_DIR
export XDG_RUNTIME_DIR=$(mktemp -d)
chmod 0700 "$XDG_RUNTIME_DIR"
export XDG_CONFIG_HOME="$XDG_RUNTIME_DIR/config"
mkdir -p "$XDG_CONFIG_HOME/awelauncher"

# Cleanup on exit
trap 'rm -rf "$XDG_RUNTIME_DIR"; kill $(jobs -p)' EXIT

echo "Starting headless Wayland compositor (Weston)..."
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

echo "DEBUG: QML2_IMPORT_PATH=$QML2_IMPORT_PATH"
echo "DEBUG: QT_PLUGIN_PATH=$QT_PLUGIN_PATH"

echo "Running BASIC launch (no config overrides)..."
# Ensure no config
rm -f "$XDG_CONFIG_HOME/awelauncher/config.yaml"

export QT_DEBUG_PLUGINS=0
./build/awelaunch --show drun &
PID_BASIC=$!
sleep 2
if ps -p $PID_BASIC > /dev/null; then
   echo "Basic launch RUNNING."
   kill $PID_BASIC
else
   echo "Basic launch CRASHED."
   exit 1
fi

echo "Creating margin config..."
# Top anchor with large margin
cat > "$XDG_CONFIG_HOME/awelauncher/config.yaml" <<EOF
window:
  anchor: "top"
  margin: 100
  width: 500
  height: 300
EOF

echo "Starting awelauncher..."
# Assuming build exists
./build/awelaunch &
APP_PID=$!

sleep 2

echo "Taking screenshot..."
grim "$XDG_RUNTIME_DIR/margin_test.png"

# Copy out to project dir for inspection
cp "$XDG_RUNTIME_DIR/margin_test.png" ./margin_test.png

echo "Done. Screenshot saved to ./margin_test.png"
