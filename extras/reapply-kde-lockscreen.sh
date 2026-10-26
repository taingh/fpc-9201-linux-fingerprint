#!/usr/bin/env bash
# Apply or revert the KDE lock screen fingerprint workaround.
#
# Idempotent: if "fpArmTimer" is already present, do nothing.
# If LockScreen.qml or the anchor line is missing, skip (exit 0).
# Messages go to stderr. dnf's libdnf5 actions plugin parses stdout.
#
# FINGERPP_LOCKSCREEN_QML overrides the QML path and skips the root check.
# That is for tests; install.sh and uninstall.sh unset it.
set -euo pipefail

readonly DEFAULT_QML=/usr/share/plasma/shells/org.kde.plasma.desktop/contents/lockscreen/LockScreen.qml
readonly MARKER='fpArmTimer'
# Exact line from Plasma 6.6 LockScreen.qml (4-space indent).
readonly ANCHOR='    property bool suspendToDiskSupported: false'

verbose=0
restore=0

usage() {
  printf '%s\n' "usage: $0 [--verbose] [--restore]" >&2
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --verbose) verbose=1 ;;
    --restore) restore=1 ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      usage
      exit 1
      ;;
  esac
  shift
done

qml="${FINGERPP_LOCKSCREEN_QML:-${DEFAULT_QML}}"
tmp=""

cleanup_tmp() {
  if [[ -n "${tmp}" ]]; then
    rm -f -- "${tmp}"
  fi
}
trap cleanup_tmp EXIT

say() {
  printf '%s\n' "$*" >&2
}

say_verbose() {
  if [[ "${verbose}" -eq 1 ]]; then
    say "$@"
  fi
}

require_privilege() {
  if [[ "${EUID}" -eq 0 ]]; then
    return 0
  fi
  if [[ -n "${FINGERPP_LOCKSCREEN_QML:-}" ]]; then
    return 0
  fi
  say "This script needs root. Run: sudo $0"
  exit 1
}

restore_selinux() {
  local path="$1"
  if command -v restorecon >/dev/null 2>&1; then
    restorecon -F -- "${path}" || true
  fi
}

# Follow a symlink so the .bak sits next to the file plasma-desktop owns.
resolve_qml() {
  if [[ ! -L "${qml}" ]]; then
    return 0
  fi
  local resolved
  resolved="$(readlink -f -- "${qml}" || true)"
  if [[ -z "${resolved}" ]]; then
    say "Could not resolve KDE lock screen symlink ${qml}; skipping fingerprint patch."
    exit 0
  fi
  qml="${resolved}"
}

insert_patch() {
  cat <<'EOF'
    // fingerprint patch: start authentication as soon as the lock is effective,
    // and re-arm the noninteractive (fingerprint) authenticator when it ends.
    property bool locked: false
    onLockedChanged: if (locked) fpArmTimer.restart();
    Timer {
        id: fpArmTimer
        interval: 1500
        onTriggered: authenticator.startAuthenticating();
    }
    Connections {
        target: authenticator
        function onFailed(kind) {
            if (kind != 0 && root.locked) fpArmTimer.restart();
        }
    }
EOF
}

apply_patch() {
  resolve_qml

  if [[ ! -e "${qml}" ]]; then
    say_verbose "KDE lock screen not found (${qml}); skipping fingerprint patch."
    exit 0
  fi
  if [[ ! -f "${qml}" ]]; then
    say "KDE lock screen path is not a regular file (${qml}); skipping fingerprint patch."
    exit 0
  fi

  if grep -F -q -e "${MARKER}" -- "${qml}"; then
    say_verbose "KDE lock screen fingerprint patch already present; skipping."
    exit 0
  fi

  # Upstream plasma-desktop!3891 declares this property itself. Inserting it
  # again is a QML error and would break the lock screen.
  if grep -E -q '^[[:space:]]*property[[:space:]]+bool[[:space:]]+locked[[:space:]]*:' -- "${qml}"; then
    say_verbose "LockScreen.qml already declares \"property bool locked\"; skipping fingerprint patch."
    exit 0
  fi

  if ! grep -F -x -q -e "${ANCHOR}" -- "${qml}"; then
    say "KDE lock screen has no anchor line; skipping fingerprint patch."
    say "  file: ${qml}"
    say "  looked for: ${ANCHOR}"
    exit 0
  fi

  tmp="$(mktemp)"
  local inserted=0
  while IFS= read -r line || [[ -n "${line}" ]]; do
    printf '%s\n' "${line}"
    if [[ "${inserted}" -eq 0 && "${line}" == "${ANCHOR}" ]]; then
      insert_patch
      inserted=1
    fi
  done < "${qml}" > "${tmp}"

  if [[ "${inserted}" -ne 1 ]]; then
    say "Failed to insert the fingerprint patch into ${qml}."
    exit 1
  fi

  # Replace .bak with the file we are about to patch, so uninstall restores
  # this Plasma version rather than an older backup.
  cp -a -- "${qml}" "${qml}.bak"
  if ! cat "${tmp}" > "${qml}"; then
    cp -a -- "${qml}.bak" "${qml}"
    say "Failed to write ${qml}; restored the backup."
    exit 1
  fi
  restore_selinux "${qml}"
  say "Applied KDE lock screen fingerprint patch to ${qml} (backup: ${qml}.bak)."
}

restore_backup() {
  resolve_qml
  local bak="${qml}.bak"

  if [[ -f "${bak}" ]]; then
    mv -f -- "${bak}" "${qml}"
    if [[ -f "${qml}" ]]; then
      restore_selinux "${qml}"
    fi
    say "Restored ${qml} from ${bak}."
    exit 0
  fi

  if [[ -f "${qml}" ]] && grep -F -q -e "${MARKER}" -- "${qml}"; then
    say "Fingerprint patch is present in ${qml}, but ${bak} is missing; left the file unchanged."
    exit 0
  fi

  say "No KDE lock screen backup to restore (${bak})."
  exit 0
}

require_privilege
if [[ "${restore}" -eq 1 ]]; then
  restore_backup
else
  apply_patch
fi
