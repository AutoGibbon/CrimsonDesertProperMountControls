# Stage 9d test

If the horse leaves its walking animation during the backward walk while S is held, this build now gives it another short forward input to bring the animation back. The main thing to test is whether this works uphill, and whether you can feel the forward input as a lurch. Only the backward walk and stopping need testing.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply.
2. Load your save and wait until at least 20 seconds after launch.
3. Mount your horse on the same flat ground as stages 9b and 9c.

## Steps

### Flat ground

1. **From standing, hold S for about 5 seconds, then release:** the horse should walk backward with its walking animation.
2. **Reverse steering, 5 times:** from standing, hold S for 3 seconds, add A or D for 3 seconds, release A or D, and keep holding S for 5 more seconds. Then release S. The horse should return to its walking animation each time, with no lurch.

### Stopping

3. **Stop from a trot and a gallop, twice each:** release W and hold S at once. The horse should stop, then walk backward, with no extra forward step while it slows down.

Stop, hold A and D together for 5 seconds without W or S to mark the end of this section in the log, then ride to the steepest slope you can reverse up.

### Steep slope

4. **Straight uphill, at least 5 times:** hold S for 10 seconds or more, staying on the steepest part. Note whether the horse ever slides or lurches forward, and whether the walking animation comes back.
5. **Reverse steering uphill, 5 times:** as in step 2. If the horse lurches or slides at any point, keep S held, then press and release A or D, and note what happens.

### Wall

6. **Back into a wall:** hold S for 10 seconds with the horse's rear against a wall or large rock. Note whether it rocks forward and back, or stays still.

Close the game and leave the logs in place until they've been read.

## Stage 9d results

| Step | What happened |
| --- | --- |
| Load | |
| 1. S from standing | ok |
| 2. Reverse steering on flat ground | ok |
| 3. Stopping from a trot and a gallop | ok - even on hills |
| 4. Straight uphill | ok |
| 5. Reverse steering uphill | ok |
| 6. Into a wall | ok |

- Any forward lurch you could feel? no
- Any crash or freeze? no
- Notes: reverse uphill tests are not perfect executions on my part. maintaining straight line on slope challenging, so was a hybrid if reverse and reverse steer uphill. no noticeable problems though.
