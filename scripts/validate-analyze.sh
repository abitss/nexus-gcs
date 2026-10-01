#!/usr/bin/env bash
set -euo pipefail

panel="${1:-custom/src/NexusAnalyzePanel.qml}"
model="${2:-custom/src/NexusAnalyzeModel.cc}"
graph="${3:-custom/src/NexusAnalyzeGraph.qml}"
fly="${4:-custom/src/FlyViewCustomLayer.qml}"

required_panel=(
  "FLIGHT HISTORY"
  "ROUTE REPLAY"
  "TELEMETRY TIMELINE"
  "MODE CHANGES"
  "EVENT TIMELINE"
  "BATTERY"
  "ALTITUDE"
  "SPEED"
  "LINK HEALTH"
  "GPS / NAVIGATION"
  "LogFileParser"
  "startLogReplay"
  "gpsPath()"
  "modeSegments"
  "logParser.events"
)

required_model=(
  "telemetrySavePath"
  "logSavePath"
  "tlog"
  "ulg"
  "bin"
  "refreshHistory"
  "selectFlight"
)

required_graph=(
  "fieldSamplesFiltered"
  "fieldMinMax"
  "LineSeries"
  "ValueAxis"
)

required_fly=(
  "NexusAnalyzePanel"
  "analyzeModel: NexusAnalyze"
  "id: analyzePanel"
  "text: qsTr(\"ANALYZE\")"
)

for token in "${required_panel[@]}"; do
  grep -Fq "$token" "$panel" || { echo "Missing ANALYZE UI invariant: $token" >&2; exit 1; }
done
for token in "${required_model[@]}"; do
  grep -Fq "$token" "$model" || { echo "Missing ANALYZE model invariant: $token" >&2; exit 1; }
done
for token in "${required_graph[@]}"; do
  grep -Fq "$token" "$graph" || { echo "Missing ANALYZE graph invariant: $token" >&2; exit 1; }
done
for token in "${required_fly[@]}"; do
  grep -Fq "$token" "$fly" || { echo "Missing ANALYZE shell invariant: $token" >&2; exit 1; }
done

if grep -RIEq --include='NexusAnalyze*.cc' --include='NexusAnalyze*.h' --include='NexusAnalyze*.qml'   'sendMavCommand\(|sendCommand\(|guidedController|actionArm|actionTakeoff|actionRTL|actionLand' custom/src; then
  echo "ANALYZE must remain observational and command-free." >&2
  exit 1
fi

if grep -RIEq --include='NexusAnalyze*.cc' --include='NexusAnalyze*.h' --include='NexusAnalyze*.qml'   'https?://|QNetworkAccessManager|QWebSocket|firebase|supabase|grpc::' custom/src; then
  echo "ANALYZE must remain local-first with no direct cloud dependency." >&2
  exit 1
fi

echo "NEXUS ANALYZE architecture invariants: PASS"
