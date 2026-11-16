# Stage 49 test

Fixes for [stage 48](stage-48.md)'s brake taps that dropped more than one gait. A hold now only steps down the next gait
once the input that started the first step down has been down for 500 ms (`kHoldBrakeMs`), so a tap still down when the
chart moves drops one gait. `UpdateBrakeGap` now runs for the mount's input only, one call at a time, which stops the
stray step down that started alongside the slowdown at the lowest gait.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. The build is already in `DMM\mods\`. In DMM, turn `ProperHorseControls` off and on again and apply.
3. In `bin64\ProperHorseControls.ini`, set `Logging=1` and check `TapBrake=1`.

## Steps

On the bear and the wolf, then the horse as a control, with KBM. Use open flat ground.

1. **Taps after cruising:** hold forward at the highest gait for at least 3 seconds, then tap the brake key once. Wait 3
   seconds in the new gait, tap again, and carry on to the lowest gait.
2. **S taps:** repeat step 1 with S taps while W is held.
3. **Brake key hold:** from the highest gait with forward held, hold the brake key until the mount stops.
4. **S hold:** from the highest gait with W held, hold S until the mount stops.
5. **Horse:** do steps 1 and 3 on the horse. Expect the same as v1.4.0.

Close the game and copy the log and CSV to `scratch/probe-runs/stage49/`.

## Logs

- **Taps:** each `brake key press` or S tap is followed by one `step down start` and its `step down end`, with no
  `source=hold` line.
- **Holds:** one `source=hold` step down per gait, each at least 500 ms after the press, then `brake gap start ...
  source=no step` and `brake gap end ... reason=stopped`. No `step down start` on the same tick as `brake gap start`.
- **Lines:** no two log lines written onto one line with the same timestamp around brake events.

## Reading the results

| Outcome | Meaning |
| --- | --- |
| Taps drop one gait each, and holds step down to a stop | The fix |
| A tap is followed by a `source=hold` line | The tap was held 500 ms or more, or `kHoldBrakeMs` is too short |
| A hold pauses at a gait before the next `source=hold` | The 500 ms wait is too long for that chart |
| No brake events at all | The mount's input didn't reach `RunBrakeGap` |

Tester notes:

observed an interesting and nuanced emergent behaviour from the new hold brake. if you hold long enough to trigger the hold and then let go, mount will drop two gaits and not completely stop. I will not class this as a bug, but a nice additional feature we added by accident :D

## Results

Passed on 2026-11-16, on `B0100005`, `B0100007` and the horse `B0100002`. Logs are in `scratch/probe-runs/stage49/`.

- **Taps:** every brake key and S tap dropped one gait, with no `source=hold` line, including taps in gaits held for
  several seconds, where the step down ended 15 to 31 ms after the press.
- **Holds:** the first `source=hold` step down came 469 to 500 ms after the press, then one per gait, then `no step`,
  the slowdown and `reason=stopped` or `next state`. No `step down start` on the same tick as `brake gap start`.
- **Partial holds:** a hold released after the first `source=hold` step down drops two or three gaits and carries on
  at the lower one (lines 315 to 324, 333 to 339, 342 to 348). The tester keeps this as a feature.
- **Threads:** no interleaved log lines.
- **Horse:** `brake gap start` only, never `step down start`, as in v1.4.0.
