#!/usr/bin/env bash
# Install the tested fingerpp binary and point fprintd at it.
# Also installs the suspend hook and, when KDE is present, the lock screen patch.
# Idempotent: a second run overwrites the same files and restarts fprintd.
# Requires root (sudo or pkexec) and the distro fprintd package.
set -euo pipefail

readonly DEST_DIR=/opt/fingerpp/bin
readonly DEST="${DEST_DIR}/fingerpp"
readonly DROPIN_DIR=/etc/systemd/system/fprintd.service.d
readonly DROPIN="${DROPIN_DIR}/override.conf"
readonly SLEEP_UNIT=/etc/systemd/system/fingerpp-sleep.service
readonly REAPPLY_DEST=/usr/local/sbin/fingerpp-reapply-kde-lockscreen
readonly DNF_ACTIONS_DIR=/etc/dnf/libdnf5-plugins/actions.d
readonly DNF_ACTIONS="${DNF_ACTIONS_DIR}/fingerpp-plasma-desktop.actions"
readonly APT_HOOK=/etc/apt/apt.conf.d/99fingerpp-lockscreen
readonly EXPECTED_SHA256=4e9a3b8d71bb818ceb5b4fa6b16cbfa5b1619afd1eba548cda723396c94e5f8c
readonly WAIT_SECS=45

if [[ "${EUID}" -ne 0 ]]; then
  printf '%s\n' "This installer needs root. Run: sudo $0" >&2
  exit 1
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
bin_src="${script_dir}/bin/fingerpp3.static"

fprintd_install_hint() {
  if command -v apt-get >/dev/null 2>&1; then
    printf '%s\n' "Debian/Ubuntu (apt): sudo apt-get install fprintd libpam-fprintd" >&2
  elif command -v dnf >/dev/null 2>&1; then
    printf '%s\n' "Fedora (dnf): sudo dnf install fprintd fprintd-pam" >&2
  elif command -v pacman >/dev/null 2>&1; then
    printf '%s\n' "Arch (pacman): sudo pacman -S fprintd" >&2
  else
    printf '%s\n' "Install the distro fprintd package (apt: fprintd, dnf: fprintd, pacman: fprintd)." >&2
  fi
}

if ! command -v fprintd-list >/dev/null 2>&1; then
  printf '%s\n' "fprintd is not installed (fprintd-list was not found)." >&2
  fprintd_install_hint
  exit 1
fi

if ! command -v systemctl >/dev/null 2>&1; then
  printf '%s\n' "systemctl was not found. This installer needs systemd." >&2
  exit 1
fi

if ! systemctl cat fprintd.service >/dev/null 2>&1; then
  printf '%s\n' "systemd unit fprintd.service was not found." >&2
  fprintd_install_hint
  exit 1
fi

if [[ ! -f "${bin_src}" ]]; then
  printf '%s\n' "Missing binary: ${bin_src}" >&2
  exit 1
fi

actual_sha="$(sha256sum "${bin_src}" | awk '{print $1}')"
if [[ "${actual_sha}" != "${EXPECTED_SHA256}" ]]; then
  printf '%s\n' \
    "Checksum mismatch for ${bin_src}" \
    "  expected ${EXPECTED_SHA256}" \
    "  actual   ${actual_sha}" >&2
  exit 1
fi

if command -v ldd >/dev/null 2>&1; then
  missing="$(ldd "${bin_src}" 2>&1 | awk '/not found/ { print }' || true)"
  if [[ -n "${missing}" ]]; then
    printf '%s\n' "fingerpp is missing shared libraries:" "${missing}" >&2
    printf '%s\n' \
      "Ubuntu/Debian: libssl3 libusb-1.0-0 libdbus-1-3" \
      "Fedora: openssl-libs libusb1 dbus-libs" \
      "Arch: openssl libusb dbus" >&2
    exit 1
  fi
fi

install -d -m 0755 "${DEST_DIR}" "${DROPIN_DIR}"

# Stop before replacing the executable so a running fingerpp cannot block the copy.
systemctl stop fprintd.service
install -m 0755 "${bin_src}" "${DEST}"

tmp_override="$(mktemp)"
cat > "${tmp_override}" <<'EOF'
[Service]
ExecStart=
ExecStart=/opt/fingerpp/bin/fingerpp --bus=system --filter-before-ssim=true --min-score=0.3
EOF
install -m 0644 "${tmp_override}" "${DROPIN}"
rm -f "${tmp_override}"

# Do not "enable --now": ExecStart stops fprintd, and this unit is only
# supposed to run when sleep.target is queued.
install -m 0644 "${script_dir}/extras/fingerpp-sleep.service" "${SLEEP_UNIT}"

systemctl daemon-reload
systemctl enable fingerpp-sleep.service
systemctl restart fprintd.service

install_lockscreen_helpers() {
  install -d -m 0755 /usr/local/sbin
  install -m 0755 "${script_dir}/extras/reapply-kde-lockscreen.sh" "${REAPPLY_DEST}"

  if command -v dnf >/dev/null 2>&1 || command -v dnf5 >/dev/null 2>&1 || [[ -d /etc/dnf ]]; then
    install -d -m 0755 "${DNF_ACTIONS_DIR}"
    install -m 0644 "${script_dir}/extras/fingerpp-plasma-desktop.actions" "${DNF_ACTIONS}"
    if ! libdnf5_actions_plugin_installed; then
      printf '%s\n' \
        "Wrote ${DNF_ACTIONS}, but libdnf5-plugin-actions does not appear to be installed." \
        "Install it so a plasma-desktop update re-applies the lock screen patch:" \
        "  sudo dnf install libdnf5-plugin-actions" >&2
    fi
  fi

  if command -v apt-get >/dev/null 2>&1 || [[ -d /etc/apt/apt.conf.d ]]; then
    install -d -m 0755 /etc/apt/apt.conf.d
    install -m 0644 "${script_dir}/extras/99fingerpp-lockscreen" "${APT_HOOK}"
  fi

  # Unset the test override so a leftover env var cannot redirect the patch.
  env -u FINGERPP_LOCKSCREEN_QML "${REAPPLY_DEST}" --verbose
}

libdnf5_actions_plugin_installed() {
  local candidate
  for candidate in \
    /usr/lib64/libdnf5/plugins/actions.so \
    /usr/lib/libdnf5/plugins/actions.so \
    /usr/lib/x86_64-linux-gnu/libdnf5/plugins/actions.so
  do
    if [[ -e "${candidate}" ]]; then
      return 0
    fi
  done
  if command -v rpm >/dev/null 2>&1 && rpm -q libdnf5-plugin-actions >/dev/null 2>&1; then
    return 0
  fi
  return 1
}

# sudo sets SUDO_USER. pkexec leaves it unset and sets PKEXEC_UID instead.
# The sleep hook and the lock screen patch do not need this user; only the
# fprintd-list poll does.
resolve_invoking_user() {
  local name="${SUDO_USER:-}"
  if [[ -n "${name}" && "${name}" != "root" ]]; then
    printf '%s\n' "${name}"
    return 0
  fi
  if [[ -n "${PKEXEC_UID:-}" ]]; then
    name="$(id -nu -- "${PKEXEC_UID}" 2>/dev/null || true)"
    if [[ -n "${name}" && "${name}" != "root" ]]; then
      printf '%s\n' "${name}"
      return 0
    fi
  fi
  return 1
}

install_lockscreen_helpers

if ! user_name="$(resolve_invoking_user)"; then
  printf '%s\n' \
    "Installed fingerpp, the sleep hook, and the lock screen helper, but no normal user was found via SUDO_USER or PKEXEC_UID, so the device was not polled." \
    "Check the device with: fprintd-list <your-user>" \
    "Then enroll: fprintd-enroll -f right-index-finger" >&2
  exit 1
fi

printf '%s\n' "Waiting for fprintd to expose a device for ${user_name}..."
deadline=$((SECONDS + WAIT_SECS))
last_out=""
ready=0
while (( SECONDS < deadline )); do
  set +e
  if command -v timeout >/dev/null 2>&1; then
    last_out="$(timeout 5 fprintd-list "${user_name}" 2>&1)"
  else
    last_out="$(fprintd-list "${user_name}" 2>&1)"
  fi
  set -e
  if grep -F -q '/net/reactivated/Fprint/Device/' <<<"${last_out}"; then
    ready=1
    break
  fi
  sleep 1
done

if [[ "${ready}" -ne 1 ]]; then
  printf '%s\n' \
    "fprintd did not report a device within ${WAIT_SECS}s. Last fprintd-list output:" \
    "${last_out}" >&2
  printf '%s\n' \
    "If you see \"GetDefaultDevice doesn't exist\", wait a few seconds and retry: fprintd-list ${user_name}" >&2
  exit 1
fi

printf '%s\n' \
  "Fingerprint device is available for ${user_name}:" \
  "${last_out}" \
  "" \
  "Enroll a finger as ${user_name}. There is no progress bar; keep lifting and pressing until enroll-completed:" \
  "  fprintd-enroll -f right-index-finger"
