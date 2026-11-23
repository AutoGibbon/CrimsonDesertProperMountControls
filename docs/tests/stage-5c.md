# Stage 5c test

This gives the horse's backward loop move speed data, to see whether that makes it move. It's the same test as stage 5b otherwise, and
S still does nothing from standing.

## Setup

1. Close the game. In DMM, turn `HorseSteer Backward Walk Test` off and on again and apply, so the new version of the patch is built.
2. Load your save and wait until at least 20 seconds after launch. Stay on flat, open ground, with some clear space behind the horse.

## Steps

1. **Hold W at a walk, then release it, and don't press anything:** the horse should stop, then play the backward clips. Repeat this
   about 5 times, keeping track of which clips play.
2. **During the second clip:** note whether the horse moves, which way, how far, and whether its feet match the ground.
3. **During the second clip:** note whether the rider still has the secondary motion you saw before.
4. **Stop with a wall or slope close behind the horse:** note whether it stops at the wall and follows the slope.
5. **After the clips:** check the horse returns to idle and rides off normally with W.

Close the game and leave the logs in place until they've been read. Then turn the test mod off in DMM and apply.

## Stage 5c results

| Step | What happened |
| --- | --- |
| Load | |
| 1. Which clips play | 3 clips played, I think it was just hard to see difference in prior 3 clips used |
| 2. Horse movement in the second clip | no movement |
| 3. Rider in the second clip | no apparent movement |
| 4. Wall and slope | untested since no movement |
| 5. Return to idle and riding off | fine |

- Any crash or freeze? none
- Notes:
