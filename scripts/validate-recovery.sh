#!/usr/bin/env bash
set -euo pipefail

panel="${1:-custom/src/NexusRecoveryPanel.qml}"
model="${2:-custom/src/NexusRecoveryModel.cc}"
alerts="${3:-custom/src/NexusAlertManager.cc}"
fly="${4:-custom/src/FlyViewCustomLayer.qml}"

required_panel=(
  "CRASH + RECOVERY"
  "APP / PROCESS RESTART"
  "FOREGROUND / BACKGROUND"
  "FC REBOOT"
  "TELEMETRY / UDP"
  "USB"
  "VIDEO"
  "MISSION TRANSFER"
  "STORAGE"
  "PERMISSIONS"
  "CORRUPTED MISSION"
  "RECOVERY EVENT TIMELINE"
)

required_model=(
  "NexusRecovery"
  "applicationStateChanged"
  "communicationLostChanged"
  "MAVLINK_MSG_ID_SYSTEM_TIME"
  "bootCounterIndicatesReboot"
  "videoLost"
  "MISSION_UPLOAD_INTERRUPTED"
  "MISSION_CORRUPT"
  "STORAGE_FULL"
  "PERMISSION_REVOKED"
  "USB_DISCONNECT"
  "UDP_LOSS"
  "NEXUS_RECOVERY"
  "running"
)

required_alerts=(
  "recovery-state"
  "RECOVERY"
  "Recovery Action Required"
)

required_fly=(
  "NexusRecoveryPanel"
  "recoveryModel: NexusRecovery"
  "id: recoveryPanel"
  "text: qsTr(\"RECOVERY\")"
)

for token in "${required_panel[@]}"; do
  grep -Fq "$token" "$panel" || { echo "Missing RECOVERY UI invariant: $token" >&2; exit 1; }
done

for token in "${required_model[@]}"; do
  grep -Fq "$token" custom/src/NexusRecoveryModel.h "$model" || { echo "Missing RECOVERY model invariant: $token" >&2; exit 1; }
done

for token in "${required_alerts[@]}"; do
  grep -Fq "$token" "$alerts" || { echo "Missing RECOVERY alert invariant: $token" >&2; exit 1; }
done

for token in "${required_fly[@]}"; do
  grep -Fq "$token" "$fly" || { echo "Missing RECOVERY shell invariant: $token" >&2; exit 1; }
done

if grep -RIEq --include='NexusRecovery*.cc' --include='NexusRecovery*.h' --include='NexusRecovery*.qml'   'sendMavCommand\(|sendCommand\(|guidedController|actionArm|actionTakeoff|actionRTL|actionLand|sendToVehicle\(' custom/src; then
  echo "RECOVERY must never silently resume or generate vehicle/mission commands." >&2
  exit 1
fi

if grep -RIEq --include='NexusRecovery*.cc' --include='NexusRecovery*.h' --include='NexusRecovery*.qml'   'https?://|QNetworkAccessManager|QWebSocket|firebase|supabase|grpc::' custom/src; then
  echo "RECOVERY must remain offline-first." >&2
  exit 1
fi

echo "NEXUS CRASH + RECOVERY architecture invariants: PASS"
