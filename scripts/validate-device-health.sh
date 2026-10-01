#!/usr/bin/env bash
set -euo pipefail

panel="${1:-custom/src/NexusDeviceHealthPanel.qml}"
model="${2:-custom/src/NexusDeviceHealthModel.cc}"
alerts="${3:-custom/src/NexusAlertManager.cc}"
fly="${4:-custom/src/FlyViewCustomLayer.qml}"

required_panel=(
  "DEVICE HEALTH"
  "TABLET BATTERY"
  "TEMPERATURE"
  "STORAGE"
  "RAM"
  "USB"
  "NETWORK"
  "CAMERA PERMISSION"
  "LOCATION PERMISSION"
  "STORAGE PERMISSION"
  "SCREEN SLEEP"
  "overheatingWarning"
  "lowStorageWarning"
  "lowBatteryWarning"
)

required_model=(
  "getDevicePowerInfo"
  "QStorageInfo"
  "getSystemRAM"
  "/proc/meminfo"
  "SerialPortManager"
  "isInternetAvailable"
  "QCameraPermission"
  "QLocationPermission"
  "setKeepScreenOn"
  "getCurrentThermalStatus"
  "batteryWarningFor"
  "storageWarningFor"
  "thermalWarningFor"
)

required_alerts=(
  "device-overheat"
  "device-storage"
  "device-battery"
  "Ground Station Overheating"
  "Ground Station Storage Low"
  "Ground Station Battery Low"
  "DEVICE"
)

required_fly=(
  "NexusDeviceHealthPanel"
  "deviceModel: NexusDeviceHealth"
  "id: deviceHealthPanel"
  "text: qsTr(\"DEVICE\")"
)

for token in "${required_panel[@]}"; do
  grep -Fq "$token" "$panel" || { echo "Missing DEVICE HEALTH UI invariant: $token" >&2; exit 1; }
done

for token in "${required_model[@]}"; do
  grep -Fq "$token" custom/src/NexusDeviceHealthModel.h "$model" || { echo "Missing DEVICE HEALTH model invariant: $token" >&2; exit 1; }
done

for token in "${required_alerts[@]}"; do
  grep -Fq "$token" "$alerts" || { echo "Missing DEVICE HEALTH alert invariant: $token" >&2; exit 1; }
done

for token in "${required_fly[@]}"; do
  grep -Fq "$token" "$fly" || { echo "Missing DEVICE HEALTH shell invariant: $token" >&2; exit 1; }
done

if grep -RIEq --include='NexusDeviceHealth*.cc' --include='NexusDeviceHealth*.h' --include='NexusDeviceHealth*.qml'   'sendMavCommand\(|sendCommand\(|guidedController|actionArm|actionTakeoff|actionRTL|actionLand' custom/src; then
  echo "DEVICE HEALTH must never issue vehicle commands." >&2
  exit 1
fi

if grep -RIEq --include='NexusDeviceHealth*.cc' --include='NexusDeviceHealth*.h' --include='NexusDeviceHealth*.qml'   'https?://|QNetworkAccessManager|QWebSocket|firebase|supabase|grpc::' custom/src; then
  echo "DEVICE HEALTH must not introduce a cloud dependency." >&2
  exit 1
fi

echo "NEXUS DEVICE HEALTH architecture invariants: PASS"
