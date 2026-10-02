# NEXUS GCS 0.1.0 — Release Notes

NEXUS GCS 0.1.0 is the first productized Android release candidate of the NEXUS operator ground-control system.

## Highlights

- Operator-first Flight cockpit with centralized telemetry, alerts and guided actions.
- PLAN mission workflow with validation and vehicle readback verification.
- HEALTH and Preflight systems with explicit GO / WARNING / BLOCKED states.
- PAYLOAD workspace for supported video, recording, snapshot, zoom and gimbal workflows.
- VEHICLE configuration surface for firmware, airframe, sensors, power, radio, modes, safety and calibration.
- Protected ENGINEER mode with parameter, MAVLink, sensor and developer diagnostics.
- Offline-first maps, missions, configurations, logs and local media workflows.
- ANALYZE flight history, route/timeline/graph inspection and REPORTS evidence workflow.
- Android/device health, Security, Crash + Recovery and PX4 validation layers.
- Real-Pixhawk bench, HIL and controlled-field qualification harnesses.
- Final tablet UX pass with 48px touch targets, reduced cockpit clutter and field-readable contrast.

## Safety and qualification boundary

This release package does not by itself certify an aircraft for flight.

SITL, real-Pixhawk bench, HIL and controlled-field qualification evidence remain separate gates. Operators must use the configuration and operating envelope actually validated for their aircraft.

## Android package

Package ID: `com.abitss.nexusgcs`

Target release architecture: `arm64-v8a`

## Artifact verification

Every production release bundle contains:

- signed APK
- SHA-256 checksum
- APK signing-certificate report
- release manifest
- release notes
- changelog
- installation guide
- operator guide
- engineer guide
- troubleshooting guide

Verify the APK checksum and signing certificate before installation.
