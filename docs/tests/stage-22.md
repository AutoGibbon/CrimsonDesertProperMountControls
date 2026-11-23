# Stage 22 test

In stage 21, a tap of S on the bear or camel dropped two or three gaits, because those charts share one slowing state.
With `ReleaseOnSpeed=1`, the stick gap also ends once the mount's speed reaches its next lower gait. The mod learns each
mount's gaits as it's ridden, from each `mount_value` held for 0.3 s with W down.

It answers:

1. Whether a mount resumes at the gait just below when W returns partway through its slowing state, or at the same low
   gait as in stage 21.
2. Whether a tap at the two lowest gaits still nearly stops the mount.
3. How `AssistPerSecond=0.4` looks on these mounts, where the gaps are longer than the horse's.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. Close the game. In DMM, turn `ProperHorseControls` off and on again and apply.
3. Set `Logging=1` in `bin64\ProperHorseControls.ini`, and set this section, which each run below changes:

   ```ini
   [BrakeTest]
   Enabled=1
   AssistPerSecond=0
   ReleaseOnSpeed=1
   ```

4. Use flat, open ground.

All the runs can share one launch. At the start of each run, set its value in the ini, save, return to the game and
press Insert, so that run A is marker 1 and run D is marker 4.

When finished, close the game and copy the log and CSV to `scratch/probe-runs/stage22/`. The next launch overwrites them.

## Steps for each run

1. After mounting, hold W and use Shift to go up through every gait, holding each for at least a second, so that the mod
   learns them. If you remount or change mount, do this again.
2. From the top gait down, use Shift to reach the gait, release it, then hold W and settle at a steady speed for about 2
   seconds. For the skill gait, keep W and Shift held instead.
3. Tap S once and keep W held for 5 seconds. Note the gait it settles at, and anything odd in the feet or body.
4. Do this twice per gait.

## Runs

| Run | Marker | Mount | `AssistPerSecond` |
| --- | --- | --- | --- |
| A | 1 | Bear | 0 |
| B | 2 | Bear | 0.4 |
| C | 3 | Camel | 0 |
| D | 4 | Camel | 0.4 |

## Logs

- Log: `gait value` lines for each gait learned, and `brake gap start` with its `target`, the next lower gait, or 0 if
  none has been learned. `brake gap end` gives the reason `speed`, `next state`, `steering` or `time cap`.
- CSV: `mount_value` and `mount_vel` in the seconds after each `brake_gap` ends.

## Stage 22 results

| Run | Gaits landed one lower? | Lowest two gaits | Feet and body |
| --- | --- | --- | --- |
| A. Bear, 0 | looked ok | ? | ok |
| B. Bear, 0.4 | looked ok | ? | ok |
| C. Camel, 0 | think camel slowed more than it should? | ? | ok |
| D. Camel, 0.4 | as above | ? | ok |

- Any crash or freeze?
- Notes: hard to say if better or worse than stick-gap approach, just different.
  - Logs are in `scratch/probe-runs/stage22/`. Insert wasn't pressed for run A, so the `marker` values are A 0, B 1, C 2
    and D 3. There are no taps logged at the bear's 15.5, or the camel's 9.9 and 6.2.
  - Bear: 12 of 12 taps from 13.3, 9.2 and 5.2 landed one gait lower. A tap at 5.2 took about 0.6 to 0.8 s, against the
    3 s near-stop in stage 21.
  - Camel: 3.5 to 2.6 in 4 of 4 taps, in about 0.35 s. 9.0 released at about 6.1 m/s but landed on 3.5 in 4 of 4 taps,
    two gaits down.
  - Lowest gait, 1.8 and 2.6: no lower gait to target, so the gap still ran to the 3 s cap with the mount nearly stopped.
  - `AssistPerSecond=0.4` shortened the gaps by about a quarter: bear 13.3 from about 590 to 450 ms, camel 9.0 from about
    600 to 470 ms. It peaked at 2.0 m/s on the bear and 1.4 m/s on the camel.
