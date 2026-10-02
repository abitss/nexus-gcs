# NEXUS HIL + Controlled Field Qualification

## Scope

Task 0.19 is the progressive validation boundary between bench-qualified hardware and unrestricted real-world use.

It has two gates:

1. **PX4 HIL qualification with a real Pixhawk**
2. **Controlled outdoor field qualification**

A field test may begin only after the HIL gate passes for the same firmware / airframe configuration family.

---

# Gate A — Hardware-in-loop

## Purpose

Run the real Pixhawk/PX4 stack against simulated vehicle physics while NEXUS exercises the operational lifecycle.

The real controller must report the MAVLink `MAV_MODE_FLAG_HIL_ENABLED` flag. A normal connected Pixhawk is not treated as HIL merely because a simulator is open on another machine.

## Required interlocks

- propellers physically removed
- real Pixhawk attached
- simulator confirmed active
- HIL flag observed from the Pixhawk heartbeat
- no transition to controlled-field testing until HIL evidence is PASS

## HIL lifecycle

The HIL qualification requires:

- connection
- HIL-enabled heartbeat
- initial parameter / mission synchronization
- valid simulated Home/position
- mission upload + readback verification
- ARM
- simulated TAKEOFF
- waypoint mission execution
- HOLD
- CONTINUE
- RTL
- LAND
- final DISARM

Every command is followed by an authoritative returned vehicle state. Command dispatch alone is not a PASS.

## HIL evidence

The run writes `nexus-px4-hil.json` and retains the NEXUS/PX4 logs produced by the self-hosted HIL environment.

---

# Gate B — Controlled outdoor field

## Purpose

Validate the same NEXUS/PX4 configuration under real GNSS, real telemetry/RF conditions, actual tablet thermal load, and a tightly bounded real-flight envelope.

## Field prerequisites

Before arming:

- legal/organizational permission for the test area
- airframe/firmware configuration frozen for the session
- bench qualification complete
- HIL qualification complete
- visual line of sight
- designated pilot/operator
- designated observer where appropriate
- known emergency/kill procedure
- geofence configured and verified
- conservative altitude / distance / speed envelope predeclared
- adequate battery margin
- GPS/Home/preflight GO
- weather and visibility acceptable
- test cards reviewed before flight
- no unrelated experimental feature enabled during qualification

## Progressive field cards

### F01 — Outdoor telemetry

Goal: prove stable vehicle/GCS communications while stationary and during a short controlled hover/flight segment.

Record:
- heartbeat rate
- MAVLink packet loss
- reconnects
- recovery events
- current transport

Abort on:
- repeated unexpected disconnect
- unexplained mode change
- stale telemetry
- app crash/restart

### F02 — GPS behavior

Goal: verify navigation quality and continuity in the actual test environment.

Record:
- fix type
- satellites
- HDOP
- VDOP
- Home validity
- any GPS/EKF warnings

Abort on:
- fix below 3D during a GPS-dependent flight phase
- unexpected position jumps
- EKF/GPS critical state
- Home invalidation

### F03 — Mission execution

Goal: run one previously HIL-qualified, short, conservative waypoint mission.

Requirements:
- mission uploaded and readback verified
- no new mission geometry is invented during flight
- mission progress is observed from `MISSION_CURRENT`
- pilot retains an independent abort path

Record:
- mission index
- mode transitions
- duration
- route/log evidence
- any command rejection

Abort on:
- wrong mode
- unexpected waypoint sequence
- vehicle deviates from the predeclared envelope
- telemetry/GPS state becomes unsafe

### F04 — Link degradation and restoration

Goal: observe how NEXUS and PX4 behave as the normal telemetry link becomes weaker or is deliberately interrupted in a controlled way.

Rules:
- do not jam RF
- do not defeat RC/failsafe/geofence protections
- use normal range/attenuation or a controlled GCS link interruption
- only execute after the aircraft's configured loss behavior has been reviewed
- retain an independent recovery/control path

Record:
- MAVLink loss %
- telemetry-loss event
- configured failsafe/mode response
- telemetry-restored event
- fresh state synchronization

Abort on:
- failsafe response differs from the reviewed configuration
- independent control path is unavailable
- aircraft exits the defined test envelope

### F05 — Failsafe behavior

Goal: verify selected, pre-reviewed PX4 failsafes one at a time.

Qualification is observational: NEXUS must correctly display and record the aircraft's resulting mode/state. NEXUS must not override PX4's safety authority.

Only a failsafe that has already been safely rehearsed in HIL should be moved to field testing.

### F06 — App stability

Record:
- process restarts
- recovery events
- background/foreground transitions
- permission/storage faults

PASS requires no unexplained process restart during the nominal field session.

### F07 — Performance

Record:
- maximum event-loop lag
- heartbeat rate
- telemetry loss
- minimum free RAM

Default NEXUS qualification targets for the nominal segment:
- heartbeat rate >= 0.8 Hz
- MAVLink loss < 5%
- max event-loop lag <= 500 ms
- minimum available RAM >= 256 MB

These are NEXUS project acceptance thresholds, not universal certification limits.

### F08 — Thermal behavior

Record:
- maximum measured tablet/device temperature
- Android thermal state

Default NEXUS field gate:
- no SEVERE/CRITICAL/EMERGENCY/SHUTDOWN thermal state
- measured device/battery temperature below 45 C when that sensor is available

If the platform exposes no reliable temperature sensor, the field report must say UNAVAILABLE rather than inventing a value.

---

# Evidence recorder

The NEXUS VALIDATE workspace records:

- session phase: HIL or FIELD
- duration
- heartbeat rate
- MAVLink loss
- GPS fix
- satellite count
- HDOP / VDOP
- mission index
- flight mode
- maximum device temperature
- minimum available RAM
- maximum UI/event-loop lag
- recovery event count
- complete recovery-event list
- previous unclean-exit state
- device thermal state
- operator test-card results and notes

Evidence is exported locally as:

`nexus-<hil|field>-YYYYMMDD-HHMMSS.json`

No cloud connection is required.

---

# Automatic abort policy

The operator should terminate the current card and recover/land if any of the following occurs:

- unexpected flight mode
- GPS/EKF critical condition
- invalid Home during a Home-dependent flight
- unexpected telemetry loss
- failsafe behavior differs from the reviewed configuration
- NEXUS crashes/restarts
- tablet reaches a severe thermal state
- vehicle exits the declared geofence/envelope
- pilot or observer loses confidence in the test

A failed card is evidence. It must not be repeated immediately just to obtain a green result.

---

# Acceptance

Task 0.19 is complete only when:

1. the HIL lifecycle report is PASS,
2. the field evidence report contains PASS for:
   - OUTDOOR_TELEMETRY
   - GPS_BEHAVIOR
   - MISSION_EXECUTION
   - LINK_DEGRADATION
   - FAILSAFE_BEHAVIOR
   - APP_STABILITY
   - PERFORMANCE
   - THERMAL_BEHAVIOR
3. no OPERATOR_ABORT/FAIL card is present in the final accepted run,
4. raw metrics meet the documented NEXUS project thresholds,
5. logs are retained for review.

Passing this stage qualifies only the tested hardware/firmware/app configuration and the tested operating envelope.
