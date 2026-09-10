#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${ROOT_DIR}/build"

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -DCMAKE_CXX_COMPILER=clang++
cmake --build "${BUILD_DIR}" --parallel
ctest --test-dir "${BUILD_DIR}" --output-on-failure
"${BUILD_DIR}/passman" --help >/dev/null

printf 'All tests passed.\n'