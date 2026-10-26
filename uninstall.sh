#!/usr/bin/env bash
# Remove the fingerpp fprintd drop-in, sleep hook, KDE patch, and /opt/fingerpp.
# Idempotent: a second run just restarts fprintd again. Requires root (sudo).
set -euo pipefail

readonly DROPIN_DIR=/etc/systemd/system/fprintd.service.d
readonly DROPIN="${DROPIN_DIR}/override.conf"
readonly SLEEP_UNIT=/etc/systemd/system/fingerpp-sleep.service
readonly SLEEP_WANTS=/etc/systemd/system/sleep.target.wants/fingerpp-sleep.service
readonly REAPPLY_DEST=/usr/local/sbin/fingerpp-reapply-kde-lockscreen
readonly DNF_ACTIONS=/etc/dnf/libdnf5-plugins/actions.d/fingerpp-plasma-desktop.actions
readonly APT_HOOK=/etc/apt/apt.conf.d/99fingerpp-lockscreen

if [[ "${EUID}" -ne 0 ]]; then
  printf '%s\n' "This uninstaller needs root. Run: sudo $0" >&2
  exit 1
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
reapply="${script_dir}/extras/reapply-kde-lockscreen.sh"
if [[ -f "${reapply}" ]]; then
  if ! env -u FINGERPP_LOCKSCREEN_QML bash "${reapply}" --restore; then
    printf '%s\n' "warning: failed to restore the KDE lock screen; continuing removal" >&2
  fi
else
  printf '%s\n' "warning: ${reapply} is missing; lock screen backup was not restored" >&2
fi

rm -f -- "${REAPPLY_DEST}" "${DNF_ACTIONS}" "${APT_HOOK}"

if command -v systemctl >/dev/null 2>&1; then
  if [[ -f "${SLEEP_UNIT}" ]] || systemctl cat fingerpp-sleep.service >/dev/null 2>&1; then
    if ! systemctl disable --now fingerpp-sleep.service; then
      printf '%s\n' "warning: systemctl disable fingerpp-sleep.service failed; continuing removal" >&2
    fi
  fi
fi
rm -f -- "${SLEEP_UNIT}" "${SLEEP_WANTS}"

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

printf '%s\n' \
  "Removed /opt/fingerpp, ${DROPIN}, ${SLEEP_UNIT}, and the KDE lock screen hooks." \
  "Restored LockScreen.qml when ${reapply} found a .bak." \
  "Restarted fprintd if the unit exists."
