# Stage 26 test

The slowdown now runs on the wagon with the same rule as other mounts, started by S with W held or the brake key with W
held. S alone still reverses it. `AssistPerSecond` applies, and this is its first use on a wagon.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. Close the game. In DMM, turn `ProperHorseControls` off and on again and apply.
3. Set `Logging=1` in `bin64\ProperHorseControls.ini`.
4. Use a wagon on flat, open ground.

## Steps

Speeds, all with W held:

- **Top:** hold Shift until the wagon stops gaining speed, about 11.9 m/s.
- **Cruise:** from top, release Shift and wait about 10 seconds, about 5.5 m/s.
- **Low:** from standing, tap S or Alt within the first 2 seconds of holding W, about 2 to 4 m/s.

1. **Taps:** at each speed, tap S and keep W held for 5 seconds. Do this 3 times per speed, then the same with Alt. Note
   the speed it settles at, and whether the wagon or animal jumps or jitters.
2. **Tap with Shift held:** at top, keep Shift and W held and tap S, 3 times.
3. **Hold to a stop:** at top, hold W and S until the wagon stops, then the same with W and Alt. W and S should stay
   stopped, and W and Alt should reverse, as in the unmodded game.
4. **S alone:** at top, release W and hold S. The wagon should slow, stop, then reverse.
5. **W and S into reverse:** at top, hold W and S, then release W with S still held.

Close the game and copy the log, CSV and ini to `scratch/probe-runs/stage26/`.

## Logs

- `brake gap start` with `source=S` or `source=brake key`, and `brake gap end` with the reason `next state`, `steering`
  or `time cap`. `time cap` from cruise means the tap ran the full 3 seconds.
- CSV: `brake_gap`, `brake_assist`, `mount_state`, `mount_value` and `mount_vel`.

## Stage 26 results

| Step | Result |
| --- | --- |
| 1. Taps at top | it slows yes |
| 1. Taps at cruise | it slows yes |
| 1. Taps at low | it slows yes |
| 1. Jumps or jitter | no jitters |
| 2. Tap with Shift held | ignored, shift wins |
| 3. Hold to a stop | stops, reverses |
| 4. S alone | stops, reverses |
| 5. W and S into reverse | stops, reverses |

- Any crash or freeze? no
- Notes:
  - Logs are in `scratch/probe-runs/stage26/`.
  - Top, S: 11.8 to 5.1 and 5.3 m/s in about 1 s. Then 2.8 m/s within about 4 s with W held.
  - Alt from 6.3 to 9.4 m/s, 3 taps: ended at 5.2 to 5.7 m/s in 0.36 to 0.7 s.
  - Cruise, 2 taps with S and 5 with Alt: all on the 3 s cap at 0.9 to 1.1 m/s, then back to 2.8 m/s in about 3 s.
  - Low, 3 taps each with S and Alt: 1.8 to 2.4 s, down to 0.26 m/s, then back to 2.8 m/s in about 4 s.
  - Shift held, 5 taps: ended at 10.7, 11.7, 11.9, 9.5 and 5.3 m/s. The log records Shift only at the start of a tap.
  - The position write peaked at 3.27 m/s. Every reverse row in the CSV has W released.
