# Changelog

## [1.0.0] — V1 Production Acceptance Target

### Added
- Unified V1 final-acceptance gate across software, simulation, real hardware, HIL, offline, failure, field, security and production-signing evidence.
- Machine-readable final acceptance report tied to one exact NEXUS source revision.
- Protected human security-review approval gate.
- Dedicated V1 failure-matrix regression gate.
- Final production identity `nexus-v1.0.0`.

### Acceptance
- V1.0.0 is released only after every required final-acceptance gate reports PASS.
- Missing, mismatched or stale evidence blocks promotion.


All notable NEXUS GCS product changes are recorded here.

The format follows a human-reviewed release history rather than automatically treating every Git commit as a user-visible product change.

## [0.1.0] — Release Candidate

### Added
- NEXUS custom QGroundControl foundation.
- Flight cockpit and guided action surface.
- PLAN mission creation, upload and readback verification.
- HEALTH, Alerts/Failsafes and Preflight modules.
- PAYLOAD/video workspace.
- VEHICLE module.
- Protected ENGINEER mode.
- Offline-first subsystem.
- ANALYZE flight-history and telemetry evidence workflow.
- REPORTS module.
- Android/device health monitoring.
- Security, offline authentication and hash-chained audit trail.
- Crash/recovery state machine.
- PX4 SITL full-lifecycle qualification.
- Real Pixhawk bench qualification harness.
- Real-Pixhawk HIL and controlled-field qualification harness.
- Final UX design tokens, responsive tablet layouts and operator-clutter reduction.
- Production release-engineering workflow and documentation set.

### Security
- Production signing is secret-backed and fail-closed.
- Signing keys and passwords are never stored in repository files.
- Release workflow verifies the APK signature before publishing artifacts.
- Release bundles contain SHA-256 checksums and explicit build provenance.

### Known qualification constraints
- Hardware/field qualification remains configuration-specific.
- A successful APK build does not replace SITL, bench, HIL or field evidence.
- Unsupported camera/gimbal features remain unavailable rather than simulated.
