#!/usr/bin/env python3
import hashlib, json, os, re, sys
from pathlib import Path

if len(sys.argv) != 4:
    raise SystemExit("usage: validate-v1-acceptance.py <repo-root> <evidence-root> <expected-source-sha>")

root=Path(sys.argv[1])
ev=Path(sys.argv[2])
expected_sha=sys.argv[3]
release=json.loads((root/"release/release.json").read_text())

def fail(msg):
    raise SystemExit("V1 ACCEPTANCE BLOCKED: "+msg)

if release.get("version")!="1.0.0" or release.get("releaseTag")!="nexus-v1.0.0":
    fail("release metadata is not V1.0.0")
if not re.fullmatch(r"[0-9a-f]{40}", expected_sha):
    fail("expected source SHA is not a full commit SHA")

runs=json.loads((ev/"run-metadata.json").read_text())
expected_names={
    "BUILD":"NEXUS Custom Android",
    "EMULATOR":"NEXUS Android Emulator Smoke",
    "PX4 SITL":"NEXUS PX4 Full Qualification",
    "PIXHAWK BENCH":"NEXUS Real Pixhawk Bench Qualification",
    "HIL":"NEXUS PX4 HIL Hardware Qualification",
    "OFFLINE":"NEXUS OFFLINE-FIRST Validation",
    "FAILURE-MATRIX":"NEXUS V1 Failure Matrix",
    "FIELD QA":"NEXUS Controlled Field Evidence Validation",
    "SECURITY REVIEW":"NEXUS V1 Security Review",
    "SIGNED RELEASE APK":"NEXUS Production Android Release",
}
for gate,name in expected_names.items():
    d=runs.get(gate)
    if not d: fail(f"missing run metadata for {gate}")
    if d.get("workflow")!=name: fail(f"{gate} workflow mismatch: {d.get('workflow')}")
    if d.get("conclusion")!="success": fail(f"{gate} run is not success")
    if d.get("head_sha")!=expected_sha: fail(f"{gate} source SHA mismatch")

def find(name):
    hits=list(ev.rglob(name))
    if len(hits)!=1: fail(f"expected exactly one {name}, found {len(hits)}")
    return hits[0]

sitl=json.loads(find("nexus-px4-qualification.json").read_text())
stages={x.get("stage"):x.get("status") for x in sitl.get("stages",[])}
if stages.get("QUALIFICATION")!="PASS":
    fail("PX4 SITL qualification evidence is not PASS")

bench=json.loads(find("nexus-real-pixhawk-bench.json").read_text())
if bench.get("qualification")!="PASS":
    fail("real Pixhawk bench evidence is not PASS")
for stage in ["USB_TELEMETRY_CONNECTION","HEARTBEAT","PARAMETERS","GPS","BATTERY","MODES","HOME","MISSION_UPLOAD_DOWNLOAD","PROPS_OFF_ARM","PROPS_OFF_DISARM","MISSION_RESTORE"]:
    rows={x.get("stage"):x.get("status") for x in bench.get("stages",[])}
    if rows.get(stage)!="PASS": fail(f"bench stage missing/failed: {stage}")

hil=json.loads(find("nexus-px4-hil.json").read_text())
if hil.get("qualification")!="PASS":
    fail("HIL evidence is not PASS")

failure=json.loads(find("nexus-v1-failure-matrix.json").read_text())
if failure.get("qualification")!="PASS":
    fail("failure matrix evidence is not PASS")
required_failure={"NexusPlanEditingTest","NexusAlertManagerTest","NexusPreflightModelTest","NexusDeviceHealthModelTest","NexusSecurityModelTest","NexusRecoveryModelTest","NexusOfflineModelTest"}
passed={x.get("name") for x in failure.get("stages",[]) if x.get("status")=="PASS"}
if not required_failure.issubset(passed):
    fail("failure matrix missing required passing suites")

field_candidates=[p for p in ev.rglob("*.json") if p.name.startswith("nexus-field-")]
if len(field_candidates)!=1:
    fail(f"expected exactly one controlled field JSON, found {len(field_candidates)}")
field=json.loads(field_candidates[0].read_text())
if field.get("phase")!="FIELD": fail("field QA evidence phase is not FIELD")
if field.get("sourceSha")!=expected_sha: fail("field QA app source SHA mismatch")
required_cards={"OUTDOOR_TELEMETRY","GPS_BEHAVIOR","MISSION_EXECUTION","LINK_DEGRADATION","FAILSAFE_BEHAVIOR","APP_STABILITY","PERFORMANCE","THERMAL_BEHAVIOR"}
cards=field.get("cards",[])
passed_cards={x.get("name") for x in cards if x.get("status")=="PASS"}
if not required_cards.issubset(passed_cards): fail("field QA missing required PASS cards")
if any(x.get("status")=="FAIL" for x in cards): fail("field QA contains FAIL/ABORT card")

security=json.loads(find("nexus-v1-security-review.json").read_text())
if security.get("status")!="PASS" or security.get("sourceSha")!=expected_sha or security.get("version")!="1.0.0":
    fail("security review evidence invalid")

manifest=json.loads(find("release-manifest.json").read_text())
if manifest.get("version")!="1.0.0": fail("signed release version mismatch")
if manifest.get("androidPackage")!="com.abitss.nexusgcs": fail("signed release package mismatch")
if manifest.get("nexusSourceSha")!=expected_sha: fail("signed release source SHA mismatch")
if manifest.get("qgcBaselineSha")!=release["qgcBaseline"]: fail("signed release QGC baseline mismatch")

apk=find(release["artifactBaseName"]+".apk")
sha_file=find(release["artifactBaseName"]+".apk.sha256")
actual=hashlib.sha256(apk.read_bytes()).hexdigest()
declared=sha_file.read_text().strip().split()[0]
if actual!=declared or actual!=manifest.get("apkSha256"):
    fail("signed APK SHA-256 mismatch")

signing=find(release["artifactBaseName"]+".signing.txt").read_text()
m=re.search(r"Signer #1 certificate SHA-256 digest:\s*([0-9A-Fa-f:]+)",signing)
if not m: fail("APK signing certificate digest missing")
actual_cert=re.sub(r"[^0-9A-F]","",m.group(1).upper())
expected_cert=re.sub(r"[^0-9A-F]","",os.environ.get("NEXUS_ANDROID_CERT_SHA256","").upper())
if not expected_cert: fail("trusted production certificate fingerprint unavailable")
if actual_cert!=expected_cert: fail("APK signing certificate fingerprint mismatch")

gates=[{"gate":g,"status":"PASS","runId":runs[g]["run_id"]} for g in expected_names]
report={
    "schemaVersion":"1.0",
    "product":"NEXUS GCS",
    "version":"1.0.0",
    "sourceSha":expected_sha,
    "status":"ACCEPTED",
    "accepted":True,
    "gates":gates,
    "apk":release["artifactBaseName"]+".apk",
    "apkSha256":actual,
    "androidPackage":"com.abitss.nexusgcs",
    "qgcBaseline":release["qgcBaseline"],
    "securityReviewer":security.get("reviewer"),
}
out=ev/"NEXUS-GCS-V1.0.0-FINAL-ACCEPTANCE.json"
out.write_text(json.dumps(report,indent=2)+"\n")
print("NEXUS GCS V1.0.0 FINAL ACCEPTANCE: PASS")
