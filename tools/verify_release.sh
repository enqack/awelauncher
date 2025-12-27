#!/usr/bin/env bash
set -e

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}🔍 Verifying Release Candidate...${NC}"

# Detect project root
SCRIPT_DIR=$(dirname "$(realpath "$0")")
PROJECT_ROOT="$SCRIPT_DIR/.."
cd "$PROJECT_ROOT"

# 1. Version Check
echo -e "\n${BLUE}[1/4] Checking Version Consistency...${NC}"
if ./tools/bump_version.sh --check; then
    echo -e "${GREEN}✅ Version check passed${NC}"
else
    echo -e "${RED}❌ Version check failed${NC}"
    exit 1
fi

# 2. Build
echo -e "\n${BLUE}[2/4] Building with Nix...${NC}"
if nix build -L; then
    echo -e "${GREEN}✅ Build passed${NC}"
else
    echo -e "${RED}❌ Build failed${NC}"
    exit 1
fi

# 3. Automated Tests (Flake Check)
echo -e "\n${BLUE}[3/4] Running Automated Tests (Unit + E2E)...${NC}"
# 'nix flake check' runs the e2e_test defined in flake.nix
if nix flake check; then
    echo -e "${GREEN}✅ All tests passed (Nix Flake Check)${NC}"
else
    echo -e "${RED}❌ Tests failed${NC}"
    exit 1
fi

# 4. Smoke Test (Binary Launch)
echo -e "\n${BLUE}[4/4] Smoke Test (Binary Help Check)...${NC}"
if ./result/bin/awelaunch --help > /dev/null; then
    echo -e "${GREEN}✅ Binary executable (result/bin/awelaunch)${NC}"
else
    echo -e "${RED}❌ Binary failed to execute${NC}"
    exit 1
fi

echo -e "\n${GREEN}✨ RELEASE VERIFICATION SUCCESSFUL ✨${NC}"
echo "You are ready to tag and ship!"
