# HorseProbe test

`HorseProbe.asi` logs riding data and finds the WASD input value in memory. It does not change game memory or controls.

Record your observations in [horseprobe-results.md](horseprobe-results.md) as you go.

## Setup

1. In DMM, enable `HorseProbe` and apply. Leave your other mods enabled.
2. Launch the game and load your save.
3. Mount your horse on flat, open ground, away from quests and enemies.
4. Wait until at least 20 seconds after launch. The probe ignores F8 and F10 until then.

## Part 1: heading log

Do each step for about 10 seconds, in order:

1. Hold W with the mouse still.
2. Keep holding W and swing the camera left and right.
3. Release W and stop. Rotate the camera without pressing any keys.
4. Hold A from a standstill, then release and stop.
5. Hold D from a standstill, then release and stop.
6. Hold S from a standstill, then release.

## Part 2: input scan (F8)

Keep your hand off the mouse, because moving the camera can make the scan discard the right value.

1. Press **F8**. It beeps once.
2. Hold **W** until it beeps, then release. The game may hitch here, and the beep may take several seconds.
3. Hold **D** until it beeps, then release.
4. Hold **S** until it beeps, then release.
5. Hold **A** until it beeps, then release.
6. Touch nothing until you hear three rising beeps.

Hold only one movement key at a time, because the probe ignores the step until exactly the right key is held. If it goes wrong, press F8 to start again.

## Part 3: write watch (F10)

This step could crash the game, so do it last and don't save afterwards. A crash still gives useful information.

1. Press **F10**. It beeps once.
2. Tap W, A, S and D until you hear two quick beeps, about 5 seconds later.
3. Repeat up to three more times. Each press checks the next four scan results.

Skip this part if Part 2 did not finish.

If the game crashes or the results look wrong, don't disable mods yet. Report it first.

## Afterwards

1. Close the game. This makes sure the logs are fully written.
2. In DMM, disable `HorseProbe` and apply.
3. Fill in the results file.

The probe writes `HorseProbe.log` and `HorseProbe.csv` to the game's `bin64` folder, and I read them from there. The next launch with the probe enabled overwrites both files, so tell me before running it again.
