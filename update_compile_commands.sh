#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="$(pwd)"
TARGET="$PROJECT_ROOT/compile_commands.json"

# mögliche Cache-Pfade
CANDIDATES=(
  "$HOME/.cache/arduino/sketches"
  "$HOME/snap/arduino-cli/current/.cache/arduino/sketches"
  "$HOME/snap/arduino-cli/62/.cache/arduino/sketches"
)

ARDUINO_CACHE=""

for dir in "${CANDIDATES[@]}"; do
  if [[ -d "$dir" ]]; then
    ARDUINO_CACHE="$dir"
    break
  fi
done

if [[ -z "$ARDUINO_CACHE" ]]; then
  echo "Arduino cache nicht gefunden."
  echo "Gesucht in:"
  printf '  %s\n' "${CANDIDATES[@]}"
  exit 1
fi

LATEST_BUILD_DIR=$(
  find "$ARDUINO_CACHE" -maxdepth 1 -type d \
    -printf '%T@ %p\n' \
  | sort -nr \
  | awk 'NR==1 {print $2}'
)

SRC="$LATEST_BUILD_DIR/compile_commands.json"

if [[ ! -f "$SRC" ]]; then
  echo "compile_commands.json nicht gefunden."
  echo "Bitte zuerst: arduino-cli compile"
  exit 1
fi

cp "$SRC" "$TARGET"
echo "compile_commands.json aktualisiert."
