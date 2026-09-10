#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build-release"
STAGE_DIR="${ROOT_DIR}/.release-stage"
ARTIFACT_DIR="${ROOT_DIR}/release"
ARCHIVE="${ARTIFACT_DIR}/passman-1.0.0-macos-arm64.tar.gz"

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release
cmake --build "${BUILD_DIR}" --parallel
ctest --test-dir "${BUILD_DIR}" --output-on-failure

rm -rf "${ARTIFACT_DIR}" "${STAGE_DIR}"
mkdir -p "${ARTIFACT_DIR}" "${STAGE_DIR}/passman-1.0.0-macos-arm64"
cmake --install "${BUILD_DIR}" \
    --prefix "${STAGE_DIR}/passman-1.0.0-macos-arm64"
mv "${STAGE_DIR}/passman-1.0.0-macos-arm64/bin/passman" \
    "${STAGE_DIR}/passman-1.0.0-macos-arm64/passman"
rmdir "${STAGE_DIR}/passman-1.0.0-macos-arm64/bin"
cp "${ROOT_DIR}/README.md" \
    "${ROOT_DIR}/LICENSE" \
    "${ROOT_DIR}/SECURITY.md" \
    "${ROOT_DIR}/CHANGELOG.md" \
    "${STAGE_DIR}/passman-1.0.0-macos-arm64/"
tar -C "${STAGE_DIR}" -czf "${ARCHIVE}" passman-1.0.0-macos-arm64
shasum -a 256 "${ARCHIVE}" > "${ROOT_DIR}/SHA256SUMS"
rm -rf "${STAGE_DIR}"

printf 'Created %s\n' "${ARCHIVE}"
cat "${ROOT_DIR}/SHA256SUMS"
