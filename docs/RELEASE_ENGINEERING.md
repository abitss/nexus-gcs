# NEXUS GCS — Release Engineering Guide

## Release identity

Current production release metadata lives in:

`release/release.json`

For the first productized release candidate:

- Product: NEXUS GCS
- Version: 0.1.0
- Release tag: `nexus-v0.1.0`
- Android package: `com.abitss.nexusgcs`
- ABI: `arm64-v8a`
- Build type: Release
- Channel: production

This file is the NEXUS release source of truth.

## Why the QGC build receives a local tag

The pinned QGroundControl build system derives Android `versionName` and `versionCode` from the Git tags of the QGC checkout.

NEXUS does not rewrite upstream QGC versioning code.

During the production workflow only, the pinned QGC checkout receives a temporary local annotated build tag:

`v0.1.0-nexus`

This tag is not pushed upstream.

It causes the existing QGC version machinery to emit semantic Android version `0.1.0`.

The release manifest separately records the authoritative NEXUS source SHA and QGC baseline SHA.

## Protected GitHub environment

Create a GitHub Actions environment named:

`production-release`

Recommended environment protection:

- restrict deployment branches/tags
- require an authorized reviewer
- restrict who can approve production release jobs

The production signing workflow is the only NEXUS workflow that should receive release-key secrets.

## Required production secrets

Store these as environment/repository secrets available to `production-release`:

### NEXUS_ANDROID_KEYSTORE_B64

Base64 representation of the complete production Android keystore.

Generate without line-wrapping where possible:

```bash
base64 -w 0 nexus-production.keystore
```

macOS:

```bash
base64 < nexus-production.keystore | tr -d '\n'
```

Never commit the keystore or encoded value.

### NEXUS_ANDROID_KEYSTORE_ALIAS

Production signing-key alias.

### NEXUS_ANDROID_KEYSTORE_STORE_PASS

Keystore password.

### NEXUS_ANDROID_KEYSTORE_KEY_PASS

Signing-key password.

### NEXUS_ANDROID_CERT_SHA256

Trusted SHA-256 fingerprint of the production signing certificate.

Obtain it from the trusted keystore independently, for example:

```bash
keytool -list -v -keystore nexus-production.keystore -alias YOUR_ALIAS
```

Store the expected SHA-256 fingerprint as a secret.

The production workflow compares the certificate embedded in the generated APK with this expected fingerprint.

## Release key hygiene

The production keystore should:

- be generated/stored outside the repository
- have at least two secure backups
- be access-controlled
- never be attached to issues, chat logs or build artifacts
- never be copied into the source tree
- never use debug/default passwords
- never be replaced casually after users have installed production builds

Losing the signing key can prevent seamless updates to existing installations.

Compromise of the signing key requires an incident-response process, not merely generating another key.

## Pre-release checklist

Before creating the release tag:

1. Confirm `release/release.json`.
2. Confirm release notes.
3. Confirm CHANGELOG.
4. Confirm Install, Operator, Engineer and Troubleshooting guides.
5. Confirm no secrets are present in the repository.
6. Confirm dedicated Release Engineering validation.
7. Confirm applicable software regression gates.
8. Confirm the hardware/field qualification status for the intended aircraft configuration.
9. Confirm the production signing fingerprint from the trusted key source.
10. Confirm production-release environment protection.

Release Engineering completion does not waive outstanding aircraft qualification.

## Cut the release

After the release branch/commit is accepted, create the NEXUS tag on the exact intended source commit:

```bash
git tag -a nexus-v0.1.0 -m "NEXUS GCS 0.1.0"
git push origin nexus-v0.1.0
```

The production release workflow validates that the Git tag exactly matches `release.json`.

Do not move an already-published production release tag.

## Manual release candidate

The workflow also supports manual dispatch.

The operator must type:

`RELEASE`

This is intended for producing a controlled release candidate from an approved branch before the final tag is pushed.

A manually generated artifact is still production-signed, so environment approval and release-key controls still apply.

## Release pipeline

The production workflow performs:

1. checkout pinned QGC baseline
2. checkout NEXUS source
3. validate release metadata
4. verify QGC SHA
5. create temporary local QGC version tag
6. validate signing secrets
7. decode keystore into runner-temporary storage
8. configure Android Release build
9. sign APK through Qt Android packaging
10. verify APK signature
11. compare signing certificate SHA-256
12. verify Android package and versionName
13. generate APK SHA-256
14. generate release manifest
15. package documentation
16. delete temporary keystore
17. upload production release bundle

Missing credentials or a signing-certificate mismatch causes release failure.

There is no fallback to debug signing.

## Production bundle

Expected artifact bundle:

```text
NEXUS-GCS-0.1.0-arm64-v8a.apk
NEXUS-GCS-0.1.0-arm64-v8a.apk.sha256
NEXUS-GCS-0.1.0-arm64-v8a.signing.txt
release-manifest.json
RELEASE_NOTES_0.1.0.md
CHANGELOG.md
INSTALL_GUIDE.md
OPERATOR_GUIDE.md
ENGINEER_GUIDE.md
TROUBLESHOOTING.md
```

## Release manifest

`release-manifest.json` records:

- product
- version
- release tag
- Android package
- ABI
- build type
- release channel
- NEXUS source SHA
- QGC baseline SHA
- APK SHA-256
- APK filename

This is the provenance record for the build.

## Independent post-build verification

Before distribution, verify from a separate trusted machine where practical:

```bash
sha256sum NEXUS-GCS-0.1.0-arm64-v8a.apk
apksigner verify --verbose --print-certs NEXUS-GCS-0.1.0-arm64-v8a.apk
```

Compare:

- APK SHA-256 against the bundle checksum
- signing certificate fingerprint against the separately stored trusted fingerprint
- package ID against `com.abitss.nexusgcs`
- version against `0.1.0`

## Install smoke test

Before wider distribution:

1. install on a clean supported Android tablet
2. launch NEXUS
3. verify version/package provenance
4. verify permissions
5. test offline launch
6. connect a bench Pixhawk
7. verify heartbeat/parameters/GPS/battery/modes/Home
8. close and relaunch
9. verify no unexpected recovery/error state
10. preserve smoke-test evidence

Do not turn a release smoke test into an unplanned prop-on test.

## Updating the version

For a future release:

1. edit `release/release.json`
2. create matching release notes
3. update CHANGELOG
4. ensure `artifactBaseName` includes the new version
5. run release validation
6. use a matching immutable `nexus-vX.Y.Z` tag

Version code generation continues to use the pinned QGC Android version mechanism.

## CI APK versus production APK

The regular `NEXUS Custom Android` workflow intentionally uses a generated CI/debug signing key.

Its output is named:

`NEXUS-GCS-CI-debug-signed-arm64.apk`

It is not a production artifact.

Only `NEXUS Production Android Release` using the trusted release certificate may produce the production release bundle.
