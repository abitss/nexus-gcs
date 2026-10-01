# NEXUS Real Pixhawk Bench Qualification

## Scope

Task 0.18 qualifies NEXUS against a physically connected Pixhawk/PX4 controller on the bench.

This is **not** a prop-on test and **not** a flight authorization.

## Mandatory safety interlocks

The hardware qualification refuses to run command-path testing unless all are true:

- `NEXUS_PROPS_REMOVED=YES`
- `NEXUS_KILL_SWITCH_CONFIRMED=YES`
- vehicle reports not flying
- a real serial/telemetry port is explicitly supplied

The suite contains no takeoff, throttle, mission-start, motor-test, actuator-test, RTL, land, or movement command.

The only command-path qualification is:

1. ARM
2. verify returned armed state + accepted ARM COMMAND_ACK
3. immediately DISARM
4. verify returned disarmed state + accepted DISARM COMMAND_ACK

## Required bench setup

- Pixhawk powered from the normal aircraft power path where practical
- battery/power module connected if battery telemetry is being qualified
- GPS connected and placed where a 3D fix can be achieved
- telemetry transport connected by direct USB, USB-UART, or supported serial radio
- props physically removed
- motors clear of loose objects
- aircraft restrained
- kill switch / emergency stop procedure confirmed
- no person in the propeller plane

## Qualification stages

A hardware run passes only when the evidence JSON contains PASS for:

1. USB_TELEMETRY_CONNECTION
2. HEARTBEAT
3. PARAMETERS
4. GPS
5. BATTERY
6. MODES
7. HOME
8. MISSION_UPLOAD_DOWNLOAD
9. PROPS_OFF_ARM
10. PROPS_OFF_DISARM

## Evidence

The run writes `nexus-real-pixhawk-bench.json` containing:

- serial/telemetry port
- baud
- MAVLink system ID
- default component ID
- firmware/autopilot type
- vehicle type
- heartbeat frame count
- measured heartbeat rate
- parameter count
- missing-parameter state
- GPS fix type
- satellite count
- battery voltage
- battery current when reported
- battery remaining percent when reported
- current flight mode
- available flight-mode list
- Home latitude/longitude/altitude
- mission readback fingerprint
- ARM/DISARM stage evidence
- explicit bench-only qualification scope

## Mission validation

The test uploads a small two-waypoint mission and uses `NexusPlanVerifier` to download the vehicle copy and compare the flight-critical payload.

The mission is never started.

A successful upload alone is insufficient. The vehicle readback must match.

## Connection validation

The qualification opens the operator-specified serial transport through QGC's normal `SerialConfiguration` / `LinkManager` path.

A real PX4 vehicle must then be discovered from MAVLink HEARTBEAT traffic.

## GPS

A 3D fix with at least four satellites is required.

This means the real bench may need to be moved outdoors or to a location with adequate sky visibility.

## Battery

At least one battery fact group must exist and report a finite voltage above 1 V.

Current and remaining percentage are recorded when the autopilot/power module reports them.

## Parameters

The full parameter manager must reach `parametersReady=true`.

The qualification also records the received parameter count and whether QGC reports missing parameters.

## Modes

The Pixhawk must expose at least one flight mode and a non-empty current mode.

This stage validates mode telemetry/enumeration only. It deliberately does not command a flight mode change as part of the bench qualification.

## Acceptance boundary

PASS means:

> NEXUS is bench-qualified to communicate with, synchronize, inspect, round-trip a mission to, and safely exercise the ARM/DISARM command path on this real Pixhawk configuration with props removed.

PASS does **not** mean:

- prop-on qualified
- hover qualified
- waypoint-flight qualified
- RTL-in-air qualified
- production aircraft certified

Those belong to a later progressive real-aircraft validation stage.
