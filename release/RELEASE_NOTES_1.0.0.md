# NEXUS GCS 1.0.0 — Release Notes

NEXUS GCS 1.0.0 is the first V1 production release target of the NEXUS operator ground-control system.

## V1 scope

V1 consolidates the complete operator product developed through the Phase 0 acceptance program:

- operator-first Flight cockpit
- mission planning, upload and vehicle readback verification
- health, alerts, preflight and failsafe visibility
- payload/video workflows
- vehicle configuration and protected Engineer mode
- offline-first maps, missions, logs, media and configuration
- Analyze and Reports evidence workflows
- Android/device health
- security, local authentication and audit trail
- crash/recovery handling
- PX4 SITL qualification
- real Pixhawk bench qualification
- real-Pixhawk HIL qualification
- controlled field QA
- final tablet UX
- production release engineering

## Final acceptance requirement

NEXUS GCS 1.0.0 is not accepted merely because this release metadata exists.

Promotion requires machine-verifiable PASS evidence for:

1. BUILD
2. EMULATOR
3. PX4 SITL
4. PIXHAWK BENCH
5. HIL
6. OFFLINE
7. FAILURE-MATRIX
8. FIELD QA
9. SECURITY REVIEW
10. SIGNED RELEASE APK

Only after all ten gates pass on the accepted V1 source revision may the final status become:

**NEXUS GCS V1.0.0 ACCEPTED**

## Production identity

- package: `com.abitss.nexusgcs`
- architecture: `arm64-v8a`
- build type: Release
- channel: production
- release tag: `nexus-v1.0.0`

## Verification

The V1 production bundle must contain a signed APK, SHA-256 checksum, signing-certificate evidence, release manifest, acceptance report, release notes, changelog and user/engineering documentation.

Do not distribute an APK as NEXUS GCS V1.0.0 unless the final acceptance report is PASS.
