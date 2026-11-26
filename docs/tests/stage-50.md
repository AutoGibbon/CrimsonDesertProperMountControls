# stage-50: v1.4.1 reaches MOD ACTIVE on 0x6B07E5E9, and steering and braking work on the horse and one other mount

Status, date, build and the one-line result are in `docs/state/tests.json`; this file holds the detail.

## Checks

The game update to `0x6B07E5E9` doesn't break the deployed v1.4.1 build. Every static check in the `game-update` skill
passed on this build. The patch moved the chart's AND, OR and NOT opcodes to `0x28E`, which the mod doesn't use, and
the cached condition method to vtable slot 78, which the mod finds at runtime.

Pass: the log reaches `MOD ACTIVE: all features enabled.`, steering is horse-relative, and braking slows and stops the
horse and one step-down mount.

## Setup

1. The deployed asi in `bin64` is the v1.4.1 build (commit `045f9de`). No new build or DMM apply is needed.
2. `bin64\ProperHorseControls.ini` has `Logging=1`, set on 2026-11-26.

## Steps

With KBM, on open flat ground:

1. Launch the game, load save slot 1 and mount the horse.
2. Hold W and turn the camera. The horse should keep its heading and turn only with A and D.
3. From gallop 2, tap the brake key once per gait down to walk, then hold it until the horse stops.
4. Mount the wolf or the bear and repeat steps 2 and 3.

Close the game and copy `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/stage50/`.

## Logs

- **Setup:** every `lookup` line found, `lookup cachedConditionSlot: 78`, `no input answer installed`, then
  `MOD ACTIVE: all features enabled.`
- **Brake:** each tap gives one `step down start` and its `step down end`, and the hold ends with `brake gap end ...
  reason=stopped`.

## Reading the results

| Outcome | Meaning |
| --- | --- |
| `MOD INACTIVE` or a `FAILED (required)` lookup | A required lookup broke despite the static check |
| `not found (optional)` for a lookup | That feature drops out; the static check missed a change |
| Steering works but braking does nothing or flips in and out of slowing | The chart conditions changed meaning, which `check_charts.py` can't see |

## Results

## Conclusion
