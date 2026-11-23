# Stage 23 test

The build intended for release. Tapping S with W held, or pulling the stick back, writes 0 to the raw stick until the
second change into a chart state whose `mount_value` is 0 or missing, on every ground mount but the wagon.
`AssistPerSecond` under `[Slowdown]` replaces the `[BrakeTest]` section and defaults to 0.4.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. Close the game. In DMM, turn `ProperHorseControls` off and on again and apply.
3. Set `Logging=1` in `bin64\ProperHorseControls.ini`. The `[BrakeTest]` section is ignored now, and can be deleted.
4. Use flat, open ground. Settling means holding W at a steady speed for about 2 seconds, with no A or D.

## Steps

Do these on the horse, then on the bear or camel.

1. **Tap:** at each gait, use Shift to reach it, release Shift, settle, tap S once, and keep W held for 5 seconds. Do this
   twice per gait, and note the gait it settles at and how the feet and body look.
2. **Lowest two gaits:** W should take over again after about 2 seconds, not 3.
3. **Hold to a stop:** at a gallop, hold W and S until the mount stops. Then release S with W still held, and note what it
   does.
4. **S alone:** at a gallop, release W and hold S. The mount should slow, stop, then walk backward.
5. **Steering:** at a gallop, hold W and press A or D at the same moment as S. Do the same with S first and A or D a
   moment later. There should be no slide or drift. From standing, S with A or D should still pivot.
6. **Wagon, if one is to hand:** S should still reverse it, with no slowdown behaviour.
7. **Gamepad, if one is to hand:** at a gallop, pull the stick back briefly and return it to forward.

Close the game and copy the log, CSV and ini to `scratch/probe-runs/stage23/`.

## Logs

- The `ini` line ends with `assist per second 0.40`, and the ini has gained `[Slowdown]` with `AssistPerSecond=0.4`.
- `brake gap start` and `brake gap end` lines, with the end reason `next state`, `steering` or `time cap`.
- CSV: `brake_gap` and `brake_assist` during taps, and `mount_value` and `mount_vel` afterwards.

## Stage 23 results

| Step | Horse | Bear or camel |
| --- | --- | --- |
| 1. Tap: gait reached | | |
| 1. Tap: feet and body | | |
| 2. Lowest two gaits | | |
| 3. Hold to a stop | | |
| 4. S alone | | |
| 5. Steering | | |
| 6. Wagon | | |
| 7. Gamepad | | |

- Any crash or freeze?
- Notes: all good, no point filling the table
  - Logs are in `scratch/probe-runs/stage23/`, with the bear, horse and wagon. No camel ride is in the log.
  - Bear, taps with W held and no Shift: 13.3 to 5.2 in 4 of 4 and 9.2 to 5.2 in 6 of 6. 5.2 to 1.8 in 5 of 5, ending in
    about 2.2 s on the changed condition, with one more on the 3 s cap. 1.8 stayed at 1.8, ending in about 1.6 s. The
    position write peaked at 3.5 m/s, from 15.5.
  - Horse, same taps: 12 to 9.5 in 4 of 4, 9.5 to 5.2 in 2 of 2, 5.2 to 2.8 in 3 of 3. 2.8 walked on after about 1.8 s. No
    taps at 14.
  - W and S held to a stop took 3.1 to 3.5 s on the bear and 3.6 to 4.6 s on the horse. W then resumed at the lowest gait.
  - S alone slowed the mount and reversed it. S with A or D kept the run gait.
  - Wagon: no slowdown started, and S reversed it.
