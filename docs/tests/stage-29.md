# Stage 29 test

A slowdown now also ends once the mount is below 0.3 m/s, so that W works again without waiting for the 3 second cap.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. Close the game. In DMM, turn `ProperHorseControls` off and on again and apply.
3. Set `Logging=1` in `bin64\ProperHorseControls.ini`.
4. Use the bear on flat, open ground.

## Steps

1. **Lowest gait:** hold W from standing without Shift for about 5 seconds, tap S, and keep W held for 5 seconds. Do this
   3 times. The bear should slow nearly to a stop, then get going again by itself.

Close the game and copy the log, CSV and ini to `scratch/probe-runs/stage29/`.

## Logs

- `brake gap end` with `reason=stopped` or `reason=next state`, at under about 2 seconds. `reason=time cap` means the
  check didn't catch it.

## Stage 29 results

| Step | Result |
| --- | --- |
| 1. Lowest gait | GOOD |

- Any crash or freeze?
- Notes:
  - Logs are in `scratch/probe-runs/stage29/`, on the bear. No tap ended on `time cap`, in 8 taps.
  - Lowest gait, 1 tap: 1.8 in 1.63 s on `reason=next state`, then 1 m/s within 0.3 s with W held.
  - 5.2, 3 taps: landed on 1.8. 2 ended on `reason=stopped` at 0.29 and 0.30 m/s in 2.02 and 2.08 s, and 1 on
    `reason=next state` in 2.13 s, which is about what the taps took before.
  - 9.2, 4 taps: 2 landed on 5.2 in about 0.86 s, and 2 ended on `reason=steering` and kept 9.2.
  - One `brake gap end` was logged twice on the same line, with the same tick.
