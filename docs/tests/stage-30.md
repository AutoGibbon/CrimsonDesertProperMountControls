# Stage 30 test

`StickDeadzone` under `[Reverse]` makes an analog stick count as straight back within 0.30 of the angle from straight back
to straight sideways, about 27°, so that a stick slightly off center walks the mount backward instead of spinning it.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. Close the game. In DMM, turn `ProperHorseControls` off and on again and apply.
3. Set `Logging=1` in `bin64\ProperHorseControls.ini`. Launching adds `StickDeadzone=0.30` to it.
4. Use the horse on flat, open ground, with a gamepad.

## Steps

1. **Straight back:** from standing, pull the stick fully back for about 5 seconds, without trying to keep it perfectly
   centered. Do this 3 times. The horse should walk backward in a straight line.
2. **Slow pull:** from standing, ease the stick about a third of the way back and hold for 3 seconds. The horse should
   stay still.
3. **Reverse steering:** while walking backward, lean the stick well to one side, then the other. The horse should turn
   each way once the lean passes about a quarter of the way to the side, and go straight again when it comes back.
4. **Pivot:** from standing, push the stick to the back-left diagonal, then the back-right. The horse should pivot.
5. **Keyboard:** from standing, hold S for 3 seconds, then S with A, then S with D. S should walk backward, and S with A
   or D should pivot, as before.
6. **Braking:** at a gallop, pull the stick back briefly, a few times. The horse should drop a gait each time.
7. **Wagon:** pull the stick fully back from standing for about 5 seconds. The wagon should reverse straight.

Close the game and copy the log, CSV and ini to `scratch/probe-runs/stage30/`.

## Logs

- The `ini` line shows `deadzone 0.30`.
- During step 1, `raw_stick_y` near -1 with a small `raw_stick_x`, `i1_stick_x` of 0, and `mount_phys_yaw` steady, with
  `mount_vel` about 1 m/s. A yaw change of about 64°/s means x is still reaching the game.
- During step 2, `mount_phys_yaw` and `i1_stick_x` steady at 0.
- During step 6, `brake gap start` with `source=stick`.

## Stage 30 results

Passed on the horse, with the tester reporting all good and no crash or freeze. Step 7 wasn't run, as no wagon could be
found. Logs are in `scratch/probe-runs/stage30/`, with `deadzone 0.30` on the `ini` line.

- **Straight back:** 3 pulls of 6 to 10 s with `raw_stick_x` up to -0.06 had no yaw change, at about 1 m/s in `A73F`.
  A later 4 s pull with `raw_stick_x` of -0.15 to -0.03 also had none.
- **Slow pull:** `raw_stick_y` of -0.07 to -0.40 from standing stayed at 0 m/s with no yaw change.
- **Reverse steering:** logged on the stick to one side only. At tick 2128421, `raw_stick_x` of about 0.55 to 0.65,
  34° off back, turned the horse about 60°/s at about 1 m/s. That walk began by easing in from a pivot, and its state
  stayed in `99F1` and `9884`, never reaching `A37D` or `A73F`.
- **Pivot:** back diagonals on the stick, 33° to 40° off back, turned at -48°/s and 48°/s at 0 m/s.
- **Keyboard:** S alone had no yaw change at about 1 m/s. S with D turned at 54°/s and S with A at -54°/s.
- **Braking:** 5 `source=stick` taps at up to 14° off back each dropped one gait, with `mount_value` going from 12.0 to
  9.5 four times and from 9.5 to 5.2 once.
