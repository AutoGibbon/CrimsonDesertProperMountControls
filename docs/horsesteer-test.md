# HorseSteer stage 1 test

`HorseSteer.asi` makes riding input follow the mount's facing instead of the camera. This test checks whether that works
in game. It changes game memory while running, and has no effect once disabled.

Only forward steering is the target of this stage. A, D and S are expected to feel wrong, so describe what they do rather
than treating it as a failure.

Keys, while the game is focused:

- **F5:** toggle the mod. A high beep means on and a low beep means off. It starts on.
- **Shift+F5:** flip the forward direction, with one medium beep. Use this only if step 2 tells you to.

## Setup

1. In DMM, disable `HorseProbe`, enable `HorseSteer` and apply.
2. Launch, load your save, and wait until at least 20 seconds after launch, when the hook installs.
3. Stay on flat, open ground. Ride only your horse: the mod currently applies to anything you ride.

## Steps

Spend about 10 seconds on each. Write what happened in the Results section below.

1. **On foot:** walk around with WASD and move the camera. This should feel normal.
2. **Mount up and hold W, mouse still:** the horse should go straight ahead without turning. If it turns around or spins,
   press Shift+F5 once, try again, and note it in the results.
3. **Hold W and swing the camera left and right:** this is the main test. The horse should keep going straight.
4. **Press F5 (low beep), then repeat step 3:** the old behaviour should return, with the horse following the camera. Press
   F5 again (high beep) to turn the mod back on.
5. **Hold W and A, then W and D:** the horse should curve left, then right.
6. **Standing still, hold A, then D:** describe whether the horse turns in place or moves off.
7. **Standing still, hold S:** describe what it does.
8. **Dismount and walk around:** this should feel normal again.

## Afterwards

1. Close the game.
2. Fill in the results below.
3. Leave `HorseSteer` enabled until the logs have been read. Disabling it in DMM deletes `HorseSteer.log` and
   `HorseSteer.csv` from `bin64`.

## Results

| Step | What happened |
| --- | --- |
| 1. On foot | |
| 2. Hold W | |
| 3. Hold W, swing camera | |
| 4. Mod off, swing camera | |
| 5. W with A, W with D | |
| 6. A, then D, from standing | |
| 7. S from standing | |
| 8. After dismounting | |

- Did you need Shift+F5? (yes / no):
- Any crash or freeze? (which step, and what you were doing):
- Which way is "left" in step 5: the horse's left, or something else?
- Notes:
