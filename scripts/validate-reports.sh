#!/usr/bin/env bash
set -euo pipefail

panel="${1:-custom/src/NexusReportsPanel.qml}"
model="${2:-custom/src/NexusReportsModel.cc}"
fly="${3:-custom/src/FlyViewCustomLayer.qml}"

required_panel=(
  "REPORTS"
  "MISSION ID"
  "OPERATOR"
  "AIRCRAFT"
  "FIRMWARE"
  "MISSION COMPLETION"
  "DATE / TIME"
  "DURATION"
  "DISTANCE"
  "MAX ALTITUDE"
  "BATTERY USED"
  "WARNING / EVENT SUMMARY"
  "ROUTE MAP UNAVAILABLE"
  "EXPORT PIPELINE"
  "PDF"
  "CSV"
  "KML"
  "LogFileParser"
  "gpsPath()"
  "fieldSamples"
)

required_model=(
  "QSettings"
  "NexusReports/"
  "routeDistanceMeters"
  "maxSampleValue"
  "batteryUsedPercent"
  "schemaVersion"
  "exportTargets"
  "missionId"
  "operator"
  "firmware"
  "missionCompletion"
)

required_fly=(
  "NexusReportsPanel"
  "reportsModel: NexusReports"
  "id: reportsPanel"
  "text: qsTr(\"REPORTS\")"
)

for token in "${required_panel[@]}"; do
  grep -Fq "$token" "$panel" || { echo "Missing REPORTS UI invariant: $token" >&2; exit 1; }
done
for token in "${required_model[@]}"; do
  grep -Fq "$token" custom/src/NexusReportsModel.h "$model" || { echo "Missing REPORTS model invariant: $token" >&2; exit 1; }
done
for token in "${required_fly[@]}"; do
  grep -Fq "$token" "$fly" || { echo "Missing REPORTS shell invariant: $token" >&2; exit 1; }
done

if grep -RIEq --include='NexusReports*.cc' --include='NexusReports*.h' --include='NexusReports*.qml'   'sendMavCommand\(|sendCommand\(|guidedController|actionArm|actionTakeoff|actionRTL|actionLand' custom/src; then
  echo "REPORTS must remain observational and command-free." >&2
  exit 1
fi

if grep -RIEq --include='NexusReports*.cc' --include='NexusReports*.h' --include='NexusReports*.qml'   'https?://|QNetworkAccessManager|QWebSocket|firebase|supabase|grpc::' custom/src; then
  echo "REPORTS must remain local-first with no direct cloud dependency." >&2
  exit 1
fi

if grep -Eq 'enabled: true.*text: "(PDF|CSV|KML)"' "$panel"; then
  echo "Future exporters must not be exposed as functional before implementation." >&2
  exit 1
fi

echo "NEXUS REPORTS architecture invariants: PASS"
