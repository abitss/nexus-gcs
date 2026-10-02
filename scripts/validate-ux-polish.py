#!/usr/bin/env python3
from pathlib import Path
import re, sys, math

root = Path(sys.argv[1] if len(sys.argv) > 1 else ".")
src = root / "custom" / "src"

def read(name):
    return (src / name).read_text(encoding="utf-8")

def fail(msg):
    raise SystemExit("UX VALIDATION FAILED: " + msg)

tokens = read("NexusTokens.js")
def num(name):
    m = re.search(rf"var\s+{re.escape(name)}\s*=\s*([0-9.]+)", tokens)
    if not m: fail(f"missing token {name}")
    return float(m.group(1))

if num("touchMin") < 48:
    fail("touchMin must be >= 48")
if num("textCaption") < 10:
    fail("textCaption must be >= 10")
if num("textBody") < 12:
    fail("textBody must be >= 12")

def color(name):
    m = re.search(rf'var\s+{re.escape(name)}\s*=\s*"(#[0-9A-Fa-f]{{6}})"', tokens)
    if not m: fail(f"missing color token {name}")
    return m.group(1)

def rgb(h):
    h=h.lstrip("#")
    return tuple(int(h[i:i+2],16)/255.0 for i in (0,2,4))

def lum(h):
    vals=[]
    for x in rgb(h):
        vals.append(x/12.92 if x <= .04045 else ((x+.055)/1.055)**2.4)
    return .2126*vals[0]+.7152*vals[1]+.0722*vals[2]

def contrast(a,b):
    x,y=lum(a),lum(b)
    hi,lo=max(x,y),min(x,y)
    return (hi+.05)/(lo+.05)

for fg,bg,min_ratio in [
    ("textPrimary","bg",7.0),
    ("textPrimary","surface",7.0),
    ("textSecondary","surface",4.5),
    ("warning","bg",4.5),
    ("critical","bg",4.5),
    ("success","bg",4.5),
]:
    ratio=contrast(color(fg),color(bg))
    if ratio < min_ratio:
        fail(f"{fg}/{bg} contrast {ratio:.2f} < {min_ratio}")

# Shared interactive primitives must use the minimum touch token.
for name in ["NexusActionButton.qml","NexusNavItem.qml","NexusIconButton.qml"]:
    text=read(name)
    if "T.touchMin" not in text:
        fail(f"{name} does not use T.touchMin")

# No intentionally tiny operator text in the custom flight surfaces.
for p in src.glob("*.qml"):
    text=p.read_text(encoding="utf-8")
    if re.search(r"font\.pixelSize\s*:\s*[78]\b", text):
        fail(f"undersized 7/8px text remains in {p.name}")

# Primary nav must stay focused.
fly=read("FlyViewCustomLayer.qml")
m=re.search(r"id:\s*navBar[\s\S]*?RowLayout\s*\{([\s\S]*?)\n\s*\}\n\s*\}\n\s*\}\n\}", fly)
if not m:
    # Less brittle fallback: slice from navBar to final file end.
    ix=fly.find("id: navBar")
    if ix < 0: fail("navBar not found")
    nav=fly[ix:]
else:
    nav=m.group(1)
primary=re.findall(r'NexusNavItem\s*\{[\s\S]*?text:\s*qsTr\("([^"]+)"\)', nav)
# The tail can include no other nav blocks; deduplicate while preserving order.
seen=[]
for x in primary:
    if x not in seen: seen.append(x)
expected=["FLIGHT","PLAN","HEALTH","PAYLOAD","MORE"]
if seen[:5] != expected or len(seen) != 5:
    fail(f"primary navigation must be exactly {expected}, got {seen}")

for secondary in ["ANALYZE","VEHICLE","ENGINEER","OFFLINE","REPORTS","DEVICE","SECURITY","RECOVERY","VALIDATE"]:
    if f'text: qsTr("{secondary}")' not in fly:
        fail(f"secondary workspace missing from MORE: {secondary}")

# Explicit loading/empty/error state coverage.
requirements={
    "NexusVehiclePanel.qml":["NexusStateView","No vehicle connected"],
    "NexusPayloadPanel.qml":["NexusStateView","No camera or stream source"],
    "NexusEngineerPanel.qml":["NexusStateView","Engineer workspace locked"],
    "NexusAnalyzePanel.qml":["NexusStateView","Parsing flight evidence"],
    "NexusReportsPanel.qml":["NexusStateView","Report could not be generated"],
}
for name,tokens_required in requirements.items():
    text=read(name)
    for token in tokens_required:
        if token not in text: fail(f"{name} missing state invariant: {token}")

# Destructive / disruptive local actions use the standardized confirmation surface.
for name in ["NexusVehiclePanel.qml","NexusRecoveryPanel.qml"]:
    if "NexusConfirmDialog" not in read(name):
        fail(f"{name} missing standardized confirmation")

# Key side panels must clamp to the available tablet width.
for name in ["NexusVehiclePanel.qml","NexusPayloadPanel.qml","NexusEngineerPanel.qml","NexusRecoveryPanel.qml","NexusAnalyzePanel.qml","NexusReportsPanel.qml"]:
    text=read(name)
    if "parent.width - 20" not in text:
        fail(f"{name} lacks tablet width clamp")

# Motion stays cosmetic. Safety-critical alert/recovery content must not pulse/blink.
for name in ["NexusAlertPanel.qml","NexusPreflightPanel.qml","NexusRecoveryPanel.qml"]:
    text=read(name)
    if re.search(r"(SequentialAnimation|ParallelAnimation|NumberAnimation|ColorAnimation).*loops\s*:\s*Animation\.Infinite", text, re.S):
        fail(f"infinite safety-critical animation found in {name}")

print("NEXUS final UX contract: PASS")
