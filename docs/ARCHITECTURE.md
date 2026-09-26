# Architecture

## Layering

```text
NEXUS UI
  |
Domain services
  |-- Safety
  |-- Alerts
  |-- Mission
  |-- Navigation
  |-- Video
  |-- Logging
  |
QGroundControl core
  |-- Vehicle Manager
  |-- Link Manager
  |-- Mission Manager
  |-- Parameter Manager
  |-- MAVLink
  |
PX4 / ArduPilot
```

## Rules
- Prefer QGC custom-build APIs.
- Do not duplicate working QGC transport/protocol logic.
- Flight-critical state must come from authoritative vehicle state.
- Core operations must work without Internet.
- Video failure must not break telemetry/control.
- Operator and engineer functions must remain separated.
- Every consequential command requires acknowledgement/state verification.
