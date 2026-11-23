# Stage 9 test

The mod now gets the horse's speed from the game's own velocity instead of working it out from position. Forward steering and the backward walk both use that speed, so the aim is to check they feel at least as good as in stage 6.
The backward walk may start slightly later in a stop, because velocity falls more slowly than position speed did.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply.
2. Load your save and wait until at least 20 seconds after launch.

## Steps

1. **Steer with A and D at a walk, trot and gallop:** turns should be as tight as before, tightest at a walk and wider at a gallop.
2. **From standing, hold S for about 5 seconds, then release, 5 times:** the horse should walk backward every time, then return to idle.
3. **At a walk, trot and gallop, release W and hold S at once, 3 times each:** the horse should stop, then walk backward. Note whether the start is smoother, the same or worse, and whether it ever fails.
4. **While walking backward, release S and press it again straight away, 3 times:** the horse should carry on or restart backward, with no forward step or stall.
5. **During a stop from a trot, tap S, release it, then hold it:** the walk should still start once the horse has stopped.
6. **While walking backward, hold A, then D:** the horse should curve as in stage 6.
7. **From standing, hold S with A or D:** the horse should pivot.
8. **While riding at speed, tap S with A or D to make a tight turn:** this should feel the same as before.

Close the game and leave the logs in place until they've been read.

## Stage 9 results

| Step | What happened |
| --- | --- |
| Load | |
| 1. Forward steering | ok |
| 2. S from standing | ok |
| 3. From a walk, trot and gallop | ok |
| 4. Re-press S while walking backward | ok |
| 5. Tap S during a stop | ok |
| 6. Steering in reverse | steering in reverse works, but releasing no longer recovers into the backward walk animation, just slides backward |
| 7. Pivot from standing | ok |
| 8. S with A or D at speed | ok |

- Any crash or freeze? no
- Notes: reverse steer recovery into reverse animation no longer reliable like before. hope log data shows why/
