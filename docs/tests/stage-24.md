# Stage 24 test

Pressing the game's brake key (Alt, mouse X2, gamepad B) with forward input held now starts the same slowdown as tapping
S, on every ground mount but the wagon. Alt with A or D should still drift. The mod reads the key through the game's own
key query, which hasn't been used in this mod before.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. Close the game. In DMM, turn `ProperHorseControls` off and on again and apply.
3. Set `Logging=1` in `bin64\ProperHorseControls.ini`.
4. Use flat, open ground. Settling means holding W at a steady speed for about 2 seconds, with no A or D.

## Steps

Do these on the horse, then on the bear or camel. Steps 3 and 4 only need the horse, because other mounts' charts don't
drift.

1. **Tap:** at each gait, use Shift to reach it, release Shift, settle, tap Alt once, and keep W held for 5 seconds. Do
   this twice per gait, and note the gait it settles at.
2. **Hold to a stop:** at a gallop, hold W and Alt until the mount stops, and note how it compares with S.
3. **Drift, together:** at a gallop, hold W and press A or D at the same moment as Alt. It should drift as in the base
   game. Do this 3 times each way.
4. **Drift, Alt first:** at a gallop, hold W, press Alt, and press A or D a moment later. Do this 3 times, and note
   whether it drifts.
5. **No forward input:** at a gallop, release W and press Alt. It should feel the same as releasing W.
6. **Standing:** from standing, press Alt with and without W. Nothing should change.
7. **Other bindings:** repeat one tap with mouse X2, and with gamepad B while holding the stick forward, if one is to hand.
8. **Wagon, if one is to hand:** Alt should still reverse it, with no slowdown behaviour.
9. **S taps:** at a gallop, tap S with W held. It should still drop one gait.

Close the game and copy the log, CSV and ini to `scratch/probe-runs/stage24/`.

## Logs

- `lookup keyCheck` with an address, not `FAILED`. If it failed, the brake key can't start a slowdown.
- `brake key press` on each press, with `forward`, `steering`, `speed` and `value`. No such lines mean the key query
  isn't answering.
- `brake gap start` with `source=brake key`, `source=S` or `source=stick`, and `brake gap end` with its reason. In step 4
  the reason should be `steering`.
- CSV: `brake_gap` and `brake_assist` during taps, and `mount_value` and `mount_vel` afterwards.

## Stage 24 results

| Step | Horse | Bear or camel |
| --- | --- | --- |
| 1. Tap: gait reached | expected | expected |
| 2. Hold to a stop | expected | did not fully stop (not actually a bad thing, consider forward and brake as slow down but not stop) |
| 3. Drift, together | y | n/a |
| 4. Drift, Alt first | y | n/a |
| 5. No forward input | y | y |
| 6. Standing | y | y |
| 7. Other bindings | y | y |
| 8. Wagon | y | 
| 9. S taps | y | y |

- Any crash or freeze? no
- Notes: interesting chart behaviour on camel there but no need to investigate. fine as is I think.
