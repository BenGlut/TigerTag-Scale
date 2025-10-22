
#!/usr/bin/env bash
set -euo pipefail
PORT="${1:-}"
if ! command -v esptool.py >/dev/null 2>&1; then
  echo "Installing esptool..."
  python3 -m pip install --user esptool
fi
CMD=(python3 -m esptool --chip esp32 --baud 921600 --before default_reset --after hard_reset write_flash -z)
if [ -n "$PORT" ]; then CMD+=("--port" "$PORT"); fi
"${CMD[@]}"   0x1000   ../firmware/bootloader.bin   0x8000   ../firmware/partitions.bin   0xE000   ../firmware/boot_app0.bin   0x10000  ../firmware/app.bin
echo "Done."
