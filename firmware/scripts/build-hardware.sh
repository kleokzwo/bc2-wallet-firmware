#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
project="$root/hardware/esp32s3_waveshare"
if ! command -v idf.py >/dev/null 2>&1; then
  echo "ESP-IDF ist nicht aktiv. Zuerst export.sh aus der ESP-IDF-Installation laden." >&2
  exit 1
fi
idf.py -C "$project" set-target esp32s3
idf.py -C "$project" build
