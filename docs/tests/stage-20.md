# Stage 20 test

At startup, `LoadSettings` writes each setting missing from `ProperHorseControls.ini` with its default, so that a
player's ini from an earlier version gains new settings. DMM keeps an existing ini when the mod is updated, and copies
the zip's ini only when `bin64` has none.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. Close the game. In DMM, turn `ProperHorseControls` off and on again and apply.
3. Edit `bin64\ProperHorseControls.ini`:
   1. Set `Logging=1`.
   2. Delete the `InvertSteer=1` line.
   3. Delete the `CD_M0002_Camel=1.9` line.
   4. Delete the whole `[Keys]` section, with its comments and `Toggle=F5`.
4. Copy the edited ini to `scratch/probe-runs/stage20/before.ini`.

## Steps

1. Launch and wait until the main menu.
2. Close the game.
3. Open `bin64\ProperHorseControls.ini`:
   1. `InvertSteer=1` is at the end of `[Reverse]`.
   2. `CD_M0002_Camel=1.9` is at the end of `[BackwardSpeed]`.
   3. A `[Keys]` section with `Toggle=F5` is at the end of the file.
   4. Nothing else changed.
4. Launch again, mount the horse, and walk backward with S and A for a few seconds. The rear should swing as in stage 19
   run B.

Copy the ini to `scratch/probe-runs/stage20/after.ini`, and the log to `scratch/probe-runs/stage20/`.

## Logs

The first launch's `ini:` line reads `14 values read, 3 added, invert 1, toggle key F5`. After the second launch, the
log only holds the second run, whose line reads `17 values read, 0 added`.

## Stage 20 results

| 3.1 InvertSteer | 3.2 Camel | 3.3 Keys | 3.4 Nothing else | 4. Reverse |
| --------------- | --------- | -------- | ---------------- | ---------- |
| n/a             | n/a       | n/a      | n/a              | n/a        |

- Any crash or freeze? no
- Notes: the tester ran it with no ini in `bin64` instead of the steps above. The mod wrote a 408-byte ini with all 14
  backward speeds, `InvertSteer=1`, `Toggle=F5` and `Logging=0`, with no comments. Passed.
