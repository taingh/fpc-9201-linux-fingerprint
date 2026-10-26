#!/usr/bin/env bash
# Remove the fingerpp fprintd drop-in and /opt/fingerpp, then restart fprintd.
# Idempotent: a second run just restarts fprintd again. Requires root (sudo).
set -euo pipefail

readonly DROPIN_DIR=/etc/systemd/system/fprintd.service.d
readonly DROPIN="${DROPIN_DIR}/override.conf"

if [[ "${EUID}" -ne 0 ]]; then
  printf '%s\n' "This uninstaller needs root. Run: sudo $0" >&2
  exit 1
fi

have_fprintd_unit=0
if command -v systemctl >/dev/null 2>&1 && systemctl cat fprintd.service >/dev/null 2>&1; then
  have_fprintd_unit=1
  if ! systemctl stop fprintd.service; then
    printf '%s\n' "warning: systemctl stop fprintd.service failed; continuing removal" >&2
  fi
fi

rm -f "${DROPIN}"
if [[ -d "${DROPIN_DIR}" ]]; then
  rmdir --ignore-fail-on-non-empty "${DROPIN_DIR}"
fi
rm -rf /opt/fingerpp

if command -v systemctl >/dev/null 2>&1; then
  systemctl daemon-reload
  if [[ "${have_fprintd_unit}" -eq 1 ]]; then
    systemctl restart fprintd.service
  fi
fi

printf '%s\n' "Removed /opt/fingerpp and ${DROPIN}, then restarted fprintd if the unit exists."
