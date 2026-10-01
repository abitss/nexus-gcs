#!/usr/bin/env bash
set -euo pipefail

APK="${1:?APK path required}"
PACKAGE="${2:?package id required}"
EVIDENCE="${3:?evidence directory required}"
ACTIVITY="org.mavlink.qgroundcontrol.QGCActivity"
COMPONENT="${PACKAGE}/${ACTIVITY}"

mkdir -p "${EVIDENCE}"

adb wait-for-device
adb install -r -g "${APK}" | tee "${EVIDENCE}/install.txt"

# Force a deterministic landscape tablet viewport.
adb shell settings put system accelerometer_rotation 0 || true
adb shell settings put system user_rotation 1 || true
adb shell wm size 1280x800
adb shell wm density 240
adb shell wm size | tee "${EVIDENCE}/wm-size.txt"
adb shell wm density | tee "${EVIDENCE}/wm-density.txt"

validate_png_landscape() {
  local image_path="$1"
  python3 - "$image_path" <<'PY'
import struct, sys
path = sys.argv[1]
with open(path, "rb") as f:
    header = f.read(24)
if len(header) < 24 or header[:8] != b"\x89PNG\r\n\x1a\n":
    raise SystemExit(f"not a valid PNG: {path}")
width, height = struct.unpack(">II", header[16:24])
print(f"{path}: {width}x{height}")
if width <= height:
    raise SystemExit(f"expected landscape screenshot, got {width}x{height}")
if width < 1000 or height < 600:
    raise SystemExit(f"viewport below Task 0.3 landscape baseline: {width}x{height}")
PY
}

launch_and_prove_stable() {
  local label="$1"

  adb logcat -c
  adb shell am force-stop "${PACKAGE}" || true
  adb shell am start -W -n "${COMPONENT}" | tee "${EVIDENCE}/${label}-launch.txt"

  # A launch that returns successfully but dies immediately is still a failure.
  sleep 20
  local pid
  pid="$(adb shell pidof "${PACKAGE}" | tr -d '\r' || true)"
  if [[ -z "${pid}" ]]; then
    adb logcat -d > "${EVIDENCE}/${label}-logcat.txt" || true
    echo "NEXUS process is not alive after ${label} launch." >&2
    exit 1
  fi
  printf '%s\n' "${pid}" > "${EVIDENCE}/${label}-pid.txt"

  adb exec-out screencap -p > "${EVIDENCE}/${label}.png"
  validate_png_landscape "${EVIDENCE}/${label}.png"

  adb shell dumpsys activity top > "${EVIDENCE}/${label}-activity.txt" || true
  adb shell dumpsys window > "${EVIDENCE}/${label}-window.txt" || true
  adb shell uiautomator dump /sdcard/nexus-ui.xml >/dev/null 2>&1 || true
  adb pull /sdcard/nexus-ui.xml "${EVIDENCE}/${label}-ui.xml" >/dev/null 2>&1 || true
  adb logcat -d > "${EVIDENCE}/${label}-logcat.txt" || true

  # Process liveness is authoritative. Also fail on an explicit Java fatal crash
  # attributed to our package during the launch window.
  if grep -A8 -E "FATAL EXCEPTION|Fatal signal" "${EVIDENCE}/${label}-logcat.txt" | grep -Fq "${PACKAGE}"; then
    echo "Fatal runtime crash detected for ${PACKAGE} during ${label} launch." >&2
    exit 1
  fi
}

launch_and_prove_stable "online"

# Prove the application can relaunch with external connectivity disabled.
adb shell svc wifi disable || true
adb shell svc data disable || true
adb shell settings put global airplane_mode_on 1
adb shell am broadcast -a android.intent.action.AIRPLANE_MODE --ez state true >/dev/null || true
sleep 2

airplane="$(adb shell settings get global airplane_mode_on | tr -d '\r')"
printf 'airplane_mode=%s\n' "${airplane}" | tee "${EVIDENCE}/offline-state.txt"
if [[ "${airplane}" != "1" ]]; then
  echo "Failed to place emulator into offline/airplane mode." >&2
  exit 1
fi

launch_and_prove_stable "offline"

echo "NEXUS Android install/launch/offline/landscape smoke: PASS"
