#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="$(pwd)"
TARGET="$PROJECT_ROOT/compile_commands.json"

# Windows -> POSIX Pfad (Git Bash)
win_to_posix() {
  # returns empty if input empty
  [[ -z "${1:-}" ]] && { echo ""; return; }
  cygpath -u "$1"
}

LOCALAPPDATA_POSIX="$(win_to_posix "${LOCALAPPDATA:-}")"
TEMP_POSIX="$(win_to_posix "${TEMP:-}")"

# Kandidaten: Arduino IDE 2.x Cache unter Windows ist typischerweise unter %LOCALAPPDATA%\arduino
# Je nach Version/Setup gab es auch %LOCALAPPDATA%\Temp\arduino bzw. %TEMP%\arduino. :contentReference[oaicite:1]{index=1}
CANDIDATES=()

# IDE / CLI (häufig)
if [[ -n "$LOCALAPPDATA_POSIX" ]]; then
  CANDIDATES+=("$LOCALAPPDATA_POSIX/arduino/sketches")
  CANDIDATES+=("$LOCALAPPDATA_POSIX/Temp/arduino/sketches")
fi

# TEMP-Varianten
if [[ -n "$TEMP_POSIX" ]]; then
  CANDIDATES+=("$TEMP_POSIX/arduino/sketches")
  CANDIDATES+=("$TEMP_POSIX/sketches")
fi

# Fallbacks (manchmal existieren diese)
CANDIDATES+=("$HOME/AppData/Local/arduino/sketches")
CANDIDATES+=("$HOME/AppData/Local/Temp/arduino/sketches")

ARDUINO_CACHE=""
for dir in "${CANDIDATES[@]}"; do
  if [[ -d "$dir" ]]; then
    ARDUINO_CACHE="$dir"
    break
  fi
done

if [[ -z "$ARDUINO_CACHE" ]]; then
  echo "Arduino build-cache nicht gefunden."
  echo "Gesucht in:"
  printf '  %s\n' "${CANDIDATES[@]}"
  echo
  echo "Tipp: Einmal kompilieren (IDE: Verify / CLI: arduino-cli compile), dann erneut ausführen."
  exit 1
fi

# Neueste Build-Dir bestimmen (robust unter Git Bash)
LATEST_BUILD_DIR="$(ls -1dt "$ARDUINO_CACHE"/*/ 2>/dev/null | head -n 1 || true)"

if [[ -z "$LATEST_BUILD_DIR" ]]; then
  echo "Keine Build-Verzeichnisse unter: $ARDUINO_CACHE"
  echo "Bitte zuerst kompilieren (IDE: Verify / CLI: arduino-cli compile)."
  exit 1
fi

SRC="${LATEST_BUILD_DIR%/}/compile_commands.json"

if [[ ! -f "$SRC" ]]; then
  echo "compile_commands.json nicht gefunden in:"
  echo "  $LATEST_BUILD_DIR"
  echo
  echo "Hinweis: Dieses Script erwartet, dass deine Kompilation eine compile_commands.json erzeugt."
  echo "Wenn du das nicht siehst: nutze arduino-cli (oder stelle sicher, dass dein Workflow sie erzeugt)."
  exit 1
fi

cp "$SRC" "$TARGET"
echo "compile_commands.json aktualisiert:"
echo "  Quelle: $SRC"
echo "  Ziel:   $TARGET"
