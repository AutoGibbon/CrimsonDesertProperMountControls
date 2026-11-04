# Stage 5f test

This build moves the horse backward at 1 m/s from the mod itself while S alone is held. It writes the horse's position directly, so it ignores
collisions and may look wrong. This is a one-off test, and the code won't stay in the mod unless it's worth keeping.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply, so the new build is copied to `bin64`. Leave
   `HorseSteer Backward Walk Test` on, so the clips can be tried.
2. Load your save and wait until at least 20 seconds after launch. Stay on flat, open ground, with some clear space behind the horse.

Save before this test. The horse may end up inside something, so be ready to dismount or reload.

## Steps

1. **From standing, hold S for about 5 seconds, then release:** note whether the horse moves backward smoothly, jitters or snaps back on
   release, and whether the rider and camera stay with it.
2. **Hold W at a walk, release it, and once the first backward clip starts, hold S until the clips end:** note how the movement and clips look
   together, and whether the feet match the ground.
3. **Ride off with W:** the horse should set off normally from where it ended up.
4. **Hold S on a slope, uphill and downhill:** note whether the horse follows the ground.
5. **Hold S with a wall or tree close behind the horse:** note whether it stops or goes through.

Close the game and leave the logs in place until they've been read.

## Stage 5f results

| Step | What happened |
| --- | --- |
| Load | |
| 1. S from standing | smooth movement |
| 2. S during the clips | movement is too fast for the animations |
| 3. Riding off | yes |
| 4. Slope | follows ground |
| 5. Wall or tree | stops at walls |

- Any crash or freeze?
- Is this worth keeping, if the problems can be fixed?
- Notes: turning with reverse movement not possible, might be impossible given no ground mount has built in working backward movement states and only half baked unused states available.
