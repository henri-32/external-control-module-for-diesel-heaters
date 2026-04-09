#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build-tests}"
UNITY_ROOT="${UNITY_ROOT:-third_party/unity}"

cmake -S . -B "${BUILD_DIR}" \
  -DHEIZUNG_ENABLE_TESTS=ON \
  -DUNITY_ROOT="${UNITY_ROOT}"

cmake --build "${BUILD_DIR}" --clean-first -j

ctest --test-dir "${BUILD_DIR}" --output-on-failure -V
