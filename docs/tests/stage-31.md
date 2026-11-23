# Stage 31 test

`CameraDetached` and `TapBrake` under `[Features]` turn off the mount-relative controls and the slowdown separately.
Stage 31 tests the three new combinations. Both on is the behaviour stages 29 and 30 passed.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. Close the game. In DMM, turn `ProperHorseControls` off and on again and apply.
3. Set `Logging=1` in `bin64\ProperHorseControls.ini`. Launching adds `[Features]` to it.
4. Use the horse on flat, open ground, with a keyboard and a gamepad.

Run all three parts in one launch. Before each part, set the two flags in `bin64\ProperHorseControls.ini` and press End.

## Part 1: `CameraDetached=1`, `TapBrake=0`

1. **Controls:** ride with W, A and D while looking around. The horse should ignore the camera, as usual.
2. **Reverse:** S alone and the stick pulled back from standing should walk backward, and S with A or D should pivot.
3. **S tap:** at a gallop with W held, tap S a few times. There should be no gait drop beyond what letting go of W for
   that moment gives.
4. **Stick back:** at a gallop, pull the stick back briefly a few times. The horse should only slow while it's held.
5. **Brake key:** at a gallop with W held, tap the brake key a few times. It should do what the game does by itself.

## Part 2: `CameraDetached=0`, `TapBrake=1`

1. **Controls:** ride while looking around. The horse should go where the camera points, as in the game without the mod.
2. **S alone:** S and the stick pulled back should turn the horse toward the camera, as in the game.
3. **S tap:** at a gallop with W held and the camera behind the horse, tap S a few times. The horse should drop a gait
   each time.
4. **Brake key:** at a gallop with W held, tap the brake key a few times. The horse should drop a gait each time.
5. **Camera to the side:** ride toward the camera's side so that the input is A or D, or the stick sideways. Tap the brake
   key a few times. The horse should drop a gait each time.
6. **Toward the camera:** ride toward the camera with S or the stick back. Tap the brake key a few times. The horse should
   drop a gait each time.

## Part 3: `CameraDetached=0`, `TapBrake=0`

1. **Everything:** ride, tap S with W held, and tap the brake key. It should all feel like the game without the mod.

Close the game and copy the log, CSV and ini to `scratch/probe-runs/stage31/`.

## Logs

- Each `reloading settings` line starts a part, and the `ini` line after it shows that part's `camera detached` and
  `tap brake`.
- Parts 2 and 3: `i1_applied` is 0 in the CSV, and the log has no `backward` or `brake reverse` lines.
- Parts 1 and 3: no `brake gap start` lines.
- Part 2: `brake gap start` has `source=S` or `source=brake key`, never `source=stick`, and `brake gap end` never has
  `reason=steering`.

## Stage 31 results

Passed on the horse, with the tester reporting good play. Logs are in `scratch/probe-runs/stage31/`.

- **Part 1:** `i1_applied` was 1 on 1738 of 1739 rows, with no `brake gap` lines. The 3 S taps with W held kept
  `mount_value` at 12.0.
- **Part 2:** `i1_applied` was 0 on every row. 23 slowdowns, 3 `source=S` and 20 `source=brake key`, none `source=stick`, all
  ending on `reason=next state`. 5 started with D held and no W, 4 with the stick sideways, and 5 with the stick pulled
  back. 22 dropped one gait.
- **Part 3:** `i1_applied` was 0 on every row, with no `brake gap` lines.
- **Reload:** both presses applied mid ride.
- **One tap dropped two gaits:** tick 3755406, from 12.0 to 5.2, with the chart going `D85C`, `C2B2`, `C5AF`, `B7CC`. It
  started at 7.72 m/s, the lowest of the 7 taps from 12.0, and the tester suspects a collision.
- **Not shown by the logs:** the brake key with `TapBrake=0`, as `brake key press` is only logged inside `UpdateBrakeGap`.
  Part 3 had no S taps with W held.
