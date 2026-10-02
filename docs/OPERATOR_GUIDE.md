# NEXUS GCS 0.1.0 — Operator Guide

## Mission

NEXUS is an operator-first Android ground-control station built around a simple rule:

**Understand the aircraft state before issuing the next command.**

The operator workflow is centered on FLIGHT, PLAN, HEALTH, PAYLOAD and Preflight. Engineering/configuration tools live behind secondary workspaces.

## 1. Before connecting the aircraft

Confirm:

- correct aircraft and battery
- propeller/airframe condition
- approved test/operating area
- operator emergency procedure
- RC/kill/failsafe path where applicable
- tablet battery and storage
- required offline maps
- intended mission version

## 2. Connect

Connect through the approved USB/telemetry path.

NEXUS should show:

- CONNECTED
- vehicle/system identity
- current flight mode
- heartbeat/telemetry
- parameter synchronization
- GPS
- battery
- Home when available

If telemetry is lost, NEXUS intentionally hides stale live values rather than pretending they remain current.

## 3. Preflight

Open Preflight before arming.

The overall state is:

### GO
Required checks are acceptable.

### WARNING
Operation may require operator review. Read the specific warning.

### BLOCKED
Do not arm through normal workflow until the blocking condition is resolved.

Typical checks include:

- vehicle connection
- sensor/arming health
- GPS/navigation
- battery
- Home
- mission
- geofence
- datalink
- payload state

NEXUS supplements PX4/QGC checks. It does not replace physical inspection or autopilot safety logic.

## 4. PLAN

For mission flight:

1. Create or load the intended mission.
2. Review waypoint sequence and geometry.
3. Validate the mission.
4. Upload to the vehicle.
5. Require readback verification.
6. Confirm the mission is marked verified before execution.

An upload request alone is not proof that the aircraft stores the expected mission.

If upload/readback is interrupted, mission trust is invalidated and must be reverified.

## 5. FLIGHT

The primary Flight surface prioritizes:

- connection/link
- flight mode
- GPS/navigation
- battery
- mission state
- active alert
- guided actions

Guided commands use QGC's confirmation path.

Do not issue commands merely to clear a warning or test a button during live operation.

## 6. ARM / TAKEOFF

Before ARM:

- Preflight is acceptable
- aircraft area is clear
- Home/geofence are correct where required
- mission is verified if mission flight is intended
- battery has adequate margin
- operator retains the independent emergency/control path

After ARM, verify the returned armed state.

After TAKEOFF, verify airborne state, altitude and expected mode.

## 7. Mission / HOLD / CONTINUE

During mission execution:

- monitor `MISSION_CURRENT`/mission progress
- verify the expected flight mode
- watch GPS, battery, link health and alerts

HOLD should result in the aircraft's configured hold/loiter mode.

CONTINUE should return to mission execution without unexpected mission-index regression.

If vehicle behavior differs from the plan, use the approved abort/recovery procedure rather than repeatedly sending commands.

## 8. RTL and LAND

RTL and LAND are flight-critical commands.

Confirm:

- returned flight mode
- aircraft trajectory
- Home validity for RTL
- landing area condition

Do not assume a command succeeded because the button was pressed.

## 9. HEALTH

HEALTH consolidates navigation, sensors and vehicle-health evidence.

Use it when:

- Preflight is WARNING/BLOCKED
- GPS behavior is abnormal
- EKF/IMU warnings appear
- the aircraft reports unusual state changes

## 10. PAYLOAD

PAYLOAD shows supported camera/video functions.

Depending on connected hardware:

- map + video
- video main/fullscreen
- snapshot
- recording
- zoom
- gimbal
- local media paths

Unavailable hardware features remain unavailable rather than being simulated.

## 11. Alerts

Alert severity is centralized.

### INFO
Awareness/action may not be required.

### WARNING
Review before continuing or escalating the operation.

### CRITICAL
Immediate operator attention. Follow the aircraft/test procedure.

Acknowledge alerts only after understanding the underlying state.

## 12. Link loss

On telemetry loss:

1. Maintain awareness of the independent vehicle-control/failsafe path.
2. Do not rely on stale displayed telemetry.
3. Observe configured PX4 failsafe behavior.
4. When telemetry returns, allow fresh synchronization.
5. Reverify mission/state before resuming any interrupted workflow.

## 13. Recovery

RECOVERY records:

- unclean app restart
- FC reboot evidence
- telemetry/UDP loss
- USB loss
- video loss
- interrupted mission transfer
- storage full
- permission revocation

Recovery does not silently resume privileged sessions, half-uploaded missions or prior commands.

## 14. End of flight

After landing:

1. Confirm landed state.
2. Confirm disarmed.
3. Save required logs/media.
4. Open ANALYZE/REPORTS when evidence is needed.
5. Record anomalies before power cycling equipment.
6. Charge/store batteries according to the aircraft procedure.

## 15. Operator rule

When the displayed state and observed aircraft behavior disagree, trust neither blindly.

Stop the current test/operation, establish a safe aircraft state, collect evidence, and diagnose before continuing.
