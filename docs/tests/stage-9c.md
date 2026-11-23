# Stage 9c test

This build goes back to the stage 9 way of working out the horse's speed, and repeats the stage 9b test on the same kind of ground. Stage 9b fixed the slide after steering in reverse, but it also moved to flat ground, so this test shows whether the speed change or the ground made the difference. The log now records the horse's height as well. Only the backward walk needs testing.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply.
2. Load your save and wait until at least 20 seconds after launch.
3. Mount your horse on ground as flat as you can find, in the same place as stage 9b if you can.

## Steps

1. **From standing, hold S for about 5 seconds, then release:** the horse should walk backward with its walking animation, as a check before the next step.
2. **Reverse steering, 10 times, 5 with A and 5 with D:** from standing, hold S for 3 seconds, add A or D for 3 seconds, release A or D, and keep holding S for 5 more seconds. Then release S and wait until the horse has stopped. After each release, note whether the horse returned to its walking animation or slid.

Close the game and leave the logs in place until they've been read.

## Stage 9c results

FLATTEST GROUND I COULD FIND TEST

| Step | What happened |
| --- | --- |
| Load | |
| 1. S from standing | ok |
| 2. Releases that returned to the walking animation, out of 10 | all ok |

- Same place as stage 9b? same location yes
- Any crash or freeze? no
- Notes:

UNEVEN GROUND TEST (after holding A+D for 5 seconds)

| Step | What happened |
| --- | --- |
| Load | |
| 1. S from standing | ok |
| 2. Releases that returned to the walking animation, out of 10 | ok, until going uphill, then less ok |

- Same place as stage 9b? different location
- Any crash or freeze? no
- Notes: looks like going uphill might be the cause. backward speed may not be enough when moving up hill to trigger the blend into backward walk state after steer.

bonus uneven tests. starting a reverse uphill never had a problem activating reverse walk state.