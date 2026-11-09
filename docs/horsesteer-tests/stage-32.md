# Stage 32 test

`Reload` under `[Keys]`, End by default, reads `ProperHorseControls.ini` again while the game runs.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. Close the game. In DMM, turn `ProperHorseControls` off and on again and apply.
3. Set `Logging=1` in `bin64\ProperHorseControls.ini`.
4. Use the horse on flat, open ground. Have the ini open in an editor, and save it before each press of End.

## Steps

1. **Speed:** set `CD_R0001_Horse=3.0` and press End while mounted. S from standing should walk backward about three
   times as fast. Set it back to `1.0` and press End.
2. **Invert:** set `InvertSteer=0` and press End. A and D should swap direction during the backward walk. Set it back to `1`
   and press End.
3. **Flag during reverse:** hold S to walk backward. While holding it, set `CameraDetached=0`, save and press End. The
   backward walk should stop, and the horse should turn toward the camera as in the game. Set it back to `1` and press End.
4. **Flag during a slowdown:** set `TapBrake=0`, save, then at a gallop tap S with W held and press End at the same moment.
   Nothing should stick. W should keep working and `brake_gap` should read 0. Set it back to `1` and press End.
5. **Bad key:** set `Reload=W` and press End. The log should report it, and End should still reload. Set it back to `End`.

Close the game and copy the log, CSV and ini to `scratch/probe-runs/stage32/`.

## Logs

- A `reloading settings` line, an `ini` line and a `mount package` line for each press while mounted. The `mount package`
  line has the new backward speed.
- `backward_rate` in the CSV changes with step 1.
- Step 5: `ini: Reload=W isn't an accepted key, so End is used`.

## Stage 32 results

Not run.
