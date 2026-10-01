# NEXUS GCS Security

## Release signing
Production Android signing material must exist only in the protected release environment. The repository must never contain a production keystore, private key, store password, key password, or signing property file.

Required protected GitHub Actions secrets:
- `NEXUS_ANDROID_KEYSTORE_B64`
- `NEXUS_ANDROID_KEY_ALIAS`
- `NEXUS_ANDROID_KEYSTORE_PASSWORD`
- `NEXUS_ANDROID_KEY_PASSWORD`
- `NEXUS_RELEASE_CERT_SHA256`

Release jobs decode the keystore only into the ephemeral runner directory, verify the APK signer certificate against the pinned certificate fingerprint, generate SHA-256 release evidence, and never upload the keystore.

## Local authentication
NEXUS local privileged roles are offline-first. Passwords/passphrases are not stored. The application stores a random salt and PBKDF2-SHA256 verifier with a high iteration count.

Roles:
- OPERATOR: normal flight workflows
- ENGINEER: protected diagnostics/configuration workflows
- ADMIN: credential administration and security management

## Audit trail
Security-sensitive actions are appended to a local JSONL audit trail using SHA-256 hash chaining. This is tamper-evident, not a substitute for an external WORM/SIEM audit backend.

## Updates
Production APK updates must be signed by the same protected Android release identity. NEXUS verifies the package SHA-256 against trusted release evidence, while Android's package manager enforces signing identity when updating an installed production app.

## Mission files
Untrusted mission files must pass NEXUS/QGC plan validation before entering an operational workflow. Invalid file type/version/structure must fail closed.

## Reporting vulnerabilities
Do not place credentials, exploitable details, or production signing material in public issues. Use the organization's private security reporting channel.
