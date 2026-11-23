# Stage 35 test

Below `kStandingSpeed` (0.2 m/s), keyboard steering ramps in over at least `kStandingRampMs` (100 ms), whatever
`KeyRampMs` is. This checks that `KeyRampMs=0` no longer sticks the horse turning on the spot when W and A or D are
pressed together from standing, while steering at speed is still instant.

## Background

- `KeyRampMs` under `[Steering]` sets how long W with A or D takes to reach the full steering angle that
  `TightenSteering` gives: 80° at 2.5 m/s and below, blending to 55° at 5 m/s and above, against the game's own 45°. The
  ramp scales the whole angle from straight ahead, and restarts when A or D is pressed or the side changes.
- In [stage 34](stage-34.md), `KeyRampMs=0` stuck the horse in chart state `9884`, turning on the spot at up to about
  1 m/s until A or D was released, on 2 of 3 standing W+A starts. At 50 ms, 1 of 7 starts with W and A in the same CSV row
  did. The 300 ms default passed 7 of 7 in stage 4. Clean starts go from standing to `A37D` and then `A56E`, and taking the
  full 80° once in `A37D` was fine.
- Pressing A or D clearly before W can still give the pivot, because A alone at a standstill is the game's own turn on
  the spot, and the ramp starts counting at the A press. The tester accepts this, and it's not what this test checks.
- 100 ms was chosen between the failing 50 and the passing 300, untested. If it fails, raise `kStandingRampMs` to 300.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. Copy `build/HorseSteer/Release/ProperHorseControls.asi` to `DMM\mods\`. In DMM, turn `ProperHorseControls` off and on
   again and apply.
3. In `bin64\ProperHorseControls.ini`, set `Logging=1` and `KeyRampMs=0`.
4. Use the horse on flat, open ground.

## Steps

1. **Standing starts:** stop fully, let go of every key, then press W and A at the same moment. Hold for about 3 s,
   release and stop. Do this at least 10 times, half with D instead of A. Note any start that turns on the spot.
2. **Gallop:** at a gallop with W held, switch between A and D several times. Steering should change direction at once.
3. **Optional, another mount:** repeat step 1 five times on a non-horse mount.

Close the game and copy the log and CSV to `scratch/probe-runs/stage35/`.

## Logs

- The `ini` line should read `key ramp 0ms`.
- `scratch/analysis/stick_table.py <csv>`, run with `scratch/venv/Scripts/python`, prints one row per CSV row: tick, keys
  (`WASD`), speed, yaw rate in °/s, the low 16 bits of `mount_state`, the raw stick and the stick after the mod
  (`i1_stick_x`, `i1_stick_y`). CSV rows are about 110 to 220 ms apart, so W and A in the same row can be up to that far
  apart in either order.
- **Pass:** no standing start whose first row has both W and A or D goes into `9884`. A `state 9884` line in the log
  marks each entry.
- **Standing ramp working:** the first row of a standing start shows a partial angle after the mod, such as
  (−0.41, 0.91), rather than the full (∓0.98, 0.17).
- **Gallop:** the stick after the mod reaches the full 55°, (±0.82, 0.57), within one row of each A or D press. Stage 34
  at 0 ms also had press rows showing (0, 1) with the raw stick already steering, so one such row isn't a failure.

## Stage 35 results

Passed on the horse. Logs are in `scratch/probe-runs/stage35/`. The `raw_stick` columns show which key the game read
first, which the `WASD` columns can't within a row.

| Start | Starts | Into `9884` |
| --- | --- | --- |
| W and A or D in the first row, raw stick y = 1 | 11 | 0 |
| A or D pressed just before W on purpose | 3, at ticks 1875156, 1910265 and 1957625 | 3 |
| W first, or W alone | 3 | 0 |

- **The pivots:** the tester pressed A or D first on purpose for the last W+A start, the third W+D start, and the last
  or second-to-last W+D start. In each, the first row shows a raw stick of (±1, 0) with `9884` already entered, so the
  side key alone reached the game first. Two of them show W and D in the same row's keys. This is the accepted case.
- **Against stage 34:** stage 34's 0 ms pivots at 5090484 and 5106890 had a raw stick of (−1, 1) on the first row, so W
  had reached the game and the horse still pivoted. Here 0 of 11 such starts did.
- **Standing ramp working:** first rows show partial angles such as (−0.42, 0.91), (0.43, 0.90) and (−0.77, 0.64), with
  the full (∓0.98, 0.17) on the next row.
- **Gallop:** on 9 of 10 switches with W held, the full 55°, (±0.82, 0.57), showed on the first or second row after the
  press. The yaw rate reached about 60°/s within 2 rows and changed sign within about 200 ms. At 2095656, the first row
  still showed the old side. A or D alone turned at about 98°/s. The run ended in a collision with an NPC before a full
  circle on A or D alone.
- **Other mount (step 3):** not run.
