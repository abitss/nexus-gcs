# NEXUS GCS V1.0.0 — Operational Test Campaign

## Purpose

This runbook is the execution ledger for moving NEXUS from code-complete operational-acceptance machinery to real, configuration-bound operational use.

No stage is PASS because a command was sent, a screen looked correct, or a reviewer intended to approve it. PASS requires retained evidence proving the returned vehicle/software state.

The campaign applies to one exact locked configuration only.

## Status vocabulary

- CODE COMPLETE — implementation exists in the repository.
- TEST READY — prerequisites and instrumentation exist, but the test has not executed on the frozen candidate.
- EXECUTED — the test ran and evidence exists, regardless of outcome.
- PASS — evidence was independently validated.
- BLOCKED — prerequisite unavailable or failed.
- OPERATIONALLY ACCEPTED — final certificate generated from all required PASS evidence for the exact frozen configuration.

## Campaign stop rule

Any unexpected flight mode, critical GPS/EKF issue, invalid Home during a Home-dependent test, unexplained telemetry loss, app crash/restart, severe thermal state, geofence/envelope excursion, unexpected failsafe response, or loss of independent recovery authority is a FAIL/ABORT for that card.

Do not disable PX4 failsafes, geofence, or independent recovery control to obtain a pass. Do not use RF jamming.

## Frozen identity before physical escalation

Before bench/HIL/field evidence can become final acceptance evidence, freeze and retain:

- NEXUS source SHA
- production APK SHA-256
- production signing certificate SHA-256
- QGC baseline SHA
- aircraft acceptance ID
- MAVLink system ID
- MAVLink component ID
- airframe
- flight controller
- exact PX4 firmware
- complete PX4 parameter export + SHA-256
- mission baseline + SHA-256, or NONE
- geofence baseline + SHA-256
- primary telemetry path
- independent recovery/control path
- battery/power configuration
- required failsafe responses
- operational envelope
- approved mission types and payloads

The locked profile must pass scripts/validate-operational-profile.py.

## 26-stage execution ledger

| # | Stage | Initial state | PASS evidence |
|---:|---|---|---|
| 1 | Freeze aircraft/release identity | TEST READY | committed LOCKED profile + retained baseline files/hashes |
| 2 | Production software preflight | TEST READY | scripts/v1-production-preflight.sh PASS on candidate |
| 3 | Android build/install verification | TEST READY | approved BUILD workflow PASS + APK identity |
| 4 | Emulator validation | TEST READY | emulator workflow PASS |
| 5 | PX4 SITL validation | TEST READY | PX4 full qualification PASS |
| 6 | Real Pixhawk props-off bench | TEST READY | nexus-real-pixhawk-bench.json PASS, same source + MAVLink identity |
| 7 | HIL validation | TEST READY | nexus-px4-hil.json PASS, same source + MAVLink identity |
| 8 | Offline/airplane validation | TEST READY | OFFLINE gate PASS |
| 9 | Failure matrix | TEST READY | nexus-v1-failure-matrix.json qualification PASS |
| 10 | Controlled outdoor telemetry | TEST READY | OUTDOOR_TELEMETRY card PASS |
| 11 | GPS/navigation validation | TEST READY | GPS_BEHAVIOR card PASS |
| 12 | Controlled hover | TEST READY | CONTROLLED_HOVER card PASS |
| 13 | Mode-transition validation | TEST READY | MODE_TRANSITIONS card PASS |
| 14 | Short waypoint mission | TEST READY | MISSION_EXECUTION card PASS |
| 15 | Hold/Continue | TEST READY | authoritative mode/state evidence inside mission/HIL/field logs |
| 16 | RTL | TEST READY | RTL state and expected return behavior verified |
| 17 | Land | TEST READY | LAND + landed/disarmed authoritative state verified |
| 18 | Telemetry-loss/failsafe | TEST READY | LINK_DEGRADATION + FAILSAFE_BEHAVIOR PASS, no RF jamming |
| 19 | Reconnect/state resync | TEST READY | RECONNECT_RECOVERY PASS, no stale/replayed privileged state |
| 20 | Stability/performance/thermal | TEST READY | APP_STABILITY + PERFORMANCE + THERMAL_BEHAVIOR PASS |
| 21 | Post-flight evidence review | TEST READY | POST_FLIGHT_REVIEW PASS + logs retained |
| 22 | Operator sign-off | BLOCKED until field PASS | protected operator-review artifact PASS |
| 23 | Engineer sign-off | BLOCKED until bench/HIL PASS | protected engineer-review artifact PASS |
| 24 | Security-review sign-off | BLOCKED until security run | approved security-review evidence PASS |
| 25 | Production signed APK verification | BLOCKED until production secrets/environment | signed release gate PASS |
| 26 | Final operational GO / NO-GO | BLOCKED until all prior required gates PASS | operational-flight acceptance certificate |

## Progressive field order

Execute the field portion only after software, SITL, bench, HIL, offline, and failure-matrix prerequisites are green for the frozen candidate.

Recommended progression:

1. stationary outdoor telemetry
2. GPS/Home/navigation health
3. props-on preflight with no takeoff until GO
4. minimum controlled hover
5. reviewed mode transitions
6. one short HIL-qualified waypoint mission
7. Hold/Continue
8. RTL
9. Land
10. controlled telemetry interruption / restoration
11. only pre-rehearsed safe failsafe observations
12. reconnect/state-resync
13. post-flight log review

Each command must be followed by authoritative PX4/QGC state verification.

## Cross-aircraft evidence prohibition

Bench, HIL, and field evidence must match the locked profile's:

- source SHA
- MAVLink system ID
- MAVLink component ID
- PX4 identity

Final acceptance must reject evidence from another aircraft even when the NEXUS source SHA is identical.

## Requalification triggers

At minimum, requalify affected scope after changes to:

- APK or NEXUS source
- signing certificate
- PX4 firmware
- safety-critical parameters
- airframe / propulsion / power
- flight controller
- telemetry architecture
- independent recovery/control architecture
- geofence baseline
- mission baseline where fixed
- required failsafes
- approved operating envelope

An unresolved critical anomaly or incident suspends operational acceptance.

## First executable action

Do not begin with a real flight.

The first execution step is:

1. provide an eligible `nexus-v1-software` self-hosted runner so the operational acceptance harness can execute,
2. freeze the candidate source SHA after the harness passes,
3. run the local/software production preflight,
4. execute BUILD → EMULATOR → PX4 SITL → OFFLINE → FAILURE-MATRIX before physical escalation.

Only after those are green should the real Pixhawk props-off bench campaign start.
