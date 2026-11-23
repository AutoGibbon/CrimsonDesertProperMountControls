# Stage 33 test

`PHC_SetSuspended` turns the mod off while any calling mod holds it, without changing the F5 toggle.

The caller is `scratch/phc-suspend-caller/build/Release/PhcSuspendCaller.asi`, which uses the lookup from the README. In
game, F9 toggles a hold by owner `TestA` and F10 a hold by owner `TestB`.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. Copy `build/HorseSteer/Release/ProperHorseControls.asi` to `DMM\mods\`. In DMM, turn `ProperHorseControls` off and on
   again and apply.
3. Set `Logging=1` in `bin64\ProperHorseControls.ini`.
4. Copy `PhcSuspendCaller.asi` to `bin64`.
5. Use the horse on flat, open ground. Check that F9 and F10 aren't bound by another mod.

## Steps

1. **Suspend:** ride with W held and press F9. The horse should follow the camera, as with F5 off.
2. **Resume:** press F9. The horse should ignore the camera again.
3. **Two owners:** press F9, then F10, then F9. The horse should still follow the camera. Press F10, and it should ignore
   the camera.
4. **Toggle kept:** press F5 to turn the mod off, then F9 twice. The horse should still follow the camera. Press F5, and
   it should ignore the camera.
5. **During the backward walk:** hold S until the horse walks backward, then press F9. The backward walk should stop with
   nothing stuck. Release S and press F9. S from standing should walk backward again.
6. **During a slowdown:** at a gallop with W held, tap S and press F9 at the same moment. W should keep working. Press F9
   and tap S again. The horse should drop a gait.

Close the game, copy the log and CSV to `scratch/probe-runs/stage33/`, and delete `bin64\PhcSuspendCaller.asi`.

## Logs

- Step 1: `suspended by TestA (1 holding)`. If this line never appears, the caller couldn't find
  `ProperHorseControls.asi` under that name.
- Step 2: `resumed by TestA (0 holding)`.
- Step 3: the holding count goes 1, 2, 1, 0.
- Step 6: `brake_gap` in the CSV returns to 0 after the F9 press.

## Stage 33 results

Passed on the horse, with the tester reporting that every step worked. Logs are in `scratch/probe-runs/stage33/`. The
`suspended by` lines show that the README's lookup finds the mod when DMM installs it.

- **All holds:** `i1_applied` was 0 on all 403 rows with the mod on and an owner holding, and back to 1 on the first or
  second row after each release.
- **Step 1:** with W and no A or D, `travel_yaw` went from -118.2 to -38.6, toward the camera at -35.7.
  `i0_camera_yaw` isn't updated during a hold, so it reads the last value before the hold.
- **Step 2:** the clearest evidence of the horse ignoring the camera after a release came later in the run, at tick 8873062 onward:
  `mount_phys_yaw` stayed at -78.79 while `i0_camera_yaw` moved from 97.6 to 144.1.
- **Step 3:** the holding count went 1, 2, 1, 0, and `i1_applied` stayed 0 for the 59 rows with only `TestB` holding.
- **Step 4:** `enabled` stayed 0 through both F9 presses, and `i1_applied` stayed 0 until `mod enabled`.
- **Step 5:** run with S held from tick 8818375 to 8830296, across both F9 presses, rather than released before the
  second one. The suspend moved the chart from `A73F` to `A56E` and the horse turned toward the camera. The resume returned
  to `A73F` about 120 ms later. A backward walk from standing after a resume wasn't run. One `backward lost pulse` fired
  4.4 s after the resume, and the walk recovered.
- **Step 6:** the first S tap started a gap 25 ms before F9. `brake_gap` was 1 at tick 8866718 and 0 at 8866828, with no
  `brake gap end` line, because `OnStickRead` clears `g_brakeGap` without logging when `RidingGroundMount` is false. With W
  held, `mount_value` stayed 12 at about 8.3 m/s for the whole hold. After the release, an S tap took `mount_value` from
  12 to 9.5, ending on `reason=next state` after 157 ms.
