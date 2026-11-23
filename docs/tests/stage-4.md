# Stage 4 test

This build eases steering in over 0.3 s instead of applying it at once, and should stop the horse getting stuck in a pivot when
W and A or D are pressed at the same moment.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply, so the new build is copied to `bin64`.
2. Load your save and wait until at least 20 seconds after launch.

## Steps

1. **From standing, press W and A at exactly the same moment, then W and D:** repeat about 5 times each. The horse should set off
   and turn, without getting stuck in a pivot.
2. **From standing, hold W, then add A or D a moment later:** this should set off as well as before.
3. **From standing, hold A or D, then add W a moment later:** this should set off as well as before.
4. **At a trot and gallop, hold W and hold A or D:** the turn should build up smoothly instead of starting at once, and reach the
   same turn speed as before.
5. **At a trot and gallop, hold W and tap A or D:** the tap will turn less than before. Note whether small corrections still feel
   useful.
6. **Hold W, then swap directly between A and D:** the horse should change direction without a jerk or a pause.
7. **From standing, hold S and A, then S and D:** the pivot should be unchanged.

Leave `HorseSteer` enabled until the logs have been read.

## Stage 4 results

| Step | What happened |
| --- | --- |
| 1. W with A or D, same moment | seems good |
| 2. W, then A or D | good |
| 3. A or D, then W | good |
| 4. Hold A or D in motion | good |
| 5. Tap A or D in motion | feels good to me |
| 6. Swap between A and D | good |
| 7. S with A or D | good |

- Any crash or freeze?
- Is 0.3 s too slow, too quick, or about right? seems juust about right.
- Notes:
