# NEXUS GCS 1.0.0 — Installation Guide

## 1. Release files

A production release bundle should contain:

- `NEXUS-GCS-1.0.0-arm64-v8a.apk`
- `NEXUS-GCS-1.0.0-arm64-v8a.apk.sha256`
- `NEXUS-GCS-1.0.0-arm64-v8a.signing.txt`
- `release-manifest.json`
- release notes and documentation

Do not install an APK labeled debug, unsigned, CI-debug, development, or from an unknown source.

## 2. Supported package identity

Android package:

`com.abitss.nexusgcs`

Release architecture:

`arm64-v8a`

The exact Android minimum/target SDK is inherited from the pinned QGroundControl/Qt build baseline and is recorded by the build system.

## 3. Verify the release before installation

### SHA-256

On Linux/macOS:

```bash
sha256sum NEXUS-GCS-1.0.0-arm64-v8a.apk
```

or on macOS where `sha256sum` is unavailable:

```bash
shasum -a 256 NEXUS-GCS-1.0.0-arm64-v8a.apk
```

Compare the output exactly with the provided `.sha256` file.

### APK signing certificate

If Android SDK Build Tools are installed:

```bash
apksigner verify --verbose --print-certs NEXUS-GCS-1.0.0-arm64-v8a.apk
```

Compare the certificate SHA-256 fingerprint with the trusted release fingerprint supplied by the NEXUS release owner.

Do not install if the hash or certificate fingerprint differs.

## 4. Prepare the Android tablet

Before first use:

1. Charge the tablet sufficiently for setup.
2. Ensure adequate free storage for maps, logs and media.
3. Disable aggressive battery optimization for NEXUS if the device vendor kills long-running foreground apps.
4. Keep automatic screen sleep disabled during flight operations or use NEXUS/QGC screen-awake handling.
5. Connect the intended USB/telemetry hardware only after installation unless the device prompts for USB permissions during setup.

## 5. Install

If sideloading is allowed by organizational policy:

1. Copy the verified APK to the device.
2. Permit installation from the selected trusted file manager/source for this installation only.
3. Open the APK.
4. Confirm package identity is NEXUS GCS.
5. Complete installation.
6. Disable broad “install unknown apps” permission afterward if it is not otherwise needed.

ADB installation may be used on an engineering bench:

```bash
adb install -r NEXUS-GCS-1.0.0-arm64-v8a.apk
```

## 6. First launch

On first launch:

1. Grant only the permissions required by the intended configuration.
2. Open DEVICE and confirm storage, RAM, USB/network and permission state.
3. Connect the vehicle/telemetry link.
4. Confirm heartbeat and vehicle discovery.
5. Wait for parameter synchronization.
6. Verify GPS, battery, Home and current flight mode.
7. Open Preflight and confirm the expected GO/WARNING/BLOCKED state.
8. Do not arm until the aircraft's normal physical and procedural checks are complete.

## 7. Offline preparation

Before operating without internet:

1. Download required map areas.
2. Confirm map-cache storage.
3. Store the intended mission locally.
4. Confirm vehicle configuration and required documentation are available.
5. Test the workflow in airplane mode before relying on it operationally.

## 8. Upgrade

Before upgrading:

1. Export important logs/reports/media.
2. Back up parameter files if engineering changes were made.
3. Verify the new APK hash and certificate.
4. Install using Android update or `adb install -r`.
5. Confirm the package data remains intact.
6. Re-run a bench connection/preflight check before field use.

Never downgrade to an older build during an active operation.

## 9. Uninstall

Uninstalling NEXUS may remove application-local settings, cached data or evidence depending on Android storage behavior.

Export required evidence first.

For troubleshooting, prefer collecting logs and using documented recovery steps before uninstalling.
