#!/usr/bin/env python3
import json, re, sys
from pathlib import Path

if len(sys.argv) != 5:
    raise SystemExit("usage: validate-operational-hardware-evidence.py <profile.json> <bench.json> <hil.json> <expected-source-sha>")

profile_path=Path(sys.argv[1])
bench_path=Path(sys.argv[2])
hil_path=Path(sys.argv[3])
expected_sha=sys.argv[4]

def fail(msg):
    raise SystemExit("OPERATIONAL HARDWARE EVIDENCE BLOCKED: "+msg)

if not re.fullmatch(r"[0-9a-f]{40}", expected_sha):
    fail("expected source SHA must be a full lowercase 40-char commit SHA")

profile=json.loads(profile_path.read_text(encoding="utf-8"))
aircraft=profile.get("aircraft",{})
expected_system=aircraft.get("mavlinkSystemId")
expected_component=aircraft.get("mavlinkComponentId")

for label,path in [("bench",bench_path),("HIL",hil_path)]:
    if not path.is_file():
        fail(f"{label} evidence file not found")
    d=json.loads(path.read_text(encoding="utf-8"))
    if d.get("qualification")!="PASS":
        fail(f"{label} qualification is not PASS")
    if d.get("sourceSha")!=expected_sha:
        fail(f"{label} source SHA mismatch")
    if d.get("mavlinkSystemId")!=expected_system:
        fail(f"{label} MAVLink system ID mismatch")
    if d.get("mavlinkComponentId")!=expected_component:
        fail(f"{label} MAVLink component ID mismatch")
    if str(d.get("firmwareType","")).upper() not in {"PX4","MAV_AUTOPILOT_PX4"}:
        fail(f"{label} firmware identity is not PX4")

print("NEXUS operational hardware identity evidence: PASS")
