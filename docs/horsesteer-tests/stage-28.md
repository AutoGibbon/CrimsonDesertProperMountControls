# Stage 28 test

In [stage 27](stage-27.md), a wagon tap from top speed slowed sharply, then jumped back up by 1.5 to 2 m/s when the
position write ended. The position write is now skipped on the wagon. Wagon only.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. Close the game. In DMM, turn `ProperHorseControls` off and on again and apply.
3. Set `Logging=1` in `bin64\ProperHorseControls.ini`.
4. Use a wagon on flat, open ground.

## Steps

Keep W held throughout, and for 5 seconds after each tap.

1. **Top:** hold Shift until the wagon stops gaining speed, release Shift, and tap at once. Do this 3 times with S and 3
   times with Alt. Note whether there's a lurch, and how quickly it slows.
2. **Cruise:** from top, release Shift and wait about 10 seconds, then tap S 2 times, returning to cruise between taps.

Close the game and copy the log, CSV and ini to `scratch/probe-runs/stage28/`.

## Logs

- `brake gap end` from top: `after=` around 1400 ms with `reason=next state`. Cruise: `reason=end speed`.
- CSV: `brake_assist` stays 0 on the wagon, and `mount_vel` doesn't rise after `brake_gap` returns to 0.

## Stage 28 results

| Step | Result |
| --- | --- |
| 1. Top: any lurch | no lurch |
| 1. Top: how quickly it slows | feels appropriate |
| 2. Cruise | same as before |

- Any crash or freeze? no
- Notes: good. tbh maybe just drop assist entirely? the horse animations appear to be affected by the position write and its noticable more times than it's not.
  - Logs are in `scratch/probe-runs/stage28/`.
  - Top, 7 taps from 11.6 to 11.9 m/s: 1.44 to 1.58 s on `reason=next state`, and speed kept falling after the tap ended.
  - Cruise, 2 Alt taps: `reason=end speed` in 0.44 and 0.52 s. `brake_assist` stayed 0 on the wagon.
  - The position write was removed for every mount after this stage, with no test of its own. A bear run on that build is
    in `scratch/probe-runs/bear-no-assist/`: 5.2 to 1.8 in 2.13 to 2.20 s, 3 of 3, and 9.2 to 5.2 in about 0.9 s, 2 of 2,
    all on `reason=next state`.
