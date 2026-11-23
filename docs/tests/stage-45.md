# Stage 45 test

S taps on mounts other than the horse. In [stage 44](stage-44.md), W and S together read as no move input, so after an
S tap's step down the chart went into its shared slowing node and often came back up a gait. Now, from a step down
started by S or a stick pulled back, the mount reads `T10` as not holding and `TE` as holding until S or the stick is
released, or for at most 500 ms after the step, so that holding S still slows the mount. Alt taps are unchanged.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. The build is already in `DMM\mods\`. In DMM, turn `ProperHorseControls` off and on again and apply.
3. In `bin64\ProperHorseControls.ini`, set `Logging=1`, `CameraDetached=1` and `TapBrake=1`.
4. Load save slot 1. Use the bear, camel, wolf, battle boar and cucu bird, then the horse.

## Steps

Holding W keeps the current gait, and each Shift tap raises it one gait.

On each mount except the horse, with the keyboard:

1. **S taps:** tap Shift up to the highest gait. Tap S with W held, wait about 2 s, and repeat until the mount stops.
2. **S hold:** tap Shift up to the highest gait, then hold S with W for 2 s. The mount should keep slowing.
3. **Alt taps:** repeat step 1 with Alt, as a check.

With a gamepad, if one is to hand, on one mount:

4. **Stick back:** pull the stick back briefly at each gait, as in step 1.

On the horse, repeat steps 1 and 2 as a check.

Close the game and copy the log and CSV to `scratch/probe-runs/stage45/`.

## Logs

- **Step down:** as in stage 44.
- **Hold:** `hold input end ... reason=released` when S or the stick is released, or `reason=held` 500 ms after the
  step.
- **CSV:** `mount_value` after each tap should be one gait lower and stay there.

## Reading the results

| Outcome | Meaning |
| --- | --- |
| Each S tap lands one gait lower and stays, and the S hold keeps slowing | The fix |
| S taps still come back up | Something else reads the zeroed stick. Compare the states after `hold input end` |
| The S hold stops at one gait | The 500 ms cap is too long or holding doesn't slow these mounts |


tester notes:

horse - good
bear -  good
camel - good
wolf -  good
boar -  good
cucu -  good

extra tests: wolf on gamepad with complex inputs (real riding and braking) - seems good!
wagon - still good

## Results

Passed on 2026-11-14 with the horse, bear, camel, elite wolf (keyboard, then gamepad), battle boar, cucu bird and
wagon. Logs are in `scratch/probe-runs/stage45/`.

- **Step downs:** 63, of which 59 ended `reason=gait changed` and 4 `reason=no step` at a lowest gait, each followed by
  the slowdown.
- **Holds:** 35 ended `reason=released` and 7 `reason=held`, the S holds.
- **S taps on non-horse mounts:** of 24 taps, 21 were one gait lower 2 s later. The other 3 were at the lowest gait,
  where the slowdown stopped the mount and it walked on. None came back up. All 7 S holds kept slowing.
- **No problems logged.**