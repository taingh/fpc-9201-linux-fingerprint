#!/usr/bin/env bash
# Fixture checks for extras/reapply-kde-lockscreen.sh.
# Uses FINGERPP_LOCKSCREEN_QML so it does not need root or a real Plasma install.
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
reapply="${script_dir}/reapply-kde-lockscreen.sh"
fixtures="${script_dir}/testdata"
work="$(mktemp -d)"
trap 'rm -rf "${work}"' EXIT

assert_eq() {
  if [[ "$1" != "$2" ]]; then
    printf 'FAIL %s\n got [%s]\n expected [%s]\n' "$3" "$1" "$2" >&2
    exit 1
  fi
}

# Print the line immediately after the first exact match.
next_line() {
  local file="$1"
  local exact="$2"
  awk -v anchor="${exact}" '$0 == anchor { getline; print; exit }' "${file}"
}

runp() {
  local qml_path="$1"
  shift
  set +e
  FINGERPP_LOCKSCREEN_QML="${qml_path}" bash "${reapply}" "$@" >"${work}/out" 2>"${work}/err"
  rc=$?
  set -e
}

patch_comment='    // fingerprint patch: start authentication as soon as the lock is effective,'
view_visible='    property bool viewVisible: false'
id_root='    id: root'

# Missing file stays quiet unless --verbose, and writes nothing to stdout.
runp "${work}/missing.qml"
assert_eq "${rc}" 0 "missing rc"
assert_eq "$(cat "${work}/err")" "" "missing quiet stderr"
assert_eq "$(cat "${work}/out")" "" "missing stdout"
runp "${work}/missing.qml" --verbose
grep -q 'not found' "${work}/err"

# Plasma 6.7.5: no suspendToRam/Disk. Insert after viewVisible, not after id: root.
cp "${fixtures}/lockscreen-6.7.5.qml" "${work}/lock-6.7.5.qml"
runp "${work}/lock-6.7.5.qml"
assert_eq "${rc}" 0 "6.7.5 rc"
assert_eq "$(cat "${work}/out")" "" "6.7.5 stdout empty"
grep -q 'Applied KDE lock screen' "${work}/err"
if grep -q 'suspendToDiskSupported' "${work}/lock-6.7.5.qml"; then
  printf '%s\n' 'FAIL 6.7.5 fixture unexpectedly contains suspendToDiskSupported' >&2
  exit 1
fi
assert_eq "$(next_line "${work}/lock-6.7.5.qml" "${view_visible}")" "${patch_comment}" "6.7.5 inserts after viewVisible"
assert_eq "$(next_line "${work}/lock-6.7.5.qml" "${id_root}")" "    property bool debug: false" "6.7.5 leaves id: root alone"
assert_eq "$(grep -c 'id: fpArmTimer' "${work}/lock-6.7.5.qml")" 1 "6.7.5 one timer"

# Plasma 6.6 has both anchors. viewVisible wins over suspendToDisk and id: root.
cp "${fixtures}/lockscreen-6.6.qml" "${work}/lock-6.6.qml"
runp "${work}/lock-6.6.qml"
assert_eq "${rc}" 0 "6.6 rc"
assert_eq "$(next_line "${work}/lock-6.6.qml" "${view_visible}")" "${patch_comment}" "6.6 inserts after viewVisible"
assert_eq "$(next_line "${work}/lock-6.6.qml" "${id_root}")" "    property bool debug: false" "6.6 leaves id: root alone"
grep -q '    property bool suspendToDiskSupported: false' "${work}/lock-6.6.qml"
assert_eq "$(grep -c 'id: fpArmTimer' "${work}/lock-6.6.qml")" 1 "6.6 one timer"
# The old suspend anchor is still in the file, after the patch, and is not the insertion point.
awk '
  /property bool viewVisible: false/ { seen_view = 1 }
  seen_view && /fingerprint patch/ { seen_patch = 1 }
  seen_patch && /suspendToDiskSupported/ { seen_suspend = 1 }
  END { exit !(seen_view && seen_patch && seen_suspend) }
' "${work}/lock-6.6.qml"

# Second run is a no-op and does not refresh the backup of an already patched file.
sum1="$(sha256sum "${work}/lock-6.7.5.qml" "${work}/lock-6.7.5.qml.bak")"
runp "${work}/lock-6.7.5.qml" --verbose
grep -q 'already present' "${work}/err"
sum2="$(sha256sum "${work}/lock-6.7.5.qml" "${work}/lock-6.7.5.qml.bak")"
assert_eq "${sum1}" "${sum2}" "idempotent hashes"
runp "${work}/lock-6.7.5.qml"
assert_eq "$(cat "${work}/err")" "" "already-present quiet"

# Package upgrade replaces a patched 6.6 file with unpatched 6.7.5. The old
# .bak is from 6.6; re-apply must replace it with the new unpatched file and
# insert after viewVisible.
cp "${fixtures}/lockscreen-6.6.qml" "${work}/upgraded.qml"
runp "${work}/upgraded.qml"
cp "${fixtures}/lockscreen-6.7.5.qml" "${work}/upgraded.qml"
cp "${fixtures}/lockscreen-6.7.5.qml" "${work}/upgraded.expected-bak"
runp "${work}/upgraded.qml"
cmp -s "${work}/upgraded.qml.bak" "${work}/upgraded.expected-bak"
assert_eq "$(next_line "${work}/upgraded.qml" "${view_visible}")" "${patch_comment}" "upgrade inserts after viewVisible"
if grep -q 'suspendToDiskSupported' "${work}/upgraded.qml"; then
  printf '%s\n' 'FAIL upgraded file still has the 6.6 suspend anchor' >&2
  exit 1
fi

# Last resort: no viewVisible, so insert after id: root.
cp "${fixtures}/lockscreen-id-root-only.qml" "${work}/id-only.qml"
runp "${work}/id-only.qml"
assert_eq "${rc}" 0 "id-root rc"
assert_eq "$(next_line "${work}/id-only.qml" "${id_root}")" "${patch_comment}" "id: root fallback"
assert_eq "$(grep -c 'id: fpArmTimer' "${work}/id-only.qml")" 1 "id-root one timer"

# Neither anchor: skip, leave the file alone, name both lines.
cp "${fixtures}/lockscreen-no-anchor.qml" "${work}/no-anchor.qml"
cp "${work}/no-anchor.qml" "${work}/no-anchor.orig"
runp "${work}/no-anchor.qml"
assert_eq "${rc}" 0 "no-anchor rc"
grep -q 'no anchor line' "${work}/err"
grep -F -q 'property bool viewVisible: false' "${work}/err"
grep -F -q 'id: root' "${work}/err"
cmp -s "${work}/no-anchor.qml" "${work}/no-anchor.orig"
[[ ! -e "${work}/no-anchor.qml.bak" ]]

# Upstream property bool locked is left untouched.
cat > "${work}/upstream.qml" <<'EOF'
Item {
    id: root
    property bool viewVisible: false
    property bool locked: false
    onLockedChanged: if (locked) authenticator.startAuthenticating()
}
EOF
cp "${work}/upstream.qml" "${work}/upstream.orig"
runp "${work}/upstream.qml"
assert_eq "${rc}" 0 "upstream rc"
assert_eq "$(cat "${work}/err")" "" "upstream quiet"
cmp -s "${work}/upstream.qml" "${work}/upstream.orig"
[[ ! -e "${work}/upstream.qml.bak" ]]
runp "${work}/upstream.qml" --verbose
grep -q 'already declares' "${work}/err"
cmp -s "${work}/upstream.qml" "${work}/upstream.orig"

# Symlink: backup sits next to the target.
cp "${fixtures}/lockscreen-6.7.5.qml" "${work}/real.qml"
ln -s real.qml "${work}/link.qml"
runp "${work}/link.qml"
assert_eq "${rc}" 0 "symlink rc"
[[ -f "${work}/real.qml.bak" ]]
[[ ! -e "${work}/link.qml.bak" ]]
grep -q 'id: fpArmTimer' "${work}/real.qml"

runp "${work}/real.qml" --restore
assert_eq "${rc}" 0 "restore rc"
grep -q 'Restored' "${work}/err"
[[ ! -e "${work}/real.qml.bak" ]]
if grep -q 'fpArmTimer' "${work}/real.qml"; then
  printf '%s\n' 'FAIL restore left the marker' >&2
  exit 1
fi

printf '%s\n' 'ALL_TESTS_OK'
