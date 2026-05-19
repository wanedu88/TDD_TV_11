#!/usr/bin/env bash
# Regenerate Golden Master approved files.
# Usage (from repo root): ./scripts/update_golden.sh
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

export TV_UPDATE_GOLDEN=1
./build/TVControllerGoldenTest

echo "Golden files updated under test/golden/approved/"
echo "Review diff, then commit *.approved.txt"
