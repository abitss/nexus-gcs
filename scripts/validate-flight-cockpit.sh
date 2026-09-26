#!/usr/bin/env bash
set -euo pipefail

file="${1:-custom/src/FlyViewCustomLayer.qml}"

test -f "$file"

required=(
  "QGroundControl.multiVehicleManager.activeVehicle"
  "globals.guidedControllerFlyView"
  "vehicleLinkManager.communicationLost"
  "healthAndArmingCheckReport"
  "QGroundControl.videoManager"
  "missionController.progressPct"
  "guidedController.confirmAction"
  "onMavCommandResult"
)

for token in "${required[@]}"; do
  if ! grep -Fq "$token" "$file"; then
    echo "Missing Flight cockpit invariant: $token" >&2
    exit 1
  fi
done

if grep -Eiq 'mock telemetry|fake telemetry|demo telemetry' "$file"; then
  echo "Production Flight cockpit must not contain mock/fake/demo telemetry." >&2
  exit 1
fi

if grep -Fq 'sendMavCommand(' "$file"; then
  echo "Flight cockpit must route standard actions through QGC guided actions, not direct sendMavCommand calls." >&2
  exit 1
fi

echo "NEXUS Flight cockpit architecture invariants: PASS"
