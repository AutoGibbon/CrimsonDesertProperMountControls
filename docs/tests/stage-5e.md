# Stage 5e test

This fixes a field in the patch that was left at zero in stages 5c and 5d, and which may be what makes the horse move. The clips run as before.

## Setup

1. Close the game. In DMM, turn `HorseSteer Backward Walk Test` off and on again and apply, so the new version of the patch is built. Leave
   `HorseSteer` on.
2. Load your save and wait until at least 20 seconds after launch. Stay on flat, open ground, with some clear space behind the horse.

Each attempt starts by holding W at a walk, then releasing it, and waiting for the horse to stop and start the first clip. Pressing a key before
then cancels the sequence.

## Steps

1. **No keys, mod on:** let the clips play. Repeat 3 times.
2. **Hold W through the clips, mod on:** after the first clip starts, hold W until the clips end. Repeat 3 times.
3. **Hold S through the clips, mod off:** press F5 (low beep) once the horse has stopped. After the first clip starts, hold S until the clips
   end. Press F5 again (high beep) before riding off. Repeat 3 times.
4. For each, note whether the horse moves during the second clip, which way, how far, and whether its feet match the ground.
5. **If it moves:** stop with a wall or slope close behind the horse, and note whether it stops at the wall and follows the slope.

Close the game and leave the logs in place until they've been read.

## Stage 5e results

| Step | What happened |
| --- | --- |
| Load | |
| 1. No keys, mod on | no move |
| 2. W through the clips, mod on | no move |
| 3. S through the clips, mod off | no move |
| 4. Feet and rider, if it moves | n/a |
| 5. Wall and slope, if it moves | n/a |

- Any crash or freeze? no
- Notes: n/a
