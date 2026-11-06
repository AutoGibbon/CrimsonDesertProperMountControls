# Stage 18 test

`UpdateBackwardRecovery` no longer sends its once-a-second forward pulse when the horse is at or below 0.3 m/s, the same
limit its missing-value pulse already uses. Backed against a wall, the horse should stay still while S is held instead of
pulsing forward. Across the saved logs, 26 of 46 earlier pulses were at or below 0.3 m/s.

## Setup

1. Copy `bin64\HorseSteer.log` and `bin64\HorseSteer.csv` to `scratch/probe-runs/` if they're still there.
2. Close the game. Check that `bin64\HorseSteer.ini` has `Logging=1` under `[Debug]`. In DMM, turn `HorseSteer` off and
   on again and apply.
3. Launch and wait until at least 25 seconds after launch.

## Steps

On the horse:

1. **Back into a wall with S and keep holding S for 5 seconds:** the horse should stay against the wall, with no
   forward step about once a second.
2. **Back along open ground with S for 5 seconds, then tap A and D a few times:** the backward walk should play and
   recover after each turn as before.
3. **Back up a slope with S for 5 seconds:** the backward walk should keep playing.

Close the game and leave the logs in place until they've been read.

## Logs

No new lines. `backward recovery pulse` lines should now only show speeds above 0.3.

## Stage 18 results

| 1. Wall | 2. Open ground | 3. Slope |
| ------- | -------------- | -------- |
| ?       | y              | y        |

- Any crash or freeze? no
- Notes: can't honestly tell any difference.

After the steps, the tester reversed over small objects to try to trip the recovery. The horse did what you'd expect,
and no further changes are wanted. Logs are in `scratch/probe-runs/stage18/`. From the logs, 23 pulses fired, all at
0.30 m/s or above:

- **Contact jitter:** stuck against an object, the horse's velocity alternated between about +0.3 and −0.3 m/s with no
  net movement, and the pulse still fired about once a second, each followed by a small forward nudge. Smoothing the
  backward part of the velocity over about 0.3 s would stop these, if it's ever needed.
- **Stalled walk:** behind a small object, pulses sent the horse into `A37D` and back to `A8FC` and `AAE5` without
  walking, about every 2 s.
- **Recoveries:** while backing at 0.4 to 0.6 m/s with the state dropped to `A8FC`, pulses returned the horse to the
  walk as intended.
