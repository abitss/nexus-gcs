#!/usr/bin/env python3
import json, math, sys

if len(sys.argv) != 2:
    raise SystemExit("usage: validate-field-evidence.py <nexus-field-*.json>")

path=sys.argv[1]
with open(path,"r",encoding="utf-8") as f:
    data=json.load(f)

if data.get("phase")!="FIELD":
    raise SystemExit("evidence phase is not FIELD")

required={
    "OUTDOOR_TELEMETRY","GPS_BEHAVIOR","MISSION_EXECUTION","LINK_DEGRADATION",
    "FAILSAFE_BEHAVIOR","APP_STABILITY","PERFORMANCE","THERMAL_BEHAVIOR"
}
cards=data.get("cards",[])
passed={c.get("name") for c in cards if c.get("status")=="PASS"}
failed=[c for c in cards if c.get("status")=="FAIL"]
missing=sorted(required-passed)
if missing:
    raise SystemExit("missing PASS cards: "+", ".join(missing))
if failed:
    raise SystemExit("field evidence contains FAIL/ABORT card(s)")

def number(name):
    v=data.get(name)
    if isinstance(v,(int,float)) and math.isfinite(v):
        return float(v)
    return None

hb=number("heartbeatRateHz")
loss=number("mavlinkLossPercent")
lag=number("maxEventLoopLagMs")
ram=number("minRamAvailableMb")
fix=number("gpsFix")
sats=number("satellites")
hdop=number("hdop")
temp=number("maxDeviceTempC")

checks=[]
checks.append((hb is not None and hb>=0.8, f"heartbeatRateHz={hb}"))
checks.append((loss is not None and loss<5.0, f"mavlinkLossPercent={loss}"))
checks.append((lag is not None and lag<=500.0, f"maxEventLoopLagMs={lag}"))
checks.append((ram is not None and ram>=256.0, f"minRamAvailableMb={ram}"))
checks.append((fix is not None and fix>=3, f"gpsFix={fix}"))
checks.append((sats is not None and sats>=6, f"satellites={sats}"))
if hdop is not None:
    checks.append((hdop<=2.5, f"hdop={hdop}"))
if temp is not None:
    checks.append((temp<45.0, f"maxDeviceTempC={temp}"))

thermal=str(data.get("deviceThermalState","UNKNOWN")).upper()
checks.append((thermal not in {"SEVERE","CRITICAL","EMERGENCY","SHUTDOWN"}, f"deviceThermalState={thermal}"))
checks.append((not bool(data.get("previousUncleanExit")), "previousUncleanExit"))

bad=[detail for ok,detail in checks if not ok]
if bad:
    raise SystemExit("field qualification metric failure: "+"; ".join(bad))

print("NEXUS controlled field evidence: PASS")
