# NEXUS Custom Build

NEXUS uses QGroundControl's official custom-build mechanism while keeping the upstream QGC repository pinned as a Git submodule.

## Source layout

```text
custom/             NEXUS source of truth
qgroundcontrol/     pinned QGC upstream
scripts/            staging/build helpers
```

## Stage locally

```bash
git submodule update --init --recursive
bash scripts/stage-custom.sh
```

The script recreates `qgroundcontrol/custom/` from the repository's `custom/` directory.

## Design rule

Never treat the staged `qgroundcontrol/custom/` directory as source of truth. Edit `custom/` in the NEXUS repository, then stage again.

## Task 0.2 scope

The first custom-build gate proves:
- custom plugin registration
- NEXUS application identity
- dark operator-mode palette baseline
- single-active-vehicle presentation
- Fly View overlay override
- standard QGC flight/mission/parameter/link infrastructure remains intact
