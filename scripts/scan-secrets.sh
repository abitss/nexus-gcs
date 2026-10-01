#!/usr/bin/env bash
set -euo pipefail

root="${1:-.}"
cd "$root"

forbidden_files='\.(jks|keystore|p12|pfx|pem|key|der|mobileprovision)$|(^|/)(signing|secrets)\.properties$|(^|/)\.env($|\.)'
if git ls-files | grep -Eiq "$forbidden_files"; then
  echo "Forbidden signing/secret file is tracked:" >&2
  git ls-files | grep -Ei "$forbidden_files" >&2
  exit 1
fi

# High-signal patterns only. Test fixtures/documentation must use explicit placeholders.
patterns=(
  '-----BEGIN (RSA |EC |OPENSSH )?PRIVATE KEY-----'
  'AKIA[0-9A-Z]{16}'
  'AIza[0-9A-Za-z_-]{35}'
  'gh[pousr]_[A-Za-z0-9_]{30,}'
  'sk-[A-Za-z0-9]{32,}'
  '(password|passwd|storepass|keypass|api[_-]?key|client[_-]?secret)[[:space:]]*[:=][[:space:]]*["''']?[A-Za-z0-9/+_=.-]{12,}'
)

for pattern in "${patterns[@]}"; do
  if git grep -nEI "$pattern" -- ':!scripts/scan-secrets.sh' ':!SECURITY.md' ':!.github/workflows/nexus-production-android.yml'; then
    echo "Potential repository secret detected. Replace with a placeholder or CI secret." >&2
    exit 1
  fi
done

echo "NEXUS repository secret/signing-material scan: PASS"
