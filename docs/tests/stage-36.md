# Stage 36 test

`TightenSteering`'s angles are now `[Steering]` settings: `LowSpeedBias` at `LowSpeed` m/s and below, blending to
`HighSpeedBias` at `HighSpeed` and above. A bias is W+A's angle as a share of 90°, so the multiplier on the stick angle is
2 × bias, capped at 85°. The defaults, 0.89 at 2.5 and 0.61 at 5.0, give the earlier 80° and 55° to within 0.1°. This
checks that the values are read, reloaded and applied.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. Copy `build/HorseSteer/Release/ProperHorseControls.asi` to `DMM\mods\`. In DMM, turn `ProperHorseControls` off and on
   again and apply.
3. In `bin64\ProperHorseControls.ini`, set `Logging=1`.
4. Use the horse on flat, open ground.

## Steps

1. **Defaults:** ride at a walk, trot and gallop with W and A or D held.
2. **Bias 0:** set `LowSpeedBias=0` and `HighSpeedBias=0`, press End, and repeat step 1. W with A or D should go straight.
3. **Bias 1:** set both to 1, press End, and repeat step 1. Steering should be tighter than the defaults.
4. **Speed bands:** set `LowSpeedBias=0.89` and `HighSpeedBias=0`, press End, and accelerate from a walk to a canter with
   W and A or D held. Steering should fade out between 2.5 and 5 m/s.
5. **Missing key:** with `HighSpeedBias=0` in use, delete the `HighSpeedBias` line and press End.

Close the game and copy the log and CSV to `scratch/probe-runs/stage36/`.

## Logs

- The `ini` line reads `steer bias <low> at <LowSpeed> m/s to <high> at <HighSpeed> m/s`, at launch and after each End.
- Turn rate: the change in `mount_phys_yaw` between CSV rows with `w` and `a` or `d` set, per second.
- **Missing key:** the `ini` line after the reload reads `1 added` and `to 0.61`, and the ini has `HighSpeedBias=0.61`
  again under `[Steering]`.

## Stage 36 results

Passed on the horse. Logs are in `scratch/probe-runs/stage36/`.

- **Ini:** the `ini` line read `0.89 at 2.5 m/s to 0.61 at 5.0 m/s` at launch, `0.00` to `0.00` after the reload at
  21:02:05 and `1.00` to `1.00` after the reload at 21:02:33. It reported `0 added`, because the `bin64` ini already had
  the keys, so adding missing keys wasn't tested.
- **Turn rate**, median °/s with W and A or D held:

  | Speed | Defaults | Bias 0 | Bias 1 |
  | --- | --- | --- | --- |
  | Below 2.5 m/s | 80 | 0 | 67 |
  | 2.5 to 5 m/s | 57 | not reached | 82 |
  | 5 m/s and above | 54 | 0 | 75 |

  The defaults' middle band falls between the two ends, so the blend applies. Bias 1's low band is mostly starts from
  standing, and is 85° against the defaults' 80°, so its lower median isn't read as a difference.
- **`9884` at 21:02:34 under bias 1:** D was pressed about 330 ms before W, the accepted case from
  [stage 35](stage-35.md). The horse left `9884` within about 300 ms and walked off.
- **Speed bands (step 4), logs in `scratch/probe-runs/stage36b/`:** the median turn rate was 81°/s at 2 to 3 m/s, 26 to
  28°/s at 3 to 5 m/s and 0°/s at 5 m/s and above.
- **Missing key (step 5), failed:** the reload at 21:08:45 logged `1 added` and wrote `HighSpeedBias=0.61` back under
  `[Steering]`, but the `ini` line still read `to 0.00`. `LoadSettings` wrote the default for a missing key without
  setting it, so a reload kept the earlier value, for every setting, not only the steering ones. It now sets the
  default as well.
- **Missing key re-run, passed, logs in `scratch/probe-runs/stage36c/`:** `HighSpeedBias` and `CD_R0001_Horse=6.0`
  deleted before one reload at 21:16:08 logged `2 added`, `to 0.61`, and the horse's backward speed 6.00 → 1.00. S held
  measured a median 6.01 m/s before the reload and 1.02 m/s after.
- **After the test:** the tester moved the default speeds to `LowSpeed=2.2` and `HighSpeed=4.8`.
- **Not run:** a `HighSpeed` below `LowSpeed`, a gamepad stick, and another mount.
