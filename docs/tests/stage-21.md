# Stage 21 test

A logging build for slowing a mount with S, without the game's brake key. With `[BrakeTest]` on, tapping S while W is
held zeroes the stick until the chart reaches the mount's next speed threshold down, then W takes over again. A or D
ends it. `AssistPerSecond` adds a position write against the mount's movement during that time, growing each second by
that share of the mount's speed at the tap.

It answers:

1. What the raw stick reads with W and S both held.
2. Whether the mount stays at the lower gait once W takes over.
3. How a position write acts against forward motion: speed, chart states and foot match.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. Close the game. In DMM, turn `ProperHorseControls` off and on again and apply.
3. Set `Logging=1` in `bin64\ProperHorseControls.ini`, and add this section, which each run below changes:

   ```ini
   [BrakeTest]
   Enabled=0
   AssistPerSecond=0
   ```

4. Use flat, open ground. Use Shift to reach the gait, release it, then hold W and settle. Don't touch Shift again until
   the step is finished.

All the runs can share one launch. At the start of each run, set its values in the ini, save, return to the game and
press Insert. That reloads `[BrakeTest]` and adds 1 to the CSV's `marker` column, so run A is marker 1 and run E is
marker 5. Press it for run A too, even with the values unchanged.

When finished, close the game and copy the log and CSV to `scratch/probe-runs/stage21/`. The next launch overwrites them.

## Run A: W and S together

`Enabled=0`, on the horse.

1. At a walk, trot and gallop, hold W and settle, then also hold S for 3 seconds, then release S. Keep W held throughout.

## Run B: stick gap

`Enabled=1`, `AssistPerSecond=0`, on the horse.

1. At each of walk, trot, canter and gallop, hold W and settle, tap S once, and keep W held for 5 seconds. Do this three
   times per gait, and note whether the horse settles at a lower gait or climbs back.
2. With W and Shift held throughout, tap S once at a gallop and keep both held for 5 seconds. Do the same during the
   skill that needs W and Shift held past a gallop. Note whether anything slows.
3. At a gallop, hold W and press A or D at the same moment as S, then the same with S first and A or D a moment later.
   There should be no slide or drift.

## Runs C and D: position write

`Enabled=1`, on the horse. Run C has `AssistPerSecond=0.2` and run D has `AssistPerSecond=0.4`.

1. At a trot, canter and gallop, hold W and settle, tap S once, and keep W held for 5 seconds. Do this three times per
   gait.
2. Watch the feet during the slowdown, and note any sliding, lurch at the end, or backward movement.

## Run E: another mount

`Enabled=1`, `AssistPerSecond=0`, on the bear or camel, repeat run B's step 1 at the gaits it has.

## Logs

- CSV: `raw_stick_y` with `w` and `s` both 1 in run A, `brake_gap` and `brake_assist` during taps, and `mount_value` and
  `mount_vel` in the seconds after each `brake_gap` ends.
- Log: `brake gap start` and `brake gap end` lines, with the end reason `next state`, `steering` or `time cap`, and the
  `state` lines between them.

## Stage 21 results

| Run | Result |
| --- | --- |
| A. Horse's gait with W and S held | no notes |
| B1. Walk | all run b results looked good |
| B1. Trot | |
| B1. Canter | |
| B1. Gallop | |
| B2. Gallop with Shift held | |
| B2. Skill with Shift held | |
| B3. No slide or drift | no slide or drift |
| C. 0.2: feet and end of the slowdown | slight "float" occurs during slowdown, like the animation freezes for short time. D will probably have more obvious |
| D. 0.4: feet and end of the slowdown | float not as obvious |
| E. Mount used, and result |  |

- Any crash or freeze?
- Notes:
  - Logs are in `scratch/probe-runs/stage21/`. Insert wasn't pressed for run A, so the `marker` values are A 0, B 1, C 2
    and D 3.
  - Run A: `raw_stick_y` read 0 in all 214 rows with W and S held, and the horse came to a stop each time.
  - Run B: with W held and no Shift, 48 of 48 taps from `mount_value` 14, 12, 9.5 and 5.2 held the next value down 3 s
    later. Taps at 2.8 nearly stopped the horse in about 2 s, then it walked on.
  - Run B, Shift held: 3 taps at `mount_value` 14, none at 12. Each left `E38E`. In the 2 where W and Shift stayed held,
    the value was back at 14 within about 1 s.
  - Runs C and D: the write reached at most 1.1 m/s. Canter gaps lost speed at about 2.4 m/s² with no write, 3.2 at 0.2
    and 3.6 at 0.4, from different starting speeds. The walk tap didn't get shorter.
  - Run E: logs are in `scratch/probe-runs/stage21e/`, with the bear as marker 1 and the camel as marker 2. The rule only
    gave one gait down where the next gait was the chart's first stop point:
    - Bear: 9.2 to 5.2 in 4 of 4 taps. 13.3 (5 taps) and 15.5 (2 taps) also landed on 5.2. 5.2 (3 taps) and 1.8 (3 taps)
      slowed to under 0.1 m/s, ended on `time cap`, and W then resumed at 1.8.
    - Camel: 6.2 to 3.5 in 3 of 3 taps. 9.0 (5 taps) and 9.9 (1 tap) also landed on 3.5. 3.5 and 2.6 (1 tap each) slowed
      to under 0.1 m/s, ended on `time cap`, and W then resumed at 2.6.
