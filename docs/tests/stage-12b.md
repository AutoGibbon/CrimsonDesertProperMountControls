# Stage 12b test

In stage 12, mounts could fall out of the backward walking animation and slide for several seconds. This build notices that from the game's chart data, with no per-mount setup, and keeps pulsing until the walk returns. It replaces the pulse on releasing A or D. The main things to check are that slides now end quickly, and that the pulses don't move the mount forward.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply.
2. Load the save with the mounts and wait until at least 25 seconds after launch.

## Steps

Do these on the camel, boar, cucubird, reindeer and horse, in that order. Each mount should take about two minutes. Other mounts are welcome if you have the time.

1. **Long turn:** hold S, add A or D for about 4 seconds, then release it and keep holding S for 5 seconds. Do it once each way.
2. **Short turn:** hold S, add A or D for about half a second, then release it and keep holding S for 5 seconds. Do it once each way.
3. **Slope:** hold S and walk backward up and then down a slope for about 10 seconds each way.
4. **Wall:** hold S and back into a wall or large rock for about 5 seconds. The mount should stop against it and shouldn't step forward.

For each one, note whether the mount slid in its pose, and for about how long, and whether it moved or lurched forward.

Close the game and leave the logs in place until they've been read.

## Stage 12b results

Use `ok` or a few words, such as `slid 1 s` or `lurched`.

| Mount | 1. Long turn | 2. Short turn | 3. Slope | 4. Wall |
| --- | --- | --- | --- | --- |
| Camel | good| good| good| good|
| Boar or warthog | good| good| good| good|
| Cucubird | good| good| good| good|
| Reindeer | good| good| good| good|
| Horse | good| good| good| good|

- Other mounts tried: no
- Any crash or freeze? no
- Notes: bonus test at the end with kukubird. jump off cliff and test if gliding behaviour respects modified steering (it did).

## What the logs show

The logs are in `scratch/probe-runs/stage12b/`. The longest stretch moving with no chart value was 0.4 s on the camel and 0.2 s on the boar, cucubird and reindeer, down from 2.6 to 4.0 s in stage 12, though the two runs took different routes. `backward lost pulse` fired 1 time on the camel, 5 on the boar, 9 on the cucubird and 6 on the reindeer. The horse had no lost pulses, and its state IDs fired 6 pulses from `A8FC`, whose value is 0. Rows with S held and the mount moving forward were from the tester dashing to a new starting position, then holding S.
