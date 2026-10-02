# NEXUS GCS 1.0.0 — Engineer Guide

## Scope

ENGINEER mode is for configuration, diagnostics and evidence collection that should not clutter the normal operator workflow.

It is protected by local Security roles.

## 1. Entry

Open:

MORE → ENGINEER

Authenticate using an authorized ENGINEER or ADMIN credential.

Engineer authentication is not intentionally persisted through an unclean process restart.

## 2. Safe configuration state

Parameter-changing workflows should be used only when the model reports a safe configuration state.

Avoid parameter writes while:

- armed
- flying
- telemetry is lost
- a previous write is pending
- vehicle identity/configuration is uncertain

## 3. Parameter browser

Use QGC's parameter system for:

- search
- categories
- changed/modified filtering
- favorites
- min/max/default metadata
- exact values

Do not change a parameter solely because another aircraft uses that value.

Record the reason and expected effect for safety-critical changes.

## 4. Back up parameters

Before substantial configuration work:

1. Connect to the intended vehicle.
2. Wait for full parameter synchronization.
3. Save a complete parameter backup.
4. Name it with vehicle/configuration/date context.
5. Store it with the test evidence.

## 5. Restore parameters

Restore only from a known file for the same intended vehicle/configuration family.

After restore:

- inspect pending/reboot-required state
- reboot only while safely disarmed on the ground
- reconnect
- wait for fresh parameter synchronization
- verify changed values
- perform the relevant bench/HIL validation again

## 6. Snapshot comparison

NEXUS supports in-memory parameter snapshots A/B.

Use this to compare:

- before vs after configuration
- known-good vs experimental state
- firmware migration effects

Comparison itself is read-only.

## 7. MAVLink Inspector / stream rates

Use the inspector to examine:

- message IDs
- component IDs
- field values
- message counts
- actual stream rates
- supported rate controls

Do not increase stream rates indiscriminately. Excess traffic can degrade constrained telemetry links.

## 8. Raw sensor diagnostics

Use QGC/NEXUS diagnostics for:

- IMU
- vibration
- GPS
- EKF
- raw MAVLink sensor messages

Correlate abnormal readings with vehicle logs and physical inspection.

## 9. System messages

Inspect STATUSTEXT/events when:

- arming is rejected
- failsafe activates
- estimator state changes
- GPS/sensor health changes
- commands are rejected

Preserve the message/log evidence before rebooting the FC when practical.

## 10. Developer logs

Use App Logging for categorized runtime evidence.

When diagnosing a reproducible issue, collect:

- NEXUS version
- Nexus Git revision
- QGC baseline
- Android/device model
- connection type
- PX4 firmware version
- timestamps
- relevant app logs
- vehicle logs
- mission/parameter files when appropriate

Never include signing secrets, credentials or private keys in diagnostic bundles.

## 11. Vehicle configuration

VEHICLE provides entry points to firmware-supported setup for:

- airframe
- sensors/calibration
- power
- radio
- flight modes
- safety
- camera/payload
- advanced parameters

A reboot-required indication should be treated explicitly.

## 12. Security

Security responsibilities include:

- production signing integrity
- role separation
- local auth
- mission-file validation
- audit trail
- update verification

Do not store credentials in repository files or plain diagnostic notes.

## 13. Qualification workflow

For meaningful flight-affecting changes, regress progressively:

1. static/config review
2. SITL where applicable
3. real Pixhawk bench
4. HIL
5. controlled field test

Do not skip directly from a parameter change to an expanded flight envelope.

## 14. Release/build provenance

For production support, always identify:

- NEXUS semantic version
- APK SHA-256
- signing certificate SHA-256
- Nexus source commit
- pinned QGC baseline
- Android package ID

If any of those are unknown, treat the installation as unverified until provenance is recovered.
