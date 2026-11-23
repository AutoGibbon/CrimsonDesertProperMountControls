# Stage 14 test

This build adds the wagon's package, `CD_R0012_Wagon_0001`, to the mounts the mod controls. The question is whether the
horse controls work on a wagon: W steers with A or D and ignores the camera, and S walks it backward. Wagons can only be
taken as they pass by, so there's no fixed route.

## Setup

1. Copy `bin64\HorseSteer.log` and `bin64\HorseSteer.csv` to `scratch/probe-runs/` if they're still there, because
   turning the mod off deletes them.
2. Close the game. Check that `DMM\mods\HorseSteer.ini` has `Logging=1` under `[Debug]`. In DMM, turn `HorseSteer` off
   and on again and apply.
3. Launch and wait until at least 25 seconds after launch.

## Steps

Take any wagon that passes and drive it for about a minute. Repeat on any other kind you come across, such as a
cow-drawn wagon, a carriage or a bomb cart.

1. **Hold W and turn the camera:** the wagon should keep its heading.
2. **Hold W and add A or D:** the wagon should steer.
3. **From standing, hold S for 5 seconds:** the wagon should go backward, or note what it does instead.
4. **Hold S and add A or D:** note how it turns.

Close the game and leave the logs in place until they've been read.

## Stage 14 results

| Wagon | 1. Camera ignored | 2. Steering | 3. S from standing | 4. Reverse steering |
| --- | --- | --- | --- | --- |
| y | y | y | y | y |

- Any crash or freeze? no
- Notes: wagons actually use the dedicated brake key to reverse, but our S key also works.
Wagon reverse steering is equally horrible with the base game key and the S key, identical behaviour.
perhaps we can investigate drastically reducing the turn rate?

things to try

for wagon, use the builtin reverse action instead of our bodged one for normal mounts
lower turn rate - like a LOT lower. start at 20% of whatever it is now if possible.
