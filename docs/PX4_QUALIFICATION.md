# NEXUS PX4 Qualification

## Scope

Task 0.17 qualifies NEXUS against the pinned PX4 SITL and QGroundControl baselines used by this repository.

This is a software-in-the-loop qualification gate. It is not a substitute for bench, tethered, low-altitude, or real-aircraft validation.

## Pinned baselines

- QGroundControl: `25185047e855937d694a0174c1541d8f7d794a74`
- PX4: `d6f12ad1c4f70ad3230afd7d86e971421e02fef4`
- PX4 build image: `ghcr.io/px4/px4-dev:v1.17.0-rc2`
- Simulator target: `px4_sitl sihsim_quadx`
- MAVLink transport under test: UDP/14550

## Qualification sequence

A run is qualified only if each stage reports PASS in `nexus-px4-qualification.json`:

1. CONNECT
2. PREFLIGHT
3. MISSION_VALIDATE
4. MISSION_UPLOAD_READBACK
5. ARM
6. TAKEOFF
7. WAYPOINT_MISSION
8. HOLD
9. CONTINUE
10. RTL
11. LAND
12. DISCONNECT
13. RECONNECT
14. FAILURE_CORRUPT_MISSION
15. FAILURE_INTERRUPTED_MISSION
16. FAILURE_RECOVERY
17. QUALIFICATION

## Evidence rules

Command transmission alone is not success.

- ARM requires returned `armed=true` plus accepted ARM COMMAND_ACK.
- TAKEOFF requires accepted TAKEOFF ACK plus airborne state and positive relative altitude.
- Mission requires verified vehicle readback before execution.
- Mission execution requires PX4 mission mode and `MISSION_CURRENT` progression.
- HOLD requires the returned PX4 pause/loiter mode.
- CONTINUE requires returned mission mode and a non-regressing mission index.
- RTL requires the returned PX4 RTL mode.
- LAND requires returned land mode, landed state, and post-land disarm.
- Reconnect requires a new active vehicle session and fresh initial-connect synchronization.

## Failure qualification

The gate deliberately checks failures rather than only nominal flight:

### Corrupted mission

Malformed plan input must be rejected by NEXUS Security before it can be trusted as an operational mission.

### Interrupted mission verification

NEXUS begins a real vehicle upload/readback verification and then the test removes the UDP link.

Success requires:
- the interrupted transfer is not left verified,
- recovery identifies or otherwise proves invalidated mission trust,
- no automatic continuation is treated as verified,
- a later UDP reconnect completes fresh PX4 synchronization.

### Telemetry loss/reconnect

The suite explicitly removes the live QGC UDP link and proves that the active PX4 session disappears, then recreates the link and requires fresh initial synchronization.

## Artifacts

Every run retains, where available:

- `nexus-px4-qualification.json`
- NEXUS qualification console log
- PX4 SITL console log
- NEXUS build log
- PX4 ULog files

Artifacts are retained even on failed runs so the failed stage can be diagnosed.

## Failure policy

Any missing required stage, timeout, readback mismatch, rejected required command, stale reconnect, or unexpected vehicle state fails the qualification.

A flaky or unavailable CI runner is recorded as infrastructure failure, not converted into a software PASS.

## Next validation boundary

After SITL is green, real-aircraft qualification should repeat the lifecycle progressively:

bench/props-off -> tethered/contained -> low-altitude hover -> short waypoint mission -> hold/continue -> RTL/land -> deliberate telemetry disconnect/reconnect.

Real-aircraft testing must use the aircraft's established safety procedures, kill switch, geofence, battery limits and controlled test area.
