# Stage 15 test

On the wagon, S now presses the game's brake key, which the wagon's chart treats as reverse, instead of using the mod's
backward walk. A or D with S pass through as they do with the brake key. Other mounts keep the backward walk. This build
also ramps A or D in over `SteerRampSeconds` during the other mounts' backward walk, and only ramps keyboard steering.

## Setup

1. Copy `bin64\HorseSteer.log` and `bin64\HorseSteer.csv` to `scratch/probe-runs/` if they're still there.
2. Close the game. Check that `bin64\HorseSteer.ini` has `Logging=1` under `[Debug]` and `SteerInput=0.56` under
   `[Reverse]`. In DMM, turn `HorseSteer` off and on again and apply.
3. Launch and wait until at least 25 seconds after launch.

## Steps

On a wagon:

1. **From standing, hold S for 5 seconds:** the wagon should reverse as it does with the brake key.
2. **Hold S and add A or D:** compare with the brake key and A or D.
3. **From a trot or faster, hold S:** note what the wagon does.
4. **Hold W with A or D, then release:** steering should be as in stage 14.

On the horse:

5. **From standing, hold S, then add A or D:** the backward walk should start and steer as before, with the turn easing
   in over the first 0.4 s.

Close the game and leave the logs in place until they've been read.

## Logs

`HorseSteer.log` adds:

- `lookup conditionTableCall` and `brake key installed`, or a `brake key:` line if it failed.
- `reverse by brake key` or `reverse by backward walk` on each `mount package` line.
- `brake reverse on` and `off` each time S starts and stops braking the wagon.
- `key_brake read for actor ...` for up to 4 actors whose chart reads the brake key while S brakes, with the game's own
  answer, where 1 means not pressed.
- `stick input digital` or `analog` with the raw stick, each time steering input changes between the two.

## Stage 15 results

| Mount | 1. S from standing | 2. S with A or D | 3. S at speed | 4. Forward steering | 5. Horse backward walk |
| --- | --- | --- | --- | --- | --- |
| good | exact same wonky behaviour that was always present in base game | good | good | good | horse backward steer has a delay now? |

- Any crash or freeze? no
- Notes: horse revers steer regression?
