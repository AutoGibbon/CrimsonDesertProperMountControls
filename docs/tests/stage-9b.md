# Stage 9b test

This build puts back the old way of working out the horse's speed, to find out whether the speed change caused the slide after steering in reverse. Nothing else changes. Only the backward walk needs testing.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply.
2. Load your save and wait until at least 20 seconds after launch.
3. Mount your horse on ground as flat as you can find.

## Steps

1. **From standing, hold S for about 5 seconds, then release:** the horse should walk backward with its walking animation, as a check before the next step.
2. **Reverse steering, 10 times, 5 with A and 5 with D:** from standing, hold S for 3 seconds, add A or D for 3 seconds, release A or D, and keep holding S for 5 more seconds. Then release S and wait until the horse has stopped. After each release, note whether the horse returned to its walking animation or slid.

Close the game and leave the logs in place until they've been read.

## Stage 9b results

| Step | What happened |
| --- | --- |
| Load | |
| 1. S from standing | ok |
| 2. Releases that returned to the walking animation, out of 10 | back to good performance |

- Any crash or freeze? no
- Notes: n/a
