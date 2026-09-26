# NEXUS GCS

NEXUS GCS is an Android-first UAV ground-control platform built on top of QGroundControl.

## Foundation
- QGroundControl upstream pinned as a Git submodule
- MAVLink 2
- PX4 first
- ArduPilot compatibility planned
- Offline-first operations
- Android tablet, landscape-first UI

## Primary product areas
- Flight
- Plan
- Health
- Payload
- Analyze
- Vehicle

## Architecture principle
Prefer QGroundControl custom-build and extension mechanisms. Keep upstream changes minimal and isolate NEXUS-specific code.

## Safety boundary
This repository covers vehicle command/control, navigation, telemetry, ISR/video, mission planning, safety, diagnostics, logging, and field operations. Weapon-specific target selection, terminal attack guidance, weapon release, or detonation logic is out of scope.

## Bootstrap
Clone recursively:

```bash
git clone --recurse-submodules https://github.com/abitss/nexus-gcs.git
cd nexus-gcs
git checkout develop
git submodule update --init --recursive
```

Upstream QGroundControl is pinned under `qgroundcontrol/`.
