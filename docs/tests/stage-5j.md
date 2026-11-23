# Stage 5j test

If you hold S while the horse is moving, it now slows down first, then starts walking backward once it has nearly stopped. From standing, this should
work as it did in stage 5i.

The numpad keys from stage 5i still work.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply, and leave `HorseSteer Backward Walk Test` off.
2. Load your save and wait until at least 20 seconds after launch.

## Steps

1. **At a walk, release W and hold S at once, 10 times:** count how many give the backward walk, and note how long the wait feels.
2. **At a trot, release W and hold S at once, 5 times:** as above.
3. **At a gallop, release W and hold S at once, 5 times:** as above.
4. **From standing, hold S for about 5 seconds, 5 times:** count how many give the backward walk.
5. **Alt-tab away and back, then hold S from standing, 3 times:** note whether the horse ever moves with no animation.
6. **While riding at speed, tap S with A or D to make a tight turn:** this should feel the same as before, with no backward walk.
7. **While walking backward, add A or D, then release them with S still held:** note whether the backward walk comes back cleanly.

Close the game and leave the logs in place until they've been read.

## Stage 5j results

| Step | What happened |
| --- | --- |
| Load | |
| 1. From a walk: walks out of 10, wait | perfect. maybe drop the min speed to 0.2, sometimes activation is little choppy as it forces blend too quickly from the forward settling state - this applies to trot and gallop |
| 2. From a trot: walks out of 5, wait | as above |
| 3. From a gallop: walks out of 5, wait | as above |
| 4. From standing: walks out of 5 | perfect |
| 5. After alt-tab | perfect |
| 6. S with A or D at speed | perfect |
| 7. Adding and releasing A or D | backward walk returns cleanly but the A/D inputs produce forward motion making steering in reverse look clunky |

- Any crash or freeze? no
- Notes: n/a
