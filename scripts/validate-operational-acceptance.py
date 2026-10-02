#!/usr/bin/env python3
import hashlib, json, os, re, sys
from pathlib import Path

if len(sys.argv) != 9:
    raise SystemExit("usage: validate-operational-acceptance.py <repo-root> <profile.json> <parameter-file> <mission-file-or-NONE> <geofence-file> <evidence-root> <expected-source-sha> <trusted-cert-sha256>")

root=Path(sys.argv[1])
profile_path=Path(sys.argv[2])
parameter_path=Path(sys.argv[3])
mission_arg=sys.argv[4]
geofence_path=Path(sys.argv[5])
ev=Path(sys.argv[6])
expected_sha=sys.argv[7]
trusted_cert=re.sub(r"[^0-9A-F]","",sys.argv[8].upper())

def fail(msg):
    raise SystemExit("OPERATIONAL FLIGHT ACCEPTANCE BLOCKED: "+msg)

def sha256_file(path):
    h=hashlib.sha256()
    with open(path,"rb") as f:
        for chunk in iter(lambda:f.read(1024*1024),b""):
            h.update(chunk)
    return h.hexdigest()

if not re.fullmatch(r"[0-9a-f]{40}",expected_sha):
    fail("expected source SHA must be a full lowercase 40-char commit SHA")
if not re.fullmatch(r"[0-9A-F]{64}",trusted_cert):
    fail("trusted signing certificate fingerprint is unavailable/invalid")

profile=json.loads(profile_path.read_text(encoding="utf-8"))
profile_raw=profile_path.read_bytes()
profile_sha=hashlib.sha256(profile_raw).hexdigest()

release=profile["release"]
aircraft=profile["aircraft"]
if release["sourceSha"] != expected_sha:
    fail("profile source SHA differs from accepted source")
if re.sub(r"[^0-9A-F]","",release["signingCertSha256"].upper()) != trusted_cert:
    fail("profile signing certificate fingerprint differs from trusted production certificate")
if not parameter_path.is_file():
    fail("parameter baseline file not found")
param_sha=sha256_file(parameter_path)
if param_sha.lower()!=aircraft["parameterFileSha256"].lower():
    fail("parameter baseline SHA-256 mismatch")

if not geofence_path.is_file():
    fail("geofence baseline file not found")
geofence_sha=sha256_file(geofence_path)
if geofence_sha.lower()!=aircraft["geofenceBaselineSha256"].lower():
    fail("geofence baseline SHA-256 mismatch")

mission_declared=aircraft["missionBaselineSha256"]
if mission_declared=="NONE":
    if mission_arg!="NONE":
        fail("profile declares no mission baseline but a mission file was supplied")
    mission_sha="NONE"
else:
    mission_path=Path(mission_arg)
    if not mission_path.is_file():
        fail("mission baseline file not found")
    mission_sha=sha256_file(mission_path)
    if mission_sha.lower()!=mission_declared.lower():
        fail("mission baseline SHA-256 mismatch")

def unique_json(name):
    hits=list(ev.rglob(name))
    if len(hits)!=1:
        fail(f"expected exactly one {name}, found {len(hits)}")
    return json.loads(hits[0].read_text(encoding="utf-8"))

v1=unique_json("NEXUS-GCS-V1.0.0-FINAL-ACCEPTANCE.json")
operator=unique_json("nexus-operational-operator-review.json")
engineer=unique_json("nexus-operational-engineer-review.json")

if not v1.get("accepted") or v1.get("status")!="ACCEPTED":
    fail("V1 final acceptance is not ACCEPTED")
if v1.get("version")!="1.0.0" or v1.get("sourceSha")!=expected_sha:
    fail("V1 final acceptance source/version mismatch")
if v1.get("apkSha256","").lower()!=release["apkSha256"].lower():
    fail("operational profile APK hash differs from V1 accepted APK")
if v1.get("qgcBaseline")!=release["qgcBaselineSha"]:
    fail("QGC baseline differs from V1 acceptance")

for label,row in [("operator",operator),("engineer",engineer)]:
    if row.get("status")!="PASS":
        fail(f"{label} review is not PASS")
    if row.get("sourceSha")!=expected_sha:
        fail(f"{label} review source SHA mismatch")
    if row.get("profileSha256")!=profile_sha:
        fail(f"{label} review profile hash mismatch")
    if row.get("aircraftAcceptanceId")!=aircraft["acceptanceId"]:
        fail(f"{label} review aircraft acceptance ID mismatch")

if operator.get("operator")==engineer.get("engineer"):
    fail("operator and engineer approvals must be performed by different identities")

required_gates={
    "BUILD","EMULATOR","PX4 SITL","PIXHAWK BENCH","HIL","OFFLINE",
    "FAILURE-MATRIX","FIELD QA","SECURITY REVIEW","SIGNED RELEASE APK"
}
passed={x.get("gate") for x in v1.get("gates",[]) if x.get("status")=="PASS"}
missing=sorted(required_gates-passed)
if missing:
    fail("V1 acceptance missing PASS gates: "+", ".join(missing))

certificate={
    "schemaVersion":"1.0",
    "product":"NEXUS GCS",
    "version":"1.0.0",
    "status":"OPERATIONALLY_ACCEPTED",
    "operationalFlightAccepted":True,
    "sourceSha":expected_sha,
    "profileSha256":profile_sha,
    "aircraftAcceptanceId":aircraft["acceptanceId"],
    "aircraftName":aircraft["name"],
    "airframe":aircraft["airframe"],
    "flightController":aircraft["flightController"],
    "px4FirmwareVersion":aircraft["firmwareVersion"],
    "mavlinkSystemId":aircraft["mavlinkSystemId"],
    "mavlinkComponentId":aircraft["mavlinkComponentId"],
    "parameterFileSha256":param_sha,
    "missionBaselineSha256":mission_sha,
    "geofenceBaselineSha256":geofence_sha,
    "apkSha256":release["apkSha256"],
    "signingCertSha256":release["signingCertSha256"],
    "qgcBaselineSha":release["qgcBaselineSha"],
    "operationalEnvelope":profile["operationalEnvelope"],
    "requiredFailsafes":profile["requiredFailsafes"],
    "approvedMissionTypes":profile["scope"]["approvedMissionTypes"],
    "approvedPayloads":profile["scope"]["approvedPayloads"],
    "prohibitedConditions":profile["scope"]["prohibitedConditions"],
    "operatorReviewer":operator.get("operator"),
    "engineerReviewer":engineer.get("engineer"),
    "validityRule":"VALID ONLY WHILE APK, SOURCE, PX4 FIRMWARE, PARAMETER HASH, AIRCRAFT CONFIGURATION AND APPROVED OPERATING ENVELOPE REMAIN UNCHANGED",
    "revocationTriggers":[
        "NEXUS APK/source/signing certificate changes",
        "PX4 firmware changes",
        "parameter baseline hash changes",
        "mission baseline changes when a mission baseline is approved",
        "geofence baseline hash changes",
        "airframe/propulsion/power/control-link configuration changes",
        "required failsafe configuration changes",
        "operation outside the approved envelope",
        "unresolved critical safety anomaly or incident",
        "failed regression/bench/HIL/field requalification"
    ]
}

out=ev/"NEXUS-GCS-V1.0.0-OPERATIONAL-FLIGHT-ACCEPTANCE.json"
out.write_text(json.dumps(certificate,indent=2)+"\n",encoding="utf-8")
print("NEXUS GCS V1.0.0 OPERATIONAL FLIGHT ACCEPTANCE: PASS")
