#!/usr/bin/env bash
set -euo pipefail

panel="${1:-custom/src/NexusVehiclePanel.qml}"
model="${2:-custom/src/NexusVehicleModel.cc}"
fly="${3:-custom/src/FlyViewCustomLayer.qml}"

required_panel=(
  "VEHICLE"
  "AIRFRAME"
  "SENSORS / CALIBRATION"
  "POWER"
  "RADIO"
  "FLIGHT MODES"
  "SAFETY"
  "CAMERA / PAYLOAD"
  "ADVANCED PARAMETERS"
  "REBOOT REQUIRED"
  "showKnownVehicleComponentConfigPage"
  "showVehicleConfigParametersPage"
  "safeToConfigure"
  "safeToReboot"
)

required_model=(
  "vehicleRebootRequired"
  "ParameterManager::_paramSetFailure"
  "pendingWrites"
  "rebootVehicle()"
  "KnownSensorsVehicleComponent"
  "KnownPowerVehicleComponent"
  "KnownRadioVehicleComponent"
  "KnownFlightModesVehicleComponent"
  "KnownSafetyVehicleComponent"
  "firmwareMajorVersion"
  "vehicleUIDStr"
  "gitHash"
)

required_fly=(
  "NexusVehiclePanel"
  "vehicleModel: NexusVehicle"
  "id: vehiclePanel"
  "text: qsTr(\"VEHICLE\")"
)

for token in "${required_panel[@]}"; do
  grep -Fq "$token" "$panel" || { echo "Missing VEHICLE UI invariant: $token" >&2; exit 1; }
done

for token in "${required_model[@]}"; do
  grep -Fq "$token" "$model" || { echo "Missing VEHICLE model invariant: $token" >&2; exit 1; }
done

for token in "${required_fly[@]}"; do
  grep -Fq "$token" "$fly" || { echo "Missing VEHICLE shell invariant: $token" >&2; exit 1; }
done

if grep -Eq 'sendMavCommand\(|sendCommand\(' "$model" "$panel"; then
  echo "VEHICLE module must reuse QGC setup/reboot APIs, not generate raw MAVLink commands." >&2
  exit 1
fi

if grep -Eiq 'fake|mock telemetry|estimated firmware|assumed firmware' "$model" "$panel"; then
  echo "VEHICLE module must not fabricate vehicle configuration state." >&2
  exit 1
fi

echo "NEXUS VEHICLE architecture invariants: PASS"
