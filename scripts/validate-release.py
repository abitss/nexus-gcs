#!/usr/bin/env python3
import json, re, sys
from pathlib import Path

root = Path(sys.argv[1] if len(sys.argv) > 1 else ".")
release_path = root / "release" / "release.json"
workflow_path = root / ".github" / "workflows" / "nexus-production-release.yml"
custom_path = root / "custom" / "cmake" / "CustomOverrides.cmake"

def fail(msg):
    raise SystemExit("RELEASE VALIDATION FAILED: " + msg)

if not release_path.is_file():
    fail("release/release.json missing")

data = json.loads(release_path.read_text(encoding="utf-8"))
required = [
    "product","version","releaseTag","androidPackage","androidAbi","buildType",
    "channel","qgcBaseline","artifactBaseName"
]
for key in required:
    if not str(data.get(key, "")).strip():
        fail(f"missing release field {key}")

version = data["version"]
if not re.fullmatch(r"0|[1-9]\d*\.0|[1-9]\d*\.\d+|0\.\d+\.\d+|[1-9]\d*\.\d+\.\d+", version):
    # Strict semantic x.y.z without prerelease suffix for production artifact identity.
    if not re.fullmatch(r"\d+\.\d+\.\d+", version):
        fail(f"version is not x.y.z: {version}")

expected_tag = f"nexus-v{version}"
if data["releaseTag"] != expected_tag:
    fail(f"releaseTag must be {expected_tag}")

if data["androidPackage"] != "com.abitss.nexusgcs":
    fail("unexpected Android package")
if data["androidAbi"] != "arm64-v8a":
    fail("production ABI must be arm64-v8a for 0.1.0")
if data["buildType"] != "Release":
    fail("production buildType must be Release")
if data["channel"] != "production":
    fail("release channel must be production")
if not re.fullmatch(r"[0-9a-f]{40}", data["qgcBaseline"]):
    fail("qgcBaseline must be a full 40-character SHA")
if version not in data["artifactBaseName"]:
    fail("artifactBaseName must include semantic version")

for path in [
    root / "release" / f"RELEASE_NOTES_{version}.md",
    root / "CHANGELOG.md",
    root / "docs" / "INSTALL_GUIDE.md",
    root / "docs" / "OPERATOR_GUIDE.md",
    root / "docs" / "ENGINEER_GUIDE.md",
    root / "docs" / "TROUBLESHOOTING.md",
]:
    if not path.is_file() or path.stat().st_size < 300:
        fail(f"missing/incomplete release documentation: {path.relative_to(root)}")

custom = custom_path.read_text(encoding="utf-8")
if f'set(QGC_ANDROID_PACKAGE_NAME "{data["androidPackage"]}"' not in custom:
    fail("release package does not match CustomOverrides.cmake")

if not workflow_path.is_file():
    fail("production release workflow missing")
workflow = workflow_path.read_text(encoding="utf-8")

required_workflow = [
    "environment: production-release",
    "NEXUS_ANDROID_KEYSTORE_B64",
    "NEXUS_ANDROID_KEYSTORE_ALIAS",
    "NEXUS_ANDROID_KEYSTORE_STORE_PASS",
    "NEXUS_ANDROID_KEYSTORE_KEY_PASS",
    "NEXUS_ANDROID_CERT_SHA256",
    "QT_ANDROID_SIGN_APK=ON",
    "apksigner verify",
    "sha256sum",
    "release-manifest.json",
    "QGC_STABLE_BUILD=ON",
]
for token in required_workflow:
    if token not in workflow:
        fail(f"release workflow missing invariant: {token}")

for forbidden in ["androiddebugkey", "storepass android", "keypass android", "debug.keystore"]:
    if forbidden in workflow:
        fail(f"production workflow contains debug signing material: {forbidden}")

# Prevent obvious secrets from being committed in release/config/docs scripts.
secret_pattern = re.compile(
    r"(BEGIN (?:RSA |EC |)PRIVATE KEY|keystore_password\s*=\s*['\"][^$]|"
    r"key_password\s*=\s*['\"][^$])",
    re.I
)
for base in ["release","docs","scripts","custom",".github/workflows"]:
    p = root / base
    if not p.exists():
        continue
    for file in p.rglob("*"):
        if not file.is_file():
            continue
        if file.suffix.lower() not in {".md",".json",".py",".sh",".yml",".yaml",".cmake",".qml",".cc",".h"}:
            continue
        text = file.read_text(encoding="utf-8", errors="ignore")
        if secret_pattern.search(text):
            fail(f"possible committed signing secret in {file.relative_to(root)}")

print(f"NEXUS release contract: PASS · {data['releaseTag']} · {data['androidPackage']}")
