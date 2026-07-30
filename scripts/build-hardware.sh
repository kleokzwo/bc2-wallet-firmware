#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
if ! command -v idf.py >/dev/null 2>&1; then
  echo "ESP-IDF fehlt. Waveshare verlangt ESP-IDF 5.5.0 oder neuer." >&2
  echo "Danach: source /opt/esp/idf/export.sh (Pfad ggf. anpassen)." >&2
  exit 2
fi
idf.py --version
cd "$ROOT/hardware/esp32s3_waveshare"
idf.py set-target esp32s3
idf.py build
