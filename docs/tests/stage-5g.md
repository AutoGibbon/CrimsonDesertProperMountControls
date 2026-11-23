# Stage 5g test

This restores the backward loop's original data, so the rider's motion should be back, and slows the backward movement to about 1 m/s. You can
change the speed in game. S with A or D now moves the horse backward as well. The clips still only start after a stop from a walk, and S alone
from standing still slides the horse in its idle pose.

Keys, while the game is focused:

- **Numpad +:** faster, with a high beep.
- **Numpad -:** slower, with a low beep.

## Setup

1. Close the game. In DMM, turn `HorseSteer` and `HorseSteer Backward Walk Test` off and on again and apply, so both new versions are used.
2. Save, then load and wait until at least 20 seconds after launch. Stay on flat, open ground, with some clear space behind the horse.

## Steps

1. **Hold W at a walk, release it, and once the first backward clip starts, hold S until the clips end:** press numpad + or - between attempts,
   until the horse's feet best match the ground. Note how many presses from the start, or the setting in the log will show it.
2. **On the second clip:** note whether the rider's motion is back.
3. **From standing, hold S with A, then S with D, for about 5 seconds each:** note the path the horse takes, and how it looks.
4. **During the clips, hold S with A or D:** note whether the horse turns at all.
5. **Ride off with W:** the horse should set off normally.

Close the game and leave the logs in place until they've been read.

## Stage 5g results

| Step | What happened |
| --- | --- |
| Load | |
| 1. Best speed, and how the feet look | initial speed was near perfect match for clip 2. |
| 2. Rider on the second clip | rider looks passable |
| 3. S with A or D from standing | it does the pivot we've seen before, this time with a little backwards movement while it pivots. doesn't look very natural though |
| 4. S with A or D during the clips | no turn during this test |
| 5. Riding off | works fine |

- Any crash or freeze? no
- Does this look good enough to build the S trigger for? clip 2 looked great but see note!
- Notes: SOMETHING UNEXPECTED HAPPENED! when pressing W, if release and immediately hold S before horse settles to idle state, it actually activated a backwards walking animation all on its own?????
