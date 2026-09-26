#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SOURCE_DIR="${ROOT_DIR}/custom"
QGC_DIR="${ROOT_DIR}/qgroundcontrol"
TARGET_DIR="${QGC_DIR}/custom"

if [[ ! -d "${SOURCE_DIR}" ]]; then
    echo "NEXUS custom source directory not found: ${SOURCE_DIR}" >&2
    exit 1
fi

if [[ ! -f "${QGC_DIR}/CMakeLists.txt" ]]; then
    echo "QGroundControl submodule is not initialized at: ${QGC_DIR}" >&2
    echo "Run: git submodule update --init --recursive" >&2
    exit 1
fi

rm -rf "${TARGET_DIR}"
mkdir -p "${TARGET_DIR}"
cp -a "${SOURCE_DIR}/." "${TARGET_DIR}/"

test -f "${TARGET_DIR}/CMakeLists.txt"
test -f "${TARGET_DIR}/cmake/CustomOverrides.cmake"
test -f "${TARGET_DIR}/src/NexusPlugin.h"
test -f "${TARGET_DIR}/src/NexusPlugin.cc"

echo "NEXUS custom overlay staged at ${TARGET_DIR}"
