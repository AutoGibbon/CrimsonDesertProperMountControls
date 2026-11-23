# Stage 19 test

Three settings changes before release:

- Shift+F5 no longer flips the forward direction. F5 only toggles the mod.
- `Toggle` under `[Keys]` in `ProperHorseControls.ini` sets the toggle key.
- `InvertSteer` under `[Reverse]` swaps which way A and D curve the backward walk and the wagon's brake reverse. Forward
  steering and the S with A or D pivot from a stop don't change.

This is the first test with the files named `ProperHorseControls.*`.

## Setup

1. Copy `bin64\HorseSteer.log` and `bin64\HorseSteer.csv` to `scratch/probe-runs/` if they're still there.
2. Close the game. In DMM, turn `HorseSteer` off and apply, so that `bin64\HorseSteer.asi` is gone and only one copy of
   the mod loads. Then turn `ProperHorseControls` on and apply.
3. Check that `bin64\ProperHorseControls.ini` has `Logging=1`, `InvertSteer=0` and `Toggle=F5`.

## Steps

### Run A: `InvertSteer=0`, `Toggle=F5`

Launch and wait until at least 25 seconds after launch.

1. **Horse:** walk backward with S, then hold S with A for 2 seconds, then S with D. Note which way the rear swings for A.
2. **Horse:** from a stop, press S with A: it should pivot as before.
3. **Horse:** W with A and with D: forward steering as before.
4. **Shift+F5:** the controls should switch off with the low beep, as F5 does. Press F5 to switch them back on.
5. **Another mount** (any): repeat step 1.
6. **Wagon:** reverse with S, then S with A and S with D. Note which way it turns for A.

Close the game. Copy `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/stage19/a/`.

### Run B: `InvertSteer=1`, `Toggle=Insert`

In `bin64\ProperHorseControls.ini`, set `InvertSteer=1` and `Toggle=Insert`. Launch and wait 25 seconds.

1. Repeat steps 1 to 3, 5 and 6 of run A. Steps 1, 5 and 6 should curve the opposite way to run A, and steps 2 and 3
   should be unchanged.
2. **F5** should do nothing. **Insert** should toggle the controls with the beeps.

Close the game. Copy the log and CSV to `scratch/probe-runs/stage19/b/`.

### Run C: unaccepted key

Set `Toggle=W` and launch. Set `InvertSteer=0` and `Toggle=F5` again afterward.

1. **F5** should toggle the controls.

Close the game. Copy the log to `scratch/probe-runs/stage19/c/`.

## Logs

- The `ini:` line now ends with `invert` (0 or 1) and `toggle key` (the name in use). Run A shows `invert 0, toggle key
F5`, run B `invert 1, toggle key Insert`, and run C `toggle key F5`.
- Run C's log has `ini: Toggle=W isn't an accepted key, so F5 toggles the mod`.
- The CSV has no `flip` column.

## Stage 19 results

| Run | 1. Horse reverse | 2. Pivot | 3. Forward | 4. Shift+F5 / toggle key | 5. Other mount | 6. Wagon |
| --- | ---------------- | -------- | ---------- | ------------------------ | -------------- | -------- |
| A   | y                | y        | y          | y                        | y (wagon)      | y        |
| B   | y                | y        | y          | y                        | y (wagon)      | y        |
| C   |                  |          |            | y                        |                |          |

- Mount used in step 5: the wagon. No other walking mount was ridden, but they share the horse's backward walk branch.
- Any crash or freeze? no
- Notes: the tester prefers `InvertSteer=1`.

Logs are in `scratch/probe-runs/stage19/`. The `ini:` lines match each run, and run C logged the refused `Toggle=W`.
Summing `mount_phys_yaw` changes over CSV rows with S held and `mount_vel` above 0.3 m/s:

| Run | Horse, S+A | Horse, S+D | Wagon, S+A | Wagon, S+D |
| --- | ---------- | ---------- | ---------- | ---------- |
| A   | −117°      | +182°      | +317°      | −367°      |
| B   | +310°      | −375°      | −170°      | +119°      |

Every reverse turn swapped direction with `InvertSteer=1`.
