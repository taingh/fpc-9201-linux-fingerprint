#!/usr/bin/env bash
# Install the tested fingerpp binary and point fprintd at it.
# Idempotent: a second run overwrites the same files and restarts fprintd.
# Requires root (sudo) and the distro fprintd package.
set -euo pipefail

readonly DEST_DIR=/opt/fingerpp/bin
readonly DEST="${DEST_DIR}/fingerpp"
readonly DROPIN_DIR=/etc/systemd/system/fprintd.service.d
readonly DROPIN="${DROPIN_DIR}/override.conf"
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

systemctl daemon-reload
systemctl restart fprintd.service

user_name="${SUDO_USER:-}"
if [[ -z "${user_name}" || "${user_name}" == "root" ]]; then
  printf '%s\n' \
    "Installed, but SUDO_USER is not a normal user, so the device was not polled." \
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
