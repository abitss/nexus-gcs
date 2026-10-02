#!/usr/bin/env bash
set -euo pipefail

model="${1:-custom/src/NexusFieldQualificationModel.cc}"
panel="${2:-custom/src/NexusFieldQualificationPanel.qml}"
hil="${3:-custom/test/NexusPX4HILQualificationTest.cc}"
doc="${4:-docs/HIL_FIELD_QUALIFICATION.md}"

for token in heartbeatRateHz mavlinkLossPercent gpsFix satellites hdop vdop missionIndex maxDeviceTempC minRamAvailableMb maxEventLoopLagMs recoveryEvents; do
  grep -Fq "$token" "$model" || { echo "Missing field evidence metric: $token" >&2; exit 1; }
done

for token in "START HIL" "START FIELD" "PASS TELEMETRY" "PASS GPS" "PASS MISSION" "PASS LINK" "PASS FAILSAFE" "PASS STABILITY" "PASS PERFORMANCE" "PASS THERMAL" "EXPORT"; do
  grep -Fq "$token" "$panel" || { echo "Missing qualification UI invariant: $token" >&2; exit 1; }
done

for token in NEXUS_HIL_PIXHAWK_PORT NEXUS_PROPS_REMOVED NEXUS_HIL_SIMULATOR_CONFIRMED MAV_MODE_FLAG_HIL_ENABLED MISSION_EXECUTION HOLD CONTINUE RTL LAND; do
  grep -Fq "$token" "$hil" || { echo "Missing HIL invariant: $token" >&2; exit 1; }
done

grep -Fq "do not jam RF" "$doc"
grep -Fq "NEXUS project acceptance thresholds" "$doc"

# Field recorder is observational and must never issue flight commands or manipulate RF/network state.
if grep -Eiq 'guidedMode|setArmed|startMission\(|sendMavCommand|sendCommand|tc qdisc|iptables|rfkill|iwconfig|nmcli' "$model" "$panel"; then
  echo "Field qualification recorder must remain observational." >&2
  exit 1
fi

echo "NEXUS HIL + controlled-field architecture invariants: PASS"
