#!/usr/bin/env bash

# Ensure we're in the project root
cd "$(dirname "$0")/.."

VERSION_FILE="VERSION"
README_FILE="README.md"

if [ ! -f "$VERSION_FILE" ]; then
    echo "Error: VERSION file not found!"
    exit 1
fi

CURRENT_VERSION=$(cat "$VERSION_FILE")

# Function to parse version string "X.Y.Z" into numbers
parse_version() {
    local version=$1
    IFS='.' read -r -a parts <<< "$version"
    echo "${parts[0]} ${parts[1]} ${parts[2]}"
}

# Get latest git tag
LATEST_TAG=$(git describe --tags --abbrev=0 --match "v*" 2>/dev/null || echo "v0.0.0")
TAG_VERSION=${LATEST_TAG#v} # Remove 'v' prefix

# Parse versions
read -r V_MAJOR V_MINOR V_PATCH <<< $(parse_version "$CURRENT_VERSION")
read -r T_MAJOR T_MINOR T_PATCH <<< $(parse_version "$TAG_VERSION")

# Compare logic: Convert to a comparable integer (assuming < 1000 for components)
V_NUM=$((V_MAJOR * 1000000 + V_MINOR * 1000 + V_PATCH))
T_NUM=$((T_MAJOR * 1000000 + T_MINOR * 1000 + T_PATCH))

# Suggestion logic
if [ $T_NUM -gt $V_NUM ]; then
    BASE_VERSION="$TAG_VERSION"
    BASE_MAJOR=$T_MAJOR
    BASE_MINOR=$T_MINOR
    BASE_PATCH=$T_PATCH
    WARNING="WARNING: Local VERSION ($CURRENT_VERSION) is behind latest git tag ($LATEST_TAG)."
else
    BASE_VERSION="$CURRENT_VERSION"
    BASE_MAJOR=$V_MAJOR
    BASE_MINOR=$V_MINOR
    BASE_PATCH=$V_PATCH
    WARNING=""
fi

NEXT_PATCH=$((BASE_PATCH + 1))
SUGGESTED_VERSION="$BASE_MAJOR.$BASE_MINOR.$NEXT_PATCH"

# Check mode
if [ "$1" == "--check" ]; then
    echo "VERSION file:         $CURRENT_VERSION"
    
    if [ -f "$README_FILE" ]; then
        if grep -q "version-$CURRENT_VERSION-blue.svg" "$README_FILE"; then
            echo "README.md badge:      MATCH"
        else
            echo "README.md badge:      MISMATCH"
            exit 1
        fi
    fi

    echo "Latest Git Tag:       $LATEST_TAG"
    
    if [ -n "$WARNING" ]; then
        echo "$WARNING"
    fi
    echo -e "\nSuggested Next:       $SUGGESTED_VERSION"

    exit 0
fi

if [ "$1" != "--bump" ]; then
    echo "Current version: $CURRENT_VERSION"
    echo "Latest tag:      $LATEST_TAG"
    echo "Suggested:       $SUGGESTED_VERSION"
    if [ -n "$WARNING" ]; then
        echo ""
        echo "$WARNING"
    fi
    echo ""
    echo "Usage: $0 [--check | --bump]"
    echo "  --check : Verify version consistency"
    echo "  --bump  : Increment to verified next version ($SUGGESTED_VERSION) and update files"
    exit 0
fi

echo "Bumping to: $SUGGESTED_VERSION"

# Update VERSION file
echo "$SUGGESTED_VERSION" > "$VERSION_FILE"

# Update README.md
if [ -f "$README_FILE" ]; then
    sed -i "s/version-$CURRENT_VERSION-blue.svg/version-$SUGGESTED_VERSION-blue.svg/" "$README_FILE"
    echo "Updated $README_FILE"
else
    echo "Warning: $README_FILE not found, skipping update."
fi

echo "Version bumped successfully to $SUGGESTED_VERSION"

# Verification
echo "Verifying changes..."
FETCHED_VERSION=$(cat "$VERSION_FILE")
if [ "$FETCHED_VERSION" != "$SUGGESTED_VERSION" ]; then
    echo "FAILED: VERSION file contains $FETCHED_VERSION, expected $SUGGESTED_VERSION"
    exit 1
fi

if [ -f "$README_FILE" ]; then
    if ! grep -q "version-$SUGGESTED_VERSION-blue.svg" "$README_FILE"; then
        echo "FAILED: README.md does not contain new version badge!"
        exit 1
    fi
fi
echo "Verification passed."
