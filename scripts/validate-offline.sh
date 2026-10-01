#!/usr/bin/env bash
set -euo pipefail

panel="${1:-custom/src/NexusOfflinePanel.qml}"
model="${2:-custom/src/NexusOfflineModel.cc}"
fly="${3:-custom/src/FlyViewCustomLayer.qml}"

required_panel=(
  "OFFLINE-FIRST"
  "OFFLINE MAPS"
  "LOCAL MISSIONS"
  "VEHICLE CONFIGURATIONS"
  "TELEMETRY + LOGS"
  "LOCAL MEDIA"
  "NO-CLOUD DEPENDENCY"
  "AIRPLANE-MODE BEHAVIOR"
  "MAP-CACHE MANAGEMENT"
  "showSettingsTool(\"Maps\")"
  "showPlanView()"
)

required_model=(
  "missionSavePath"
  "parameterSavePath"
  "settingsSavePath"
  "telemetrySavePath"
  "logSavePath"
  "videoSavePath"
  "photoSavePath"
  "QGCMapEngineManager"
  "QGCCachedTileSet"
  "isInternetAvailable"
  "cloudRequired() const { return false; }"
)

required_fly=(
  "NexusOfflinePanel"
  "offlineModel: NexusOffline"
  "id: offlinePanel"
  "text: qsTr(\"OFFLINE\")"
)

for token in "${required_panel[@]}"; do
  grep -Fq "$token" "$panel" || { echo "Missing OFFLINE UI invariant: $token" >&2; exit 1; }
done

for token in "${required_model[@]}"; do
  grep -Fq "$token" custom/src/NexusOfflineModel.h "$model" || { echo "Missing OFFLINE model invariant: $token" >&2; exit 1; }
done

for token in "${required_fly[@]}"; do
  grep -Fq "$token" "$fly" || { echo "Missing OFFLINE shell invariant: $token" >&2; exit 1; }
done

if grep -RIEq --include='*.cc' --include='*.h' --include='*.qml'   'https?://|QNetworkAccessManager|QWebSocket|grpc::|firebase|supabase|amplitude|segment\.io' custom/src; then
  echo "NEXUS custom runtime layer contains a direct cloud/network dependency." >&2
  grep -RIE --include='*.cc' --include='*.h' --include='*.qml'     'https?://|QNetworkAccessManager|QWebSocket|grpc::|firebase|supabase|amplitude|segment\.io' custom/src || true
  exit 1
fi

echo "NEXUS OFFLINE-FIRST architecture invariants: PASS"
