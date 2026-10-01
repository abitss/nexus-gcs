#!/usr/bin/env bash
set -euo pipefail

panel="${1:-custom/src/NexusEngineerPanel.qml}"
model="${2:-custom/src/NexusEngineerModel.cc}"
fly="${3:-custom/src/FlyViewCustomLayer.qml}"

required_panel=(
  "ENGINEER MODE"
  "PROTECTED ENTRY"
  "Type ENGINEER"
  "PARAMETER BROWSER"
  "BACKUP / RESTORE"
  "MAVLINK INSPECTOR / STREAM RATES"
  "RAW SENSOR DIAGNOSTICS"
  "EKF / GPS / IMU"
  "SYSTEM MESSAGES"
  "DEVELOPER LOGS"
  "PARAMETER SNAPSHOTS"
  "showVehicleConfigParametersPage"
  "MAVLinkInspectorPage.qml"
  "VibrationPage.qml"
  "App Logging"
  "formattedMessages"
  "showAdvancedUI"
  "safeToWrite"
)

required_model=(
  "confirmation.trimmed()"
  "ENGINEER"
  "pendingWrites"
  "communicationLost"
  "parameterNames"
  "getParameter"
  "rawValue"
  "captureSnapshotA"
  "captureSnapshotB"
  "compareSnapshots"
  "snapshotDiffSummary"
)

required_fly=(
  "NexusEngineerPanel"
  "engineerModel: NexusEngineer"
  "id: engineerPanel"
  "text: qsTr(\"ENGINEER\")"
  "showAnalyzeTool()"
)

for token in "${required_panel[@]}"; do
  grep -Fq "$token" "$panel" || { echo "Missing ENGINEER UI invariant: $token" >&2; exit 1; }
done
for token in "${required_model[@]}"; do
  grep -Fq "$token" "$model" || { echo "Missing ENGINEER model invariant: $token" >&2; exit 1; }
done
for token in "${required_fly[@]}"; do
  grep -Fq "$token" "$fly" || { echo "Missing ENGINEER shell invariant: $token" >&2; exit 1; }
done

if grep -Eq 'sendMavCommand\(|sendCommand\(|sendMessageOnLink' "$model" "$panel"; then
  echo "ENGINEER mode must reuse QGC inspection/configuration surfaces, not generate raw flight commands." >&2
  exit 1
fi

if grep -Eiq 'fake sensor|fake rate|estimated raw|mock telemetry' "$model" "$panel"; then
  echo "ENGINEER mode must not fabricate diagnostics or stream rates." >&2
  exit 1
fi

echo "NEXUS ENGINEER architecture invariants: PASS"
