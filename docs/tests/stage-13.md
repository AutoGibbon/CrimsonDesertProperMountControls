# Stage 13 test

This build removes the test keys: End, Insert, Page Up and Page Down. Nothing else should change, so this is a short check that the mod still turns on and works by itself. F5 still turns the mod off and on.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply.
2. Load the save with the mounts and wait until at least 25 seconds after launch.

## Steps

Do these on the horse and one other mount, such as the camel. It should take a few minutes.

1. **Hold W and add A or D:** the mount should steer.
2. **From standing, hold S for 5 seconds:** the mount should walk backward at its usual speed, with its feet matching the ground.
3. **Hold S, add A or D for about 4 seconds, then release it and keep holding S for 5 seconds:** the walking animation should return, with no long slide.
4. **Press End, Insert, Page Up and Page Down:** there should be no beep from the mod.

Close the game and leave the logs in place until they've been read.

## Stage 13 results

| Mount | 1. Steering | 2. S from standing | 3. Reverse steering | 4. No beeps |
| --- | --- | --- | --- | --- |
| Horse | y|y |y |y |
| iguana| y| y| y| y|

- Any crash or freeze?
- Notes: the keys in step 4 weren't pressed, and the code that read them is gone. The tester then rode the horse looking for a wagon, and no wagon was mounted in the log.

## What the logs show

The logs are in `scratch/probe-runs/stage13/`. The horse and iguana were recognised without End, and the `.ini` was read. The horse's state IDs fired 6 pulses from `A8FC` in a row, once a second, at 0.05 to 0.20 m/s.
