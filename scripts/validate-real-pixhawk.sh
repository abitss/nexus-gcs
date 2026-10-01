#!/usr/bin/env bash
set -euo pipefail

test_file="${1:-custom/test/NexusRealPixhawkBenchTest.cc}"
doc="${2:-docs/REAL_PIXHAWK_QUALIFICATION.md}"
workflow="${3:-.github/workflows/nexus-real-pixhawk-bench.yml}"

required_test=(
  "NEXUS_REAL_PIXHAWK_PORT"
  "NEXUS_REAL_PIXHAWK_BAUD"
  "NEXUS_PROPS_REMOVED"
  "NEXUS_KILL_SWITCH_CONFIRMED"
  "MAVLINK_MSG_ID_HEARTBEAT"
  "parametersReady"
  "heartbeatRateHz"
  "VehicleGPSFactGroup"
  "BatteryFactGroup"
  "flightModes"
  "homePosition"
  "verifyUpload"
  "PROPS_OFF_ARM"
  "PROPS_OFF_DISARM"
)

required_workflow=(
  "workflow_dispatch"
  "self-hosted"
  "nexus-pixhawk-bench"
  "props_removed"
  "kill_switch_confirmed"
  "NEXUS_REAL_PIXHAWK_PORT"
  "NEXUS_PIXHAWK_BENCH_EVIDENCE"
)

for token in "${required_test[@]}"; do
  grep -Fq "$token" "$test_file" || { echo "Missing real Pixhawk test invariant: $token" >&2; exit 1; }
done

for token in "${required_workflow[@]}"; do
  grep -Fq "$token" "$workflow" || { echo "Missing real Pixhawk workflow invariant: $token" >&2; exit 1; }
done

grep -Fq 'props physically removed' "$doc"

# Bench qualification must never contain flight/movement commands.
if grep -Eq 'guidedModeTakeoff|guidedModeRTL|guidedModeLand|startMission\(|guidedModeGotoLocation|sendGripperAction|actuatorTest|motorTest' "$test_file"; then
  echo "Real Pixhawk bench qualification contains a forbidden flight/movement command." >&2
  exit 1
fi

# ARM must be paired with DISARM in the same test.
grep -Fq 'setArmedShowError(true)' "$test_file"
grep -Fq 'setArmedShowError(false)' "$test_file"

echo "NEXUS real Pixhawk bench qualification invariants: PASS"
