# NEXUS Final UX Polish

## Purpose

Task 0.20 is the operator-facing finish pass for NEXUS.

The goal is not decorative redesign. The goal is to make the GCS calmer, more legible, more consistent and easier to operate on a tablet in field conditions without changing flight semantics.

## Design principles

1. **Flight state first**
   - vehicle connection
   - mode
   - navigation
   - battery
   - alerts
   - mission state
   - guided actions

2. **Primary navigation stays small**
   - FLIGHT
   - PLAN
   - HEALTH
   - PAYLOAD
   - MORE

3. **Secondary workspaces live under MORE**
   - ANALYZE
   - VEHICLE
   - ENGINEER
   - OFFLINE
   - REPORTS
   - DEVICE
   - SECURITY
   - RECOVERY
   - VALIDATE

4. **No hidden flight semantics**
   UX changes must not alter command dispatch, PX4/QGC state authority, failsafe authority or recovery behavior.

5. **No blank states**
   Loading, empty, disconnected and parser-error states must tell the operator what is happening and what safe next action exists.

---

## Design tokens

NEXUS uses `NexusTokens.js` for shared UI values.

### Spacing
- 2 / 4 / 6 / 8 / 12 / 16 / 20 / 24 px scale
- panel padding defaults to 12–16 px
- neighboring interactive controls use at least 6–8 px separation

### Radius
- small: 8 px
- medium: 12 px
- large: 16 px

### Touch targets
- minimum interactive target: **48 x 48 px**
- compact controls may be visually small but their hit area must remain at least 48 px
- primary nav, action controls and icon controls use shared primitives

### Typography
- caption floor: 10 px
- body: 12 px
- metric: 17 px
- section: 14 px
- title: 20 px
- 7–8 px operator text is prohibited

### Motion
- fast cosmetic transition: 120 ms
- standard cosmetic transition: 180 ms
- no infinite pulsing/blinking on alerts, preflight or recovery-critical information
- motion must never delay visibility of a critical state

---

## Sunlight readability

The field palette uses:
- near-black background
- bright white primary text
- high-luminance secondary text
- saturated success/warning/critical accents

The UX validator enforces WCAG-style contrast ratios for the core text/accent tokens.

This is a practical readability gate, not a display certification.

---

## Loading, empty and error states

The shared `NexusStateView` defines:

### Loading
Shows:
- activity indicator
- clear operation title
- what is being processed

### Empty
Shows:
- why content is unavailable
- the safe next action where applicable

### Error
Shows:
- visible failure state
- concrete error text
- retry only when retry is meaningful

Current explicit coverage includes:
- no connected vehicle
- no camera/stream source
- locked Engineer workspace
- no local flight history
- flight evidence parsing
- no Reports source
- Reports parsing
- Reports parse failure

---

## Confirmation dialogs

`NexusConfirmDialog` is the shared pattern for disruptive/destructive local actions.

Current standardized flows include:
- flight-controller reboot
- clearing the Recovery event timeline

Existing QGC guided-flight confirmation remains authoritative for ARM, TAKEOFF, HOLD, RTL and LAND rather than adding a second confirmation layer.

---

## Tablet responsiveness

Side panels clamp to the available viewport rather than assuming a desktop width.

Key supported behavior:
- smaller tablet: single-column cards where needed
- larger tablet: two/three-column cards
- primary nav remains five equal-width targets
- MORE adapts from three columns to two
- Analyze/Reports consume the larger available canvas without overflowing the viewport

The exact supported device matrix can be tightened later from hardware QA results.

---

## Operator clutter reduction

The old bottom bar exposed every module as a peer-level destination.

The final structure keeps only five primary destinations visible and moves non-flight-critical workspaces behind MORE.

This reduces:
- target compression
- competing labels
- accidental taps
- visual scanning cost

without removing any feature.

---

## Touch ergonomics

Operator-facing raw `Button` controls are replaced by:
- `NexusActionButton`
- `NexusIconButton`

Both preserve Qt Button behavior while enforcing field-friendly hit areas.

This includes press/release actions such as camera zoom and gimbal movement.

---

## Acceptance

Task 0.20 requires:

- centralized design tokens
- >=48 px shared interactive targets
- no 7/8 px operator text
- five-item primary nav
- all secondary modules retained under MORE
- explicit loading / empty / error states
- standardized confirmation surfaces
- tablet viewport clamps
- core color contrast gates
- no infinite safety-critical animation
- Linux QML compile
- cockpit UI regression
- inherited Android / emulator / airplane / recovery / security / vehicle / payload / PX4 regressions

The UX pass is complete only when the dedicated UX gate and inherited regressions execute successfully.
