# NEXUS GCS V1.0.0 — Production Readiness Runbook

## Current objective

Promote one frozen NEXUS source revision to:

**NEXUS GCS V1.0.0 ACCEPTED**

The repository is designed to fail closed. Code-complete status is not production acceptance.

---

## 1. Hosted-runner incident currently observed

GitHub Actions jobs on the repository have been observed with:

- `runner_id = 0`
- empty runner name
- zero executed steps
- no job log blob
- failure within seconds of job creation

That pattern means the job never reached a runner. It is not evidence of a compile, test or application failure.

Possible account/repository-side causes include hosted-runner availability, Actions policy, or billing/minute restrictions. The repository cannot diagnose those account settings from workflow code.

For that reason, V1 now has approved self-hosted alternatives for every cloud-dependent acceptance gate.

---

## 2. Required self-hosted runner labels

A single sufficiently capable Linux machine may carry multiple labels, or dedicated machines may be used.

### Software build

`nexus-v1-software`

Runs:

`NEXUS V1 Self-Hosted Software Gate`

Requirements:
- Linux x86_64
- Docker not required for the core build itself
- Git
- Java/keytool
- enough disk for Qt + Android SDK/NDK + QGC builds
- internet access to GitHub/package dependencies
- recommended >= 16 GB RAM
- recommended >= 80 GB free disk

### Android emulator

`nexus-v1-emulator`

Runs:

`NEXUS V1 Self-Hosted Android Emulator`

Additionally requires:
- hardware virtualization/KVM
- Android emulator support
- enough free disk for Android system images

### PX4 SITL

`nexus-v1-sitl`

Runs:

`NEXUS V1 Self-Hosted PX4 SITL Qualification`

Additionally requires:
- Docker
- host networking usable by PX4/QGC test flow

### Offline validation

`nexus-v1-offline`

Runs:

`NEXUS V1 Self-Hosted OFFLINE Validation`

### Failure matrix

`nexus-v1-failure`

Runs:

`NEXUS V1 Self-Hosted Failure Matrix`

### Security

`nexus-v1-security`

Runs:
- `NEXUS V1 Self-Hosted SECURITY Validation`
- `NEXUS V1 Self-Hosted Security Review`

The review workflow still uses protected environment:

`v1-security-review`

### Production signing

`nexus-v1-release`

Runs:

`NEXUS V1 Self-Hosted Production Android Release`

This runner is security-sensitive because the production keystore is decoded on it temporarily.

Use a trusted, access-controlled runner. Do not use a shared/untrusted machine.

### Final acceptance

`nexus-v1-acceptance`

Runs:

`NEXUS GCS V1 Self-Hosted Final Acceptance`

This machine needs:
- GitHub CLI `gh`
- Python 3
- access to workflow artifacts
- no production keystore itself, except access to the trusted certificate fingerprint through the protected environment

### Existing hardware/field labels

These remain:

- `nexus-pixhawk-bench`
- `nexus-pixhawk-hil`
- `nexus-field-evidence`

---

## 3. Protected environments

### production-release

Use for:
- production signing
- final V1 acceptance

Required secrets:
- `NEXUS_ANDROID_KEYSTORE_B64`
- `NEXUS_ANDROID_KEYSTORE_ALIAS`
- `NEXUS_ANDROID_KEYSTORE_STORE_PASS`
- `NEXUS_ANDROID_KEYSTORE_KEY_PASS`
- `NEXUS_ANDROID_CERT_SHA256`

Recommended:
- required reviewer
- limited approvers
- deployment branch/tag restrictions

### v1-security-review

Use for:
- explicit V1 security approval

Recommended:
- required authorized reviewer
- no automatic approval

---

## 4. Freeze one V1 candidate SHA

Before acceptance:

1. stop feature changes,
2. select one exact commit,
3. record its full 40-character SHA,
4. run every gate from that same revision.

Do not cherry-pick or edit after physical/field evidence is collected.

A new commit means a new acceptance candidate.

---

## 5. Local static production preflight

From the NEXUS repository root:

```bash
bash scripts/v1-production-preflight.sh .
```

Expected final line:

```text
NEXUS GCS V1 SOFTWARE PRODUCTION PREFLIGHT: PASS
```

This checks repository/release contracts only.

It does not replace builds, simulation, physical tests, security approval or signing.

---

## 6. V1 gate execution order

### Gate 1 — BUILD

Preferred hosted:
`NEXUS Custom Android`

Approved fallback:
`NEXUS V1 Self-Hosted Software Gate`

The self-hosted gate also runs Linux regression suites and always builds the Android arm64 candidate.

### Gate 2 — EMULATOR

Preferred hosted:
`NEXUS Android Emulator Smoke`

Approved fallback:
`NEXUS V1 Self-Hosted Android Emulator`

### Gate 3 — PX4 SITL

Preferred hosted:
`NEXUS PX4 Full Qualification`

Approved fallback:
`NEXUS V1 Self-Hosted PX4 SITL Qualification`

### Gate 4 — PIXHAWK BENCH

`NEXUS Real Pixhawk Bench Qualification`

Requires actual connected hardware and props-off safety interlocks.

### Gate 5 — HIL

`NEXUS PX4 HIL Hardware Qualification`

Requires real Pixhawk + HIL simulator.

### Gate 6 — OFFLINE

Preferred hosted:
`NEXUS OFFLINE-FIRST Validation`

Approved fallback:
`NEXUS V1 Self-Hosted OFFLINE Validation`

### Gate 7 — FAILURE MATRIX

Preferred hosted:
`NEXUS V1 Failure Matrix`

Approved fallback:
`NEXUS V1 Self-Hosted Failure Matrix`

### Gate 8 — FIELD QA

`NEXUS Controlled Field Evidence Validation`

The field JSON itself must contain the exact source SHA embedded by the Android candidate.

### Gate 9 — SECURITY REVIEW

Run one of:

- `NEXUS SECURITY Validation`
- `NEXUS V1 Self-Hosted SECURITY Validation`

Then approve one of:

- `NEXUS V1 Security Review`
- `NEXUS V1 Self-Hosted Security Review`

Both require exact-source evidence.

### Gate 10 — SIGNED RELEASE APK

Preferred hosted:
`NEXUS Production Android Release`

Approved fallback:
`NEXUS V1 Self-Hosted Production Android Release`

Both use the same protected environment, signing secrets, certificate validation and release bundle schema.

---

## 7. Final V1 acceptance

Preferred hosted:
`NEXUS GCS V1 Final Acceptance`

Approved fallback:
`NEXUS GCS V1 Self-Hosted Final Acceptance`

Provide successful run IDs for all ten gates.

Type:

`ACCEPT V1`

The final verifier requires:

- approved workflow identity
- workflow success
- identical source SHA
- real SITL evidence
- real bench evidence
- real HIL evidence
- field evidence with embedded matching source SHA
- failure-matrix evidence
- protected security-review evidence
- production APK checksum
- release manifest
- trusted signing certificate fingerprint

Only then is:

**NEXUS GCS V1.0.0 ACCEPTED**

generated.

---

## 8. Production signing runner hygiene

If self-hosted production signing is used:

1. use a dedicated trusted machine,
2. restrict OS login access,
3. avoid unrelated workloads,
4. keep the workspace on encrypted storage where possible,
5. do not persist the decoded keystore,
6. confirm the workflow removes the temporary keystore,
7. inspect the generated signing certificate report,
8. wipe runner workspace/cache after final release if policy requires it.

The repository never stores the production keystore.

---

## 9. APK distribution rule

Distribute only the APK from an accepted production release bundle.

Never distribute:

- `NEXUS-GCS-CI-debug-signed-arm64.apk`
- emulator APKs
- unsigned APKs
- locally renamed test APKs

Production artifact:

`NEXUS-GCS-1.0.0-arm64-v8a.apk`

Before installation verify:

- SHA-256
- package ID
- version
- signing-certificate fingerprint

---

## 10. Production-ready definition

### Software production-ready

Means:
- release contracts pass,
- Build passes,
- Emulator passes,
- SITL passes,
- Offline passes,
- Failure Matrix passes,
- Security Validation/Review passes,
- production APK is correctly signed and verified.

### Operational production-ready

Additionally requires:
- real Pixhawk bench PASS,
- HIL PASS,
- controlled Field QA PASS,
- final acceptance artifact.

Until those physical gates exist, do not label the aircraft/GCS combination operationally production accepted.
