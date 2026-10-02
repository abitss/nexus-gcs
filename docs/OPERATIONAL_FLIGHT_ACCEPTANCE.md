# NEXUS GCS V1.0.0 — Operational Flight Acceptance

## Purpose

This document defines the final acceptance boundary for using NEXUS GCS V1.0.0 on a real aircraft in operational flight.

Operational acceptance is **configuration-bound**.

A PASS applies only to the exact combination recorded in the locked operational profile:

- NEXUS source SHA
- production APK SHA-256
- production signing certificate
- QGC baseline
- airframe
- PX4 firmware version
- flight controller
- parameter-file hash
- mission baseline hash where used
- geofence baseline hash
- telemetry/recovery-control path
- power configuration
- failsafe configuration
- validated operating envelope

Any relevant change revokes the certificate until the affected gates are repeated.

---

# 1. Acceptance architecture

The operational chain is:

```text
NEXUS GCS V1 FINAL ACCEPTANCE
↓
LOCKED AIRCRAFT PROFILE
↓
PARAMETER / MISSION / GEOFENCE HASH VERIFICATION
↓
REAL PIXHAWK BENCH PASS
↓
HIL PASS
↓
OPERATIONAL FIELD QA PASS
↓
OPERATOR FLIGHT REVIEW
↓
ENGINEER AIRWORTHINESS REVIEW
↓
OPERATIONAL FLIGHT RELEASE
↓
NEXUS GCS V1.0.0 + TESTED AIRCRAFT
OPERATIONALLY ACCEPTED
```

There is no generic fleet-wide acceptance.

---

# 2. Locked operational profile

Start from:

`release/operational/aircraft-profile.template.json`

Create a committed profile under:

`release/operational/<aircraft-id>.json`

Set:

`"status": "LOCKED"`

Only after every field has been replaced with real values.

Validate locally:

```bash
python3 scripts/validate-operational-profile.py release/operational/<aircraft-id>.json
```

The validator blocks:

- placeholders
- missing source/APK/certificate hashes
- missing parameter/geofence fingerprints
- disabled/unknown required failsafes
- invalid envelope values
- horizontal range beyond geofence radius
- RTL altitude above accepted maximum altitude
- missing approved mission types
- VLOS disabled for V1

---

# 3. Baseline evidence files

Preserve the exact files used to produce profile hashes.

## Parameters

Export the complete PX4 parameter set after final configuration freeze.

Compute:

```bash
sha256sum <parameter-file>
```

Store the hash in:

`aircraft.parameterFileSha256`

## Mission baseline

If the operational acceptance applies to a fixed mission baseline:

```bash
sha256sum <mission-file>
```

Store the hash in:

`aircraft.missionBaselineSha256`

If no fixed mission baseline is part of the approval, use:

`NONE`

This does **not** authorize arbitrary missions. Mission types remain restricted by `scope.approvedMissionTypes` and normal Plan/Preflight validation.

## Geofence baseline

A real geofence baseline is mandatory for V1 operational acceptance.

Compute:

```bash
sha256sum <geofence-file>
```

Store the hash in:

`aircraft.geofenceBaselineSha256`

The operational release workflow independently recomputes all supplied baseline hashes.

---

# 4. Operational envelope

The profile must explicitly define:

- maximum altitude
- maximum horizontal distance
- maximum ground speed
- minimum battery reserve
- minimum GPS fix
- minimum satellite count
- maximum HDOP
- geofence radius
- RTL altitude
- visual-line-of-sight requirement
- daylight restriction for the accepted V1 profile

Use conservative values supported by actual HIL and field evidence.

The acceptance certificate does not authorize operation beyond these values.

---

# 5. Required failsafes

The locked profile must record active responses for:

- RC/control loss
- telemetry loss
- low battery
- geofence breach

Values such as:

- NONE
- DISABLED
- OFF
- UNKNOWN
- UNCONFIGURED

are rejected.

NEXUS does not replace PX4 safety authority.

---

# 6. Progressive physical acceptance sequence

## Stage A — Real Pixhawk bench

Use:

`NEXUS Real Pixhawk Bench Qualification`

Mandatory:

- props removed
- kill/emergency procedure confirmed
- physical telemetry link
- parameter synchronization
- GPS
- battery
- modes
- Home
- mission upload/readback
- props-off ARM
- immediate DISARM
- restoration of pre-test mission

Do not proceed if the bench gate fails.

---

## Stage B — Real Pixhawk HIL

Use:

`NEXUS PX4 HIL Hardware Qualification`

Mandatory:

- props removed
- simulator confirmed
- HIL flag confirmed
- mission readback
- ARM
- simulated TAKEOFF
- mission
- HOLD
- CONTINUE
- RTL
- LAND
- final DISARM

Do not move a flight-affecting behavior to the field if it has not first passed HIL.

---

# 7. Operational field cards

The final field run must contain PASS for all of the following.

## OF01 — OUTDOOR_TELEMETRY

Prove stable heartbeat/telemetry in the intended operating environment.

Abort on repeated unexpected link loss, stale state, or unexplained reconnect loops.

## OF02 — GPS_BEHAVIOR

Verify:

- 3D-or-better fix
- satellite threshold
- HDOP threshold
- valid Home
- no critical EKF/GPS state

Abort on navigation discontinuity or invalid Home during Home-dependent testing.

## OF03 — CONTROLLED_HOVER

Perform the minimum controlled hover segment needed to verify:

- stable state feedback
- expected flight mode
- altitude behavior
- link stability
- pilot abort path

This is not an endurance or envelope-expansion test.

## OF04 — MODE_TRANSITIONS

Exercise only the flight-mode transitions included in the accepted operating concept.

Verify returned vehicle state after each transition.

Unexpected mode behavior is an immediate FAIL.

## OF05 — MISSION_EXECUTION

Run one conservative, previously HIL-qualified mission inside the accepted envelope.

Require:

- upload/readback verification
- expected mission indices
- expected modes
- no unexpected route deviation
- independent pilot recovery path

## OF06 — LINK_DEGRADATION

Use a controlled link interruption or natural attenuation.

Do not jam RF.

Verify the reviewed PX4 response and clean state resynchronization after restoration.

## OF07 — FAILSAFE_BEHAVIOR

Exercise only failsafes that were already rehearsed safely in HIL.

NEXUS must accurately display/record the resulting PX4 mode/state.

## OF08 — RTL_LAND

Verify:

- RTL command/state
- valid Home
- expected return behavior
- LAND state
- landed/disarmed state

Do not infer success from command dispatch alone.

## OF09 — RECONNECT_RECOVERY

Verify:

- deliberate safe telemetry reconnect
- fresh vehicle state
- parameter/mission trust behavior
- no stale privileged state
- no accidental command replay

## OF10 — ABORT_PATH

Demonstrate the independent recovery/control path defined in the locked profile.

Do not create a hazardous condition merely to prove it.

The card verifies readiness and controlled handoff/recovery behavior.

## OF11 — APP_STABILITY

Require no unexplained process restart during the accepted nominal flight session.

## OF12 — PERFORMANCE

Current NEXUS V1 minimums:

- heartbeat >= 0.8 Hz
- MAVLink loss < 5%
- max UI/event-loop lag <= 500 ms
- free RAM >= 256 MB

## OF13 — THERMAL_BEHAVIOR

Require:

- no SEVERE / CRITICAL / EMERGENCY / SHUTDOWN thermal state
- measured temperature < 45 C when a reliable sensor exists

## OF14 — POST_FLIGHT_REVIEW

Before acceptance:

- confirm landed
- confirm disarmed
- preserve logs
- preserve field JSON
- inspect recovery events
- inspect alerts/failsafes
- record anomalies
- confirm no unresolved critical issue

A failed card is evidence. Do not immediately rerun simply to obtain green output.

---

# 8. Operational field validator

Run:

```bash
python3 scripts/validate-operational-field-evidence.py <nexus-field-*.json>
```

This is stricter than the earlier field-development gate.

It requires all fourteen operational cards and the field metric thresholds.

---

# 9. Operator flight review

Workflow:

`NEXUS V1 Operator Flight Review`

Protected environment:

`operational-flight-operator`

The operator reviewer must:

1. inspect the locked operational profile,
2. inspect the accepted field run,
3. verify the operational envelope is practical and understood,
4. review anomalies and recovery events,
5. confirm the independent abort/control path,
6. approve only if comfortable operating within the locked envelope.

Required confirmation:

`APPROVE OPERATIONAL FLIGHT`

The workflow downloads and revalidates the operational field evidence before producing a PASS artifact.

---

# 10. Engineer airworthiness review

Workflow:

`NEXUS V1 Engineer Airworthiness Review`

Protected environment:

`operational-flight-engineer`

The engineering reviewer must verify:

- exact PX4 firmware
- airframe/configuration identity
- parameter baseline
- power configuration
- telemetry/recovery path
- bench evidence
- HIL evidence
- failsafe configuration
- geofence/RTL assumptions
- operating envelope

Required confirmation:

`APPROVE CONFIGURATION`

Operator and engineer reviewers must be different identities.

---

# 11. Final operational flight release

Workflow:

`NEXUS GCS V1 Operational Flight Acceptance`

Runner label:

`nexus-operational-acceptance`

Protected environment:

`operational-flight-release`

Required inputs:

- committed locked operational profile
- exact parameter baseline file
- mission baseline file or `NONE`
- exact geofence baseline file
- V1 final acceptance run ID
- operator review run ID
- engineer review run ID
- confirmation: `RELEASE OPERATIONAL FLIGHT`

The workflow verifies:

- V1 final acceptance = ACCEPTED
- source SHA equality
- production APK SHA equality
- trusted production signing certificate equality
- QGC baseline equality
- parameter baseline hash
- mission baseline hash
- geofence baseline hash
- profile hash
- aircraft acceptance ID
- operator review
- engineer review
- different operator/engineer identities
- all original ten V1 gates

---

# 12. Final certificate

A successful operational release generates:

`NEXUS-GCS-V1.0.0-OPERATIONAL-FLIGHT-ACCEPTANCE.json`

`NEXUS-GCS-V1.0.0-OPERATIONAL-FLIGHT-ACCEPTANCE.json.sha256`

`NEXUS-GCS-V1.0.0-OPERATIONAL-FLIGHT-ACCEPTANCE.md`

The certificate records:

- exact aircraft acceptance ID
- aircraft/airframe
- FC/PX4 firmware
- source SHA
- production APK SHA
- signing certificate fingerprint
- parameter hash
- mission hash
- geofence hash
- accepted envelope
- required failsafes
- approved mission types
- approved payloads
- operator reviewer
- engineer reviewer

---

# 13. Automatic revocation

Operational acceptance is invalid if any of these changes without requalification:

- NEXUS source/APK
- signing certificate
- PX4 firmware
- parameter baseline
- accepted mission baseline
- geofence baseline
- airframe
- propulsion
- power system
- flight controller
- primary telemetry architecture
- independent recovery/control architecture
- required failsafes
- approved operating envelope

It is also suspended after:

- unresolved critical anomaly
- unexplained flyaway/mode excursion
- safety incident
- failed regression
- failed bench/HIL/field requalification

---

# 14. Daily operational GO/NO-GO

The operational certificate is not a substitute for preflight.

Before every flight:

- verify correct aircraft
- verify correct NEXUS build
- verify expected PX4 firmware/configuration
- battery acceptable
- GPS/navigation acceptable
- Home valid where required
- mission verified where used
- geofence valid
- independent recovery/control available
- Preflight not BLOCKED
- no unresolved CRITICAL alert
- operating conditions remain inside the approved envelope

If any item differs from the accepted configuration or envelope:

**NO-GO until reviewed.**

---

# 15. Meaning of OPERATIONALLY ACCEPTED

A successful certificate means:

> NEXUS GCS V1.0.0 is operationally accepted for the specifically identified aircraft configuration, software/firmware fingerprints, safety configuration, mission class, and operating envelope represented by the locked profile and acceptance evidence.

It does not mean:

- universal aircraft certification
- authorization outside applicable laws/rules
- approval for arbitrary firmware/parameter changes
- approval beyond the validated envelope
- approval for a different aircraft merely because it is similar
