# Stage 25 test

In [stage 24](stage-24.md), 7 of 22 brake key taps on the horse from 5.2 to 12 kept their gait, because the chart left
the slowing state and came back, and that counted as the second change into one. The slowdown now needs two consecutive
changes into a state whose `mount_value` is 0 or missing. Horse only. Drift passed in stage 24 and isn't retested.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. Close the game. In DMM, turn `ProperHorseControls` off and on again and apply.
3. Set `Logging=1` in `bin64\ProperHorseControls.ini`.
4. Any terrain will do. Settling means holding W at a steady speed for about 2 seconds, with no A or D.

## Steps

1. **Brake key taps:** at each of 12 and 9.5, use Shift to reach the gait, release Shift, settle, tap Alt as briefly as
   you can, and keep W held for 4 seconds. Do about 10 taps per gait, returning to the gait between taps.
2. **Trot:** the same at 5.2, about 5 taps.
3. **S taps:** at 9.5, tap S with W held, 3 times. Each should still drop one gait.

Close the game and copy the log, CSV and ini to `scratch/probe-runs/stage25/`.

## Logs

- `brake gap start` with `source=brake key`, then `brake gap end` with `after=` around 300 ms for a tap that bounces. An
  end at about 140 ms followed by the old gait means the fix didn't work.
- `state` lines between the start and end show any bounce, such as `C5AF`, `C2B2`, `C5AF`.
- CSV: `mount_value` for the gait reached.

## Stage 25 results

| Step | Result |
| --- | --- |
| 1. Brake key taps at 12 | |
| 1. Brake key taps at 9.5 | |
| 2. Trot | |
| 3. S taps | |

- Any crash or freeze?
- Notes: idk what 12 and 9.5 are so I just tapped at all the different speeds, some in sequence, some repeated up and down. seemed ok.
