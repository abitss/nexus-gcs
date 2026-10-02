# NEXUS GCS 1.0.0 — Troubleshooting Guide

## First rule: preserve evidence

Before reinstalling, clearing app data, deleting missions or resetting the flight controller, collect the evidence that explains the failure.

Record:

- NEXUS version
- Android device/model
- vehicle/PX4 firmware
- connection type
- time of incident
- current/last flight mode
- alert/recovery messages
- app logs
- vehicle logs
- mission/report files where relevant

## App will not start

1. Confirm sufficient device storage.
2. Reboot the Android tablet.
3. Launch NEXUS without connecting USB peripherals.
4. Check Android permissions and device-health state.
5. If the previous process ended uncleanly, review RECOVERY after launch.
6. Capture Android/logcat evidence before reinstalling.

Do not immediately clear app data if local logs/maps/evidence have not been exported.

## Vehicle not detected

Check:

- cable/OTG adapter
- USB permission
- serial device presence
- telemetry-radio power
- baud/configuration
- correct UDP endpoint where applicable
- HEARTBEAT presence

On USB, disconnect/reconnect the physical link once after confirming the aircraft is in a safe state.

## Connected but parameters never finish

1. Confirm heartbeat remains stable.
2. Check MAVLink loss.
3. Inspect system messages.
4. Avoid repeatedly reconnecting while parameter transfer is making progress.
5. Try a known reliable direct USB/telemetry link on the bench.
6. Record missing-parameter state.

Do not change parameters until synchronization is trustworthy.

## GPS has no fix

Check:

- antenna connection/orientation
- outdoor sky view
- satellite count
- HDOP/VDOP
- GPS/EKF messages
- interference from nearby electronics

A GPS-dependent mission should not be forced through a navigation BLOCKED state.

## Battery unavailable or wrong

Check:

- power module connection
- PX4 battery configuration
- reported voltage/current facts
- correct battery instance

Compare GCS readings with an independent known-good measurement before changing calibration.

## Home invalid

Home may require valid navigation/arming conditions depending on the autopilot configuration.

Do not manually assume a Home coordinate.

For RTL-dependent operations, resolve Home validity before flight.

## Mission upload fails

1. Revalidate the mission.
2. Check telemetry stability.
3. Inspect command/system messages.
4. Retry only after the previous transfer is no longer active.
5. Require vehicle readback verification.

After an interrupted transfer, NEXUS intentionally invalidates prior mission trust.

## Mission readback mismatch

Do not execute the mission.

Compare:

- waypoint count/order
- coordinates
- altitude/frame
- commands/parameters
- geofence/rally data where relevant

Clear/re-upload only after preserving evidence of the mismatch.

## ARM rejected

Read:

- Preflight state
- PX4 arming checks
- system messages
- GPS/Home
- sensor/EKF health
- battery
- geofence/safety state

Do not disable safety checks merely to make ARM succeed.

## Telemetry lost in operation

1. Use the independent control/failsafe procedure.
2. Observe configured PX4 link-loss response.
3. Do not trust stale telemetry values.
4. After restoration, allow fresh synchronization.
5. Reverify interrupted mission/state before continuation.

## USB repeatedly disconnects

Check:

- OTG adapter/cable
- connector strain
- tablet power policy
- USB hub/power
- device temperature

A mechanically unreliable connection should be corrected before flight qualification.

## Video lost

Check:

- camera/stream configuration
- decoder state
- network/interface
- camera power
- payload-specific connection

Video loss should not be mistaken for telemetry loss.

## Maps missing offline

Open OFFLINE/Maps and verify:

- the required area was downloaded
- cache still exists
- adequate storage remains
- airplane-mode test works before deployment

## Storage warning/full

Export or archive:

- logs
- reports
- media

Then free storage safely.

Do not delete active evidence during an unresolved incident.

## App overheats / slows down

Review DEVICE and field-validation evidence:

- Android thermal state
- maximum temperature
- RAM
- event-loop lag
- video workload

Move the tablet out of direct sun where operationally practical, improve airflow and reduce nonessential workload.

Do not chill electronics in a way that creates condensation.

## Engineer mode unavailable

Check:

- correct authorized role
- passphrase
- lockout timer
- Security state

Do not bypass authentication through code/config edits on a production device.

## Release APK will not install

Check:

- arm64 device compatibility
- package conflict/signature mismatch
- available storage
- Android installation policy
- APK SHA-256
- signing certificate

If Android reports a signature conflict with an already-installed package, verify which signing certificate is trusted before uninstalling the existing app.

## Suspected corrupted installation

Before reinstalling:

1. export evidence
2. verify installed version/provenance
3. verify release APK hash/certificate
4. reproduce on a bench if possible

Then reinstall using the verified production APK.

## Escalation bundle

For engineering escalation, provide:

- concise problem statement
- exact reproduction steps
- expected behavior
- actual behavior
- NEXUS version
- APK SHA-256 / signing fingerprint
- Nexus commit / QGC baseline
- Android model/version
- PX4 firmware
- app logs
- PX4 ULog/DataFlash/tlog as appropriate
- mission/parameter snapshot if relevant
- screenshots/video if they add evidence

Remove credentials, private signing material and unrelated sensitive data before sharing.
