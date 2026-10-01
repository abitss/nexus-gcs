#!/usr/bin/env bash
set -euo pipefail

test_file="${1:-custom/test/NexusPX4QualificationTest.cc}"
workflow="${2:-.github/workflows/nexus-px4-qualification.yml}"

required_test=(
  "PREFLIGHT"
  "ARM"
  "TAKEOFF"
  "MISSION_UPLOAD_READBACK"
  "WAYPOINT_MISSION"
  "HOLD"
  "CONTINUE"
  "RTL"
  "LAND"
  "DISCONNECT"
  "RECONNECT"
  "FAILURE_CORRUPT_MISSION"
  "FAILURE_INTERRUPTED_MISSION"
  "FAILURE_RECOVERY"
  "MISSION_CURRENT"
  "startMission"
  "pauseVehicle"
  "guidedModeRTL"
  "guidedModeLand"
  "verifyUpload"
  "interruptedMission"
)

required_workflow=(
  "QGC_BASELINE_SHA"
  "PX4_V117_SHA"
  "PX4_DEV_IMAGE"
  "NexusPX4QualificationTest"
  "NEXUS_PX4_QUALIFICATION_EVIDENCE"
  "px4-qualification-sitl.log"
  "nexus-px4-qualification.json"
)

for token in "${required_test[@]}"; do
  grep -Fq "$token" "$test_file" || { echo "Missing PX4 qualification invariant: $token" >&2; exit 1; }
done

for token in "${required_workflow[@]}"; do
  grep -Fq "$token" "$workflow" || { echo "Missing PX4 workflow invariant: $token" >&2; exit 1; }
done

# Qualification must verify returned vehicle state, not merely issue calls.
grep -Fq 'waitForCondition' "$test_file"
grep -Fq 'vehicle->armed()' "$test_file"
grep -Fq 'vehicle->flying()' "$test_file"
grep -Fq 'vehicle->flightMode() == vehicle->missionFlightMode()' "$test_file"
grep -Fq 'vehicle->flightMode() == vehicle->pauseFlightMode()' "$test_file"
grep -Fq 'vehicle->flightMode() == vehicle->rtlFlightMode()' "$test_file"
grep -Fq 'vehicle->flightMode() == vehicle->landFlightMode()' "$test_file"
grep -Fq '!vehicle->armed()' "$test_file"

# Never let a PX4 qualification gate use floating upstream refs.
if grep -Eq 'ref:[[:space:]]*(main|master|latest)$' "$workflow"; then
  echo "PX4 qualification must use pinned source revisions." >&2
  exit 1
fi

echo "NEXUS PX4 qualification architecture invariants: PASS"
