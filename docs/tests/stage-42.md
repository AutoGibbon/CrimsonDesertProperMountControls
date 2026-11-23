# Stage 42 test

Answering `TE` (type `0x0E`, "move input") as not holding for the mount during a gap, as well as `T10` ("no move input")
as holding, both through slot 77. In [stage 41](stage-41.md) the horse's slowing states went straight back up to a gait
on `TE`, which held with the stick left as is. Stage 41's write to the condition cache is removed. The stick is still
left as is, but `TE` may also drive steering or other branches, so this checks that the mount still turns during a gap
and responds after it.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. The build is already in `DMM\mods\`. In DMM, turn `ProperHorseControls` off and on again and apply.
3. In `bin64\ProperHorseControls.ini`, set `Logging=1`, `CameraDetached=1` and `TapBrake=1`.
4. Load save slot 1 and use the horse.

## Steps

On the horse, with the keyboard:

1. **Canter and trot, straight:** hold W and tap Alt 3 times from each.
2. **Canter and trot, turning:** hold W and D and tap Alt 3 times from each. Note whether the horse keeps turning
   while it slows.
3. **Gallop 1 and 2:** hold W, then hold W and D, and tap Alt twice from each.
4. **S while turning:** hold W and A and tap S, then release W and hold S with A.
5. **After each tap:** keep W held and check the horse speeds up again, and that A and D still steer.
6. **Backward walk:** from standing, hold S, then S with A.

If the horse passes, repeat steps 1, 2 and 5 on the bear or wolf.

Close the game and copy the log and CSV to `scratch/probe-runs/stage42/`.

## Logs

- **Gap reads:** after each `brake gap end`, as in [stage 40](stage-40.md). `type=0x0E ... result=1 game=0` lines are
  the mod's answer to `TE`.
- **CSV:** `mount_phys_yaw` shows whether the mount kept turning during a gap.

## Reading the results

| Outcome | Meaning |
| --- | --- |
| Each tap ends on `reason=next state`, one gait down, with no flipping, and the horse keeps turning | The fix. Next is the other mounts' landing gaits (`TC9`) |
| No flipping, but the horse stops turning during a gap | `TE` also drives the turn. Note whether it resumes after the gap |
| Flipping as in stage 41 | Another branch sends the chart back. Compare the states with `tools/chart_node_branches.py` |
| The horse doesn't speed up or steer after a gap | Something outlasts the gap. Note how long |

tester notes for steps:

1. :D
2. :D
3. :D
4. :D
5. I explained this already before. holding W only maintains a gait. Shift is required to increase gait. logs show this where I tap shift to step up through gaits. record this properly so future work understands it.
6. not impacted.

## Results

Passed on the horse and bear, on 2026-11-14. Logs are in `scratch/probe-runs/stage42/`.

- **No flipping:** all 61 gaps ended on `reason=next state` (52) or `reason=stopped` (9), after exactly two state
  changes, with no time caps.
- **Horse, one gait per tap:** canter (`C2B2`) ended at 5.3 to 5.5 m/s in 310 to 700 ms, gallop 1 (`D099`) at 9.0 to
  9.5 m/s, gallop 2 (`E38E`) at 10.5 m/s, and trot (`B4A7`) at 2.7 m/s.
- **Bear:** from `1A1A` and `1BCB`, gaps ended at 4.6 to 4.9 m/s, where its straight taps land. From `1891`, the gait
  below, the bear slowed to a stop.
- **Turning:** during gaps with A or D held, the horse turned at 52 to 59°/s and the bear at 57 to 74°/s, against a
  median of 59°/s with W and a steering key outside gaps.
- **Step 5:** as written it expected W to raise the gait, which it doesn't. The gait table in
  [README.md](README.md) now says so.

Answering `T10` as holding and `TE` as not, for the mount during a gap, slows it while it keeps turning.