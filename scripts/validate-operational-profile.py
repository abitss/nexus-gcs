#!/usr/bin/env python3
import json, math, re, sys
from pathlib import Path

if len(sys.argv) != 2:
    raise SystemExit("usage: validate-operational-profile.py <profile.json>")

p=Path(sys.argv[1])
d=json.loads(p.read_text(encoding="utf-8"))

def fail(msg):
    raise SystemExit("OPERATIONAL PROFILE BLOCKED: "+msg)

if d.get("schemaVersion")!="1.0":
    fail("schemaVersion must be 1.0")
if d.get("status")!="LOCKED":
    fail("profile status must be LOCKED")

def get(path):
    cur=d
    for key in path.split("."):
        if key not in cur: fail("missing "+path)
        cur=cur[key]
    return cur

def nonplaceholder(path):
    v=get(path)
    if not isinstance(v,str) or not v.strip() or "<" in v or ">" in v:
        fail(path+" is empty or placeholder")
    return v.strip()

for path in [
    "release.product","release.version","release.sourceSha","release.apkSha256",
    "release.signingCertSha256","release.qgcBaselineSha",
    "aircraft.acceptanceId","aircraft.name","aircraft.airframe","aircraft.autopilot",
    "aircraft.firmwareVersion","aircraft.flightController","aircraft.vehicleType",
    "aircraft.parameterFileSha256","links.primaryTelemetry",
    "links.independentRecoveryControl","power.batteryConfiguration",
    "power.powerModule","requiredFailsafes.rcLoss","requiredFailsafes.telemetryLoss",
    "requiredFailsafes.lowBattery","requiredFailsafes.geofence"
]:
    nonplaceholder(path)

if get("release.product")!="NEXUS GCS" or get("release.version")!="1.0.0":
    fail("profile must target NEXUS GCS 1.0.0")
if get("aircraft.autopilot")!="PX4":
    fail("V1 operational profile currently supports the qualified PX4 path only")

for path,n in [
    ("release.sourceSha",40),
    ("release.apkSha256",64),
    ("release.signingCertSha256",64),
    ("release.qgcBaselineSha",40),
    ("aircraft.parameterFileSha256",64)
]:
    if not re.fullmatch(r"[0-9A-Fa-f]{%d}"%n, get(path)):
        fail(path+f" must be {n} hex characters")

mission_hash=nonplaceholder("aircraft.missionBaselineSha256")
if mission_hash!="NONE" and not re.fullmatch(r"[0-9A-Fa-f]{64}",mission_hash):
    fail("aircraft.missionBaselineSha256 must be 64 hex characters or NONE")

geofence_hash=nonplaceholder("aircraft.geofenceBaselineSha256")
if not re.fullmatch(r"[0-9A-Fa-f]{64}",geofence_hash):
    fail("aircraft.geofenceBaselineSha256 must be a real 64-character hash")

env=get("operationalEnvelope")
numeric_positive=[
    "maxAltitudeM","maxHorizontalDistanceM","maxGroundSpeedMps",
    "geofenceRadiusM","rtlAltitudeM"
]
for k in numeric_positive:
    v=env.get(k)
    if not isinstance(v,(int,float)) or not math.isfinite(v) or v<=0:
        fail("operationalEnvelope."+k+" must be > 0")

reserve=env.get("minBatteryReservePercent")
if not isinstance(reserve,(int,float)) or reserve<10 or reserve>=100:
    fail("minBatteryReservePercent must be between 10 and 99")
if env.get("minimumGpsFixType",0)<3:
    fail("minimumGpsFixType must be >= 3")
if env.get("minimumSatellites",0)<6:
    fail("minimumSatellites must be >= 6")
if not isinstance(env.get("maximumHdop"),(int,float)) or env["maximumHdop"]<=0:
    fail("maximumHdop must be positive")
if env["maxHorizontalDistanceM"] > env["geofenceRadiusM"]:
    fail("maxHorizontalDistanceM cannot exceed geofenceRadiusM")
if env["rtlAltitudeM"] > env["maxAltitudeM"]:
    fail("rtlAltitudeM cannot exceed maxAltitudeM")
if env.get("visualLineOfSightRequired") is not True:
    fail("V1 operational acceptance requires visual line of sight")

for k in ["rcLoss","telemetryLoss","lowBattery","geofence"]:
    v=str(get("requiredFailsafes."+k)).strip().upper()
    if v in {"NONE","DISABLED","OFF","UNKNOWN","UNCONFIGURED"}:
        fail("requiredFailsafes."+k+" cannot be disabled or unknown")

missions=get("scope.approvedMissionTypes")
if not isinstance(missions,list) or not missions:
    fail("approvedMissionTypes must contain at least one reviewed mission type")

prohibited=get("scope.prohibitedConditions")
if not isinstance(prohibited,list) or len(prohibited)<5:
    fail("prohibitedConditions is incomplete")

print("NEXUS operational profile: PASS")
