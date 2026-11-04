# Stage 3 test

This build limits the mod to land mounts by their action package, tightens W+A and W+D turns when setting off, and slightly
increases them in motion. The steering numbers are a starting point, so report how they feel.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply, so the new build is copied to `bin64`.
2. Load your save and wait until at least 20 seconds after launch.

## Steps

1. **Mount your horse and hold W:** the horse should go straight. If the game's normal controls are back, stop and report it,
   as the mount check has failed.
2. **From standing, hold W and A, then W and D:** the turn should be much tighter than before.
3. **At a walk, trot and gallop, hold W and tap or hold A or D:** the turn should be slightly quicker than before. Check that it
   doesn't feel twitchy, especially when moving from walk to trot, where the steering angle changes.
4. **Hold W and swing the camera:** the horse should still go straight.
5. **From standing, hold S and A, then S and D:** the pivot should be unchanged.
6. **Any other mount, if you get access to one:** ride it briefly and note the name and how it behaves.

Leave `HorseSteer` enabled until the logs have been read.

## Stage 3 results

| Step | What happened |
| --- | --- |
| 1. W on horse | good |
| 2. W with A or D from standing | good, though if pressing W and steer at almost exactly the same time the horse gets stuck in a pivot without moving. |
| 3. W with A or D in motion | seems better, not too fast not too slow. though, it is noticably "instant" in application. if it could ramp up over 0.3s toward the maximum turn rate? assess for complexity cost before doing anything here |
| 4. W with camera swing | good |
| 5. S with A or D | good |
| 6. Other mounts | untested |

- Any crash or freeze?
- Too tight, too loose, or about right?
- Notes:
