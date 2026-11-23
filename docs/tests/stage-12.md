# Stage 12 test

This build recognises mounts by itself, so you shouldn't need End. It also sets each mount's backward speed from `HorseSteer.ini`, and gives a short pulse when you release A or D in reverse, so that the walking animation returns without looking up each mount's states. The main things to check are that the controls turn on, that the speeds look right, and that the pulse helps without moving the mount forward.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply.
2. Load the save with the mounts and wait until at least 25 seconds after launch.

## Steps

Do these on each mount in the table, in any order. Each mount should take about a minute. Start with the camel, boar and bear, which lost the walking animation in stage 11b. If a mount's controls don't turn on, press End (long beep) to run the other steps, and note it in the table.

1. **Hold W and add A or D:** the mount should steer by itself, without End.
2. **From standing, hold S for 5 seconds:** the mount should walk backward, and the feet should match the ground. If they don't, note whether it's too fast or too slow.
3. **Hold S, add A or D for about 2 seconds, then release it while still holding S for about 2 seconds:** the walking animation should return. Do this three times, using both A and D. Note if it slides in its pose, or if it moves or lurches forward just after the release.

If you can, also try a second kind of bear in the same way. It can use a different package, so its log line is useful.

Close the game and leave the logs in place until they've been read.

## Stage 12 results

y = good, n = bad, empty = not tested.

| Mount | 1. Controls on without End | 2. Feet match, or too fast or too slow | 3. Walking animation returns | 3. No forward lurch |
| --- | --- | --- | --- | --- |
| Horse | y| y| y| y|
| Camel | y| y| n| y|
| Reindeer | y| y/n| y| y| -- slopes can exit the reverse walk state - same for snow deer, maybe horse too
| Snow deer | y| y/n| y/n| y| -- seems fickle. lost the reverse walk on a slope once, don't think lost it on steer release but marking suspect anyway
| Boar or warthog | y| y| y/n| y| -- longer turns result in failed recovery. shorter turns recovered. possible similar case with bear
| Bear | | | | | lost first bear when swapping mounts in save data sorry
| Second bear | y| y| 3/4| y|
| Wolf | y| y/n| y| y|
| Tiger | y| y| y| y| -- only lost reverse walk when dropping off a rock
| Iguana | y| y/n| y/n| y|
| Carmabirdsaurus | y| y| y| y|
| Ibex | y| y| y| y|

- Other mounts tried: in order

| kukubird | y| y| n| y|
| lion | y| y| y| y| -- got hit by carriage in middle of test

- Any crash or freeze? no
- Notes: slopes maybe? silly human can only attempt to correlate events.
- Correction from the tester: the feet matched on every mount. The `y/n` in that column for the reindeer, snow deer, wolf and iguana meant that the mount failed to keep walking backward.

## What the logs show

The logs are in `scratch/probe-runs/stage12/`. All 13 mounts ridden were recognised without End, and DMM deployed `HorseSteer.ini`. There were 77 release pulses, with no forward lurch reported. The losses and what they led to are in [backward-walk.md](../findings/backward-walk.md).
