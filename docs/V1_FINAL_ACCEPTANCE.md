# NEXUS GCS V1.0.0 — Final Acceptance

## Final promotion chain

NEXUS GCS V1.0.0 is accepted only when this exact chain is complete on one source revision:

```text
BUILD PASS
↓
EMULATOR PASS
↓
PX4 SITL PASS
↓
PIXHAWK BENCH PASS
↓
HIL PASS
↓
OFFLINE PASS
↓
FAILURE-MATRIX PASS
↓
FIELD QA PASS
↓
SECURITY REVIEW PASS
↓
SIGNED RELEASE APK PASS
↓
NEXUS GCS V1.0.0 ACCEPTED
```

There is no partial V1 acceptance.

## Core rule: one source SHA

Every supplied workflow run must report the same `head_sha` as the final acceptance workflow.

This prevents mixing evidence from different builds.

If even one gate was executed on a different revision, final acceptance is BLOCKED.

---

## Gate 1 — BUILD PASS

Evidence source:

`NEXUS Custom Android`

Requirements:

- successful Android arm64 build run
- exact accepted source SHA
- CI artifact clearly remains debug-signed/non-production

This gate proves the Android product can be built.

It does not satisfy production signing.

---

## Gate 2 — EMULATOR PASS

Evidence source:

`NEXUS Android Emulator Smoke`

Requirements:

- successful workflow conclusion
- exact accepted source SHA
- application installs/boots under the emulator workflow

---

## Gate 3 — PX4 SITL PASS

Evidence source:

`NEXUS PX4 Full Qualification`

Requirements:

- successful workflow conclusion
- exact accepted source SHA
- `nexus-px4-qualification.json`
- final `QUALIFICATION = PASS`

The qualification already covers the full PX4 simulated lifecycle and failure/recovery evidence defined by the PX4 qualification stage.

---

## Gate 4 — PIXHAWK BENCH PASS

Evidence source:

`NEXUS Real Pixhawk Bench Qualification`

Required hardware evidence includes:

- USB/telemetry connection
- heartbeat
- parameters
- GPS
- battery
- modes
- Home
- mission upload/download
- props-off ARM
- props-off DISARM
- restoration of the pre-test mission
- final `qualification = PASS`

The final acceptance workflow does not infer this gate from harness code.

A real successful hardware run is required.

---

## Gate 5 — HIL PASS

Evidence source:

`NEXUS PX4 HIL Hardware Qualification`

Requirements:

- real Pixhawk HIL run
- exact accepted source SHA
- HIL-enabled heartbeat
- complete HIL lifecycle
- final `qualification = PASS`

---

## Gate 6 — OFFLINE PASS

Evidence source:

`NEXUS OFFLINE-FIRST Validation`

Requirements:

- successful workflow conclusion
- exact accepted source SHA

The offline validation workflow remains responsible for the detailed offline-first and airplane-mode expectations defined earlier in the project.

---

## Gate 7 — FAILURE-MATRIX PASS

Evidence source:

`NEXUS V1 Failure Matrix`

The dedicated V1 matrix must pass all of:

- NexusPlanEditingTest
- NexusAlertManagerTest
- NexusPreflightModelTest
- NexusDeviceHealthModelTest
- NexusSecurityModelTest
- NexusRecoveryModelTest
- NexusOfflineModelTest

The workflow writes:

`nexus-v1-failure-matrix.json`

and final acceptance requires:

`qualification = PASS`

This is deliberately separate from one individual Recovery workflow.

---

## Gate 8 — FIELD QA PASS

Evidence source:

`NEXUS Controlled Field Evidence Validation`

Requirements:

- exact accepted source SHA
- field evidence passes `scripts/validate-field-evidence.py`
- all required field cards PASS
- no FAIL / OPERATOR_ABORT card
- project performance/GPS/thermal acceptance limits satisfied

The original field evidence is downloaded and revalidated during final acceptance.

---

## Gate 9 — SECURITY REVIEW PASS

Evidence source:

`NEXUS V1 Security Review`

This is a protected human review gate.

Before approval:

1. run `NEXUS SECURITY Validation` on the exact V1 source SHA,
2. inspect its evidence,
3. dispatch the V1 Security Review on that same source,
4. supply the successful Security Validation run ID,
5. type `APPROVE V1`,
6. pass the protected `v1-security-review` environment approval.

The generated review evidence records:

- V1 version
- source SHA
- Security Validation run ID
- reviewer GitHub identity
- UTC review time
- review notes
- PASS state

A code-complete Security module is not equivalent to a V1 security review.

---

## Gate 10 — SIGNED RELEASE APK PASS

Evidence source:

`NEXUS Production Android Release`

The final acceptance workflow downloads the actual V1 production bundle and verifies:

- version = 1.0.0
- package = `com.abitss.nexusgcs`
- Nexus source SHA matches all other gates
- QGC baseline matches `release/release.json`
- APK SHA-256 matches:
  - APK bytes,
  - checksum file,
  - release manifest
- signing certificate digest exists
- signing certificate digest matches trusted `NEXUS_ANDROID_CERT_SHA256`

A debug-signed CI APK cannot satisfy this gate.

---

## Protected environments

Recommended protected GitHub environments:

### production-release

Used for:

- production signing
- final V1 acceptance

Contains/accesses the trusted production certificate fingerprint and signing secrets as required.

### v1-security-review

Used for:

- explicit V1 security approval

Require an authorized reviewer.

---

## Running final acceptance

Dispatch:

`NEXUS GCS V1 Final Acceptance`

from the exact V1 source revision.

Supply successful run IDs for:

1. Build
2. Emulator
3. PX4 SITL
4. Pixhawk Bench
5. HIL
6. Offline
7. Failure Matrix
8. Field QA
9. Security Review
10. Signed Release

Then type:

`ACCEPT V1`

The workflow retrieves every run through the GitHub Actions API and requires:

- expected workflow identity
- `conclusion = success`
- exact same `head_sha`

Then it downloads and independently validates the evidence requiring deeper inspection.

---

## Final acceptance output

A successful run produces:

`NEXUS-GCS-V1.0.0-FINAL-ACCEPTANCE.json`

and:

`NEXUS-GCS-V1.0.0-FINAL-ACCEPTANCE.md`

The JSON contains:

- V1 product/version
- accepted source SHA
- all ten PASS gates
- source run IDs
- APK filename
- APK SHA-256
- Android package
- QGC baseline
- security reviewer
- final `status = ACCEPTED`

This report is the machine-readable V1 promotion record.

---

## Fail-closed behavior

Final acceptance is BLOCKED if:

- a workflow run is missing
- a workflow name is wrong
- any workflow conclusion is not success
- any source SHA differs
- PX4 SITL evidence is incomplete
- physical bench evidence is incomplete
- HIL evidence is incomplete
- failure matrix is incomplete
- field evidence fails thresholds/cards
- security review is absent/mismatched
- release manifest differs
- APK checksum differs
- certificate fingerprint differs
- production certificate secret is unavailable

Nothing is silently skipped.

---

## Tagging

The target immutable release tag is:

`nexus-v1.0.0`

Recommended promotion sequence:

1. freeze candidate source revision,
2. execute all ten gates on that SHA,
3. execute final acceptance,
4. preserve final acceptance artifact,
5. create/push the immutable `nexus-v1.0.0` tag on that accepted SHA if it was not already used for release candidate production,
6. distribute only the accepted production bundle.

Do not move a published V1 tag to a different commit.

---

## Acceptance statement

Only a successful master acceptance workflow may produce:

> **NEXUS GCS V1.0.0 ACCEPTED**

Until then the correct state is:

> **V1 ACCEPTANCE BLOCKED / PENDING EVIDENCE**
