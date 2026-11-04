# Stage 5h test

The backward walk in stage 5g may come from the game's own walking blend, driven by the mod moving the horse backward. This checks that without the
test chart. The `HorseSteer` build doesn't change.

## Setup

1. Close the game. In DMM, turn off `HorseSteer Backward Walk Test` and apply. Leave `HorseSteer` on.
2. Load your save and wait until at least 20 seconds after launch. Stay on flat, open ground, with some clear space behind the horse.

## Steps

1. **Hold W at a walk, release it, and hold S straight away, for about 10 seconds:** repeat 5 times. Note whether the backward walk plays, how long it
   lasts, and what the horse does when it ends.
2. **Repeat, waiting about 0.5, then 1, then 2 seconds between releasing W and pressing S:** note the longest wait that still gives the walk.
3. **While the backward walk plays, add A, then D:** note whether the horse's body or path changes.
4. **While it plays, press numpad + or - and try again:** note the setting where the feet best match the ground. The test starts at 0.50.
5. **From a trot, release W and hold S straight away:** note what plays.

Close the game and leave the logs in place until they've been read.

## Stage 5h results

| Step | What happened |
| --- | --- |
| Load | |
| 1. How long the walk lasts, and how it ends | looks like it can loop indefinitely once triggered and the backward speed is maintained |
| 2. Longest wait that still works | there seems to be a small window of opportunity to get the blend to activate, log data will show long backward walks these were the ones that had successful activations |
| 3. Adding A or D | branches into the known pivoting on the spot state |
| 4. Best speed, and how the feet and rider look | one back walk was the longest, and had the best matched speed |
| 5. From a trot | harder to reliably activate, seems to be highly dependent on timing in its current state |

- Any crash or freeze? one CTD while alt tabbing back to game between note writing
- Notes: good looking animation if it can be leveraged.
