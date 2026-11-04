# Stage 10 test

This build finds the game's addresses when it starts, instead of using fixed ones, and no longer checks the game's build. Riding should feel the same as stage 9d. The main thing to test is that nothing has changed, and that the taming minigame still pauses the mod.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply.
2. Load your save and wait until at least 25 seconds after launch, which is 5 seconds longer than before, so that the lookups have finished.

## Steps

### Riding

1. **Mount your horse and hold W at a walk, trot and gallop, moving the camera around:** the horse should keep its heading, and A or D should steer it.
2. **From standing, hold S with A or D:** the horse should pivot.

### Backward walk

3. **From standing, hold S for about 5 seconds, then release:** the horse should walk backward with its walking animation, then return to idle.
4. **Reverse steering:** from standing, hold S for 3 seconds, add A or D for 3 seconds, release A or D, and keep holding S for 3 more seconds. The horse should return to its walking animation.
5. **At a gallop, release W and hold S at once:** the horse should stop, then walk backward.
6. **Dismount, then mount again:** the controls should work as before.

### Taming

7. **Tame a wild horse, if you can find one in reasonable time:** the minigame's controls should be normal. After a win, the horse not moving until a remount is the known issue. If you can't find one, note that here, because this build's address for the minigame then remains untested.

Close the game and leave the logs in place until they've been read.

## Stage 10 results

| Step | What happened |
| --- | --- |
| Load | |
| 1. Forward steering and free look | ok |
| 2. Pivot from standing | ok |
| 3. S from standing | ok |
| 4. Reverse steering | ok |
| 5. From a gallop | ok |
| 6. Dismount and remount | oops forgot. probably ok |
| 7. Taming | interesting find. first tame game completed, I mashed a few random keys before closing the game end UI, horse moved ok. second tame game, did not mash keys before close tame game UI, horse frozen (but S key reverse naturally works since this is not state dependent)|

- Any crash or freeze? no
- Notes: good.
