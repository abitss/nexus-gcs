#!/usr/bin/env bash
set -euo pipefail

ROOT="${1:-.}"
cd "$ROOT"

say() { printf '\n==> %s\n' "$*"; }

say "NEXUS V1 production metadata"
python3 scripts/validate-release.py .

say "Final UX contract"
python3 scripts/validate-ux-polish.py .

say "V1 acceptance identity"
python3 - <<'PY'
import json
d=json.load(open("release/release.json",encoding="utf-8"))
assert d["product"]=="NEXUS GCS"
assert d["version"]=="1.0.0"
assert d["releaseTag"]=="nexus-v1.0.0"
assert d["androidPackage"]=="com.abitss.nexusgcs"
assert d["androidAbi"]=="arm64-v8a"
assert d["buildType"]=="Release"
assert d["channel"]=="production"
assert d["artifactBaseName"]=="NEXUS-GCS-1.0.0-arm64-v8a"
print("V1 identity: PASS")
PY

say "Production release workflow invariants"
grep -Fq 'environment: production-release' .github/workflows/nexus-production-release.yml
grep -Fq 'NEXUS_ANDROID_KEYSTORE_B64' .github/workflows/nexus-production-release.yml
grep -Fq 'NEXUS_ANDROID_CERT_SHA256' .github/workflows/nexus-production-release.yml
grep -Fq 'apksigner verify --verbose --print-certs' .github/workflows/nexus-production-release.yml
grep -Fq 'sha256sum' .github/workflows/nexus-production-release.yml
grep -Fq 'release-manifest.json' .github/workflows/nexus-production-release.yml
grep -Fq -- '-DNEXUS_SOURCE_SHA=${{ github.sha }}' .github/workflows/nexus-production-release.yml
! grep -Fq 'androiddebugkey' .github/workflows/nexus-production-release.yml
! grep -Fq 'debug.keystore' .github/workflows/nexus-production-release.yml
echo "Production workflow contract: PASS"

say "CI/production artifact separation"
grep -Fq 'NEXUS-GCS-CI-debug-signed-arm64.apk' .github/workflows/nexus-custom-android.yml
! grep -Fq 'NEXUS-GCS-1.0.0-arm64-v8a.apk' .github/workflows/nexus-custom-android.yml
echo "Artifact separation: PASS"

say "V1 final acceptance contract"
for gate in   '"BUILD"'   '"EMULATOR"'   '"PX4 SITL"'   '"PIXHAWK BENCH"'   '"HIL"'   '"OFFLINE"'   '"FAILURE-MATRIX"'   '"FIELD QA"'   '"SECURITY REVIEW"'   '"SIGNED RELEASE APK"'; do
  grep -Fq "$gate" scripts/validate-v1-acceptance.py
done
grep -Fq 'field QA app source SHA mismatch' scripts/validate-v1-acceptance.py
grep -Fq 'NEXUS_ANDROID_CERT_SHA256' .github/workflows/nexus-v1-final-acceptance.yml
grep -Fq 'environment: v1-security-review' .github/workflows/nexus-v1-security-review.yml
echo "V1 acceptance contract: PASS"

say "Self-hosted fallback workflow set"
for f in   .github/workflows/nexus-v1-self-hosted-software.yml   .github/workflows/nexus-v1-self-hosted-emulator.yml   .github/workflows/nexus-v1-self-hosted-px4-sitl.yml   .github/workflows/nexus-v1-self-hosted-offline.yml   .github/workflows/nexus-v1-self-hosted-failure-matrix.yml   .github/workflows/nexus-v1-self-hosted-security-validation.yml   .github/workflows/nexus-v1-self-hosted-security-review.yml   .github/workflows/nexus-v1-self-hosted-production-release.yml   .github/workflows/nexus-v1-self-hosted-final-acceptance.yml; do
  test -s "$f"
done
echo "Self-hosted fallback workflows: PASS"

say "HIL + field architecture"
bash scripts/validate-hil-field.sh   custom/src/NexusFieldQualificationModel.cc   custom/src/NexusFieldQualificationPanel.qml   custom/test/NexusPX4HILQualificationTest.cc   docs/HIL_FIELD_QUALIFICATION.md

say "Real Pixhawk bench architecture"
bash scripts/validate-real-pixhawk.sh   custom/test/NexusRealPixhawkBenchTest.cc   docs/REAL_PIXHAWK_QUALIFICATION.md   .github/workflows/nexus-real-pixhawk-bench.yml

say "PX4 qualification architecture"
bash scripts/validate-px4-qualification.sh   custom/test/NexusPX4QualificationTest.cc   .github/workflows/nexus-px4-qualification.yml

say "No obvious release key material"
if grep -RIE --exclude-dir=.git --exclude='*.md'   'BEGIN (RSA |EC )?PRIVATE KEY|NEXUS_ANDROID_KEYSTORE_B64[[:space:]]*[:=][[:space:]]*[^$<{]'   release scripts custom .github/workflows; then
  echo "Possible signing material found in tracked release/config files" >&2
  exit 1
fi
echo "Signing-material scan: PASS"

say "Documentation completeness"
for f in   release/RELEASE_NOTES_1.0.0.md   CHANGELOG.md   docs/INSTALL_GUIDE.md   docs/OPERATOR_GUIDE.md   docs/ENGINEER_GUIDE.md   docs/TROUBLESHOOTING.md   docs/RELEASE_ENGINEERING.md   docs/V1_FINAL_ACCEPTANCE.md; do
  test -s "$f"
done
echo "Documentation: PASS"

printf '\nNEXUS GCS V1 SOFTWARE PRODUCTION PREFLIGHT: PASS\n'
printf 'Note: this proves repository/release contracts only. Hardware, field, security approval, and production signing evidence remain mandatory.\n'
