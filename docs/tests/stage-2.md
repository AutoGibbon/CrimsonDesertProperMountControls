# Stage 2 test

This build makes S a brake and limits the mod to horse-type mounts. Reversing isn't possible through input, because the horse chart has no route into its backward walk.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply, so the new build is copied to `bin64`. Check the log's first lines mention "hooks installed".
2. Load your save and wait until at least 20 seconds after launch.

## Steps

1. **Ride with W, A and D at a trot:** this should feel the same as before. If nothing has changed from the game's normal controls, stop and report it, as the horse check has failed.
2. **Hold W, then release W and hold S at a trot or gallop:** the horse should slow down and stop, without turning around.
3. **Standing still, hold S:** the horse should stay put.
4. **Hold S with A, then S with D:** the horse should act as if only A or D were held.
5. **Other mounts, if you have one available that isn't horse-based, such as the exosuit or a bear:** the controls should be the game's normal ones.
6. **Other horse-type mounts, if you have one, such as a golem horse or donkey:** the horse controls should apply.

Leave `HorseSteer` enabled until the logs have been read.

## Stage 2 results

| Step | What happened |
| --- | --- |
| 1. W, A and D at a trot | has reverted to base controls, no more horse relative movement, other steps not tested due to regression |
| 2. S to brake | |
| 3. S from standing | |
| 4. S with A or D | |
| 5. Non-horse mount | |
| 6. Other horse-type mount | |

- Any crash or freeze?
- Notes:
