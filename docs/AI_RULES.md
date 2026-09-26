# AI Engineering Rules

1. Do not rewrite working QGroundControl core functionality without a documented reason.
2. Prefer official QGC custom-build or extension mechanisms.
3. Do not use mocked telemetry in production paths.
4. Do not silently swallow safety or vehicle-state errors.
5. Consequential vehicle commands require acknowledgement handling.
6. Flight-critical state must be derived from authoritative vehicle state.
7. UI must handle missing or stale telemetry safely.
8. Keep video and aircraft-control health independent.
9. Core flight workflows must work offline.
10. A task is not complete until its acceptance checks pass.
11. Avoid unrelated file changes.
12. Do not introduce cloud dependencies into flight-critical paths.
13. Update architecture docs when boundaries change.
14. Preserve the ability to update the QGC upstream pin.
15. Keep operator and engineer interfaces separated.

## Required task format
Every implementation task must state:
- Context
- Objective
- Files allowed
- Files forbidden
- Requirements
- Non-goals
- Tests
- Pass criteria
