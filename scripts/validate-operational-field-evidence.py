#!/usr/bin/env python3
import json, math, sys

if len(sys.argv) != 2:
    raise SystemExit("usage: validate-operational-field-evidence.py <nexus-field-*.json>")

with open(sys.argv[1],"r",encoding="utf-8") as f:
    d=json.load(f)

def fail(msg):
    raise SystemExit("OPERATIONAL FIELD ACCEPTANCE BLOCKED: "+msg)

if d.get("phase")!="FIELD":
    fail("evidence phase is not FIELD")
if not d.get("sourceSha") or d.get("sourceSha")=="unknown":
    fail("field evidence sourceSha missing")
for key in ["mavlinkSystemId","mavlinkComponentId"]:
    v=d.get(key)
    if isinstance(v,bool) or not isinstance(v,int) or v<1 or v>255:
        fail(f"field evidence {key} missing/invalid")
if str(d.get("firmwareType","")).upper() not in {"PX4","MAV_AUTOPILOT_PX4"}:
    fail("field evidence firmware identity is not PX4")

required={
    "OUTDOOR_TELEMETRY",
    "GPS_BEHAVIOR",
    "CONTROLLED_HOVER",
    "MODE_TRANSITIONS",
    "MISSION_EXECUTION",
    "LINK_DEGRADATION",
    "FAILSAFE_BEHAVIOR",
    "RTL_LAND",
    "RECONNECT_RECOVERY",
    "ABORT_PATH",
    "APP_STABILITY",
    "PERFORMANCE",
    "THERMAL_BEHAVIOR",
    "POST_FLIGHT_REVIEW",
}
cards=d.get("cards",[])
failed=[c for c in cards if str(c.get("status","")).upper() in {"FAIL","OPERATOR_ABORT","ABORT"}]
if failed:
    fail("field evidence contains failed/aborted operational card(s)")
passed={c.get("name") for c in cards if str(c.get("status","")).upper()=="PASS"}
missing=sorted(required-passed)
if missing:
    fail("missing operational PASS cards: "+", ".join(missing))

def num(name):
    v=d.get(name)
    return float(v) if isinstance(v,(int,float)) and math.isfinite(v) else None

hb=num("heartbeatRateHz")
loss=num("mavlinkLossPercent")
lag=num("maxEventLoopLagMs")
ram=num("minRamAvailableMb")
fix=num("gpsFix")
sats=num("satellites")
hdop=num("hdop")
temp=num("maxDeviceTempC")

checks=[
    (hb is not None and hb>=0.8,f"heartbeatRateHz={hb}"),
    (loss is not None and loss<5.0,f"mavlinkLossPercent={loss}"),
    (lag is not None and lag<=500.0,f"maxEventLoopLagMs={lag}"),
    (ram is not None and ram>=256.0,f"minRamAvailableMb={ram}"),
    (fix is not None and fix>=3,f"gpsFix={fix}"),
    (sats is not None and sats>=6,f"satellites={sats}"),
]
if hdop is not None:
    checks.append((hdop<=2.5,f"hdop={hdop}"))
if temp is not None:
    checks.append((temp<45.0,f"maxDeviceTempC={temp}"))

thermal=str(d.get("deviceThermalState","UNKNOWN")).upper()
checks.append((thermal not in {"SEVERE","CRITICAL","EMERGENCY","SHUTDOWN"},f"deviceThermalState={thermal}"))
checks.append((not bool(d.get("previousUncleanExit")),"previousUncleanExit"))

bad=[detail for ok,detail in checks if not ok]
if bad:
    fail("operational field metric failure: "+"; ".join(bad))

print("NEXUS operational field evidence: PASS")
