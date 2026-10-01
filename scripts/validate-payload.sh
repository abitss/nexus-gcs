#!/usr/bin/env bash
set -euo pipefail

panel="${1:-custom/src/NexusPayloadPanel.qml}"
model="${2:-custom/src/NexusPayloadModel.cc}"

required_panel=(
  "MAP + VIDEO"
  "VIDEO MAIN"
  "FULLSCREEN"
  "SNAPSHOT"
  "RECORD"
  "zoomContinuous"
  "gimbalRate"
  "localVideoPath"
  "localPhotoPath"
  "_swapPip"
  "videoManager.fullScreen"
)

required_model=(
  "VideoManager::instance()"
  "capturePhotosState"
  "toggleVideoRecording"
  "videoSavePath"
  "photoSavePath"
  "framerate()"
  "rtpJitterLatencyMs"
  "end-to-end latency not measured"
  "sendGimbalRate"
)

for token in "${required_panel[@]}"; do
  grep -Fq "$token" "$panel" || { echo "Missing PAYLOAD UI invariant: $token" >&2; exit 1; }
done

for token in "${required_model[@]}"; do
  grep -Fq "$token" "$model" || { echo "Missing PAYLOAD model invariant: $token" >&2; exit 1; }
done

if grep -Eiq 'fake fps|estimated fps|fake latency|estimated latency' "$model" "$panel"; then
  echo "PAYLOAD module must not fabricate FPS or end-to-end latency." >&2
  exit 1
fi

echo "NEXUS PAYLOAD architecture invariants: PASS"
