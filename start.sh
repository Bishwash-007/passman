#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MODE="${PASSMAN_MODE:-local}"

case "${1:-}" in
    --local)
        MODE="local"
        shift
        ;;
    --dev|--development)
        MODE="development"
        shift
        ;;
esac

case "${MODE}" in
    local)
        BUILD_DIR="${ROOT_DIR}/build"
        CMAKE_ARGS=()
        ;;
    dev|development)
        BUILD_DIR="${ROOT_DIR}/build-development"
        CMAKE_ARGS=(
            -DCMAKE_BUILD_TYPE=Debug
            -DPASSMAN_ENABLE_SANITIZERS=ON
        )
        ;;
    *)
        printf 'Unsupported mode: %s\n' "${MODE}" >&2
        printf 'Use local (default) or development.\n' >&2
        exit 2
        ;;
esac

if ((${#CMAKE_ARGS[@]})); then
    cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" "${CMAKE_ARGS[@]}"
else
    cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}"
fi
cmake --build "${BUILD_DIR}" --parallel
"${BUILD_DIR}/passman" "$@"