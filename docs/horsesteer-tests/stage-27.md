# Stage 27 test

In [stage 26](stage-26.md), a wagon tap at cruise ran to the 3 second cap and dipped to about 1 m/s before climbing back
to 2.8 m/s. A tap that starts at `mount_value` 5.5 now ends on the first slowing state once speed is below 5 m/s. Wagon only.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. Close the game. In DMM, turn `ProperHorseControls` off and on again and apply.
3. Set `Logging=1` in `bin64\ProperHorseControls.ini`.
4. Use a wagon on flat, open ground.

## Steps

All with W held and no Shift during the tap. Keep W held for 5 seconds after each tap.

1. **Cruise:** hold Shift to top speed, release Shift, wait about 10 seconds, about 5.5 m/s. Tap S 3 times and Alt 3 times,
   returning to cruise between taps. Note how it feels: the wagon should ease down, without nearly stopping.
2. **Top:** at top speed, release Shift and tap S at once, 2 times.
3. **Low:** from standing, tap S within the first 2 seconds of holding W, 2 times.

Close the game and copy the log, CSV and ini to `scratch/probe-runs/stage27/`.

## Logs

- Cruise: `brake gap start` at about 5.5, then `brake gap end` after about 500 ms with `reason=end speed`. `reason=time cap`
  means the rule didn't apply.
- Top and low: `reason=next state`, as in stage 26.
- CSV: `mount_vel` should not go much below 2.8 after a cruise tap.

## Stage 27 results

| Step | Result |
| --- | --- |
| 1. Cruise: how far it slowed | slowed to somewhere between a walk and trot |
| 1. Cruise: how it feels | works. good. |
| 2. Top | lurch from top, second tap good |
| 3. Low | stops, continues |

- Any crash or freeze? no
- Notes: only the lurch feels off. wagon slows dramatically, then jumps back up in speed. maybe too much assist.
  - Logs are in `scratch/probe-runs/stage27/`.
  - Cruise, 4 taps from 5.3 to 5.9 m/s: `reason=end speed` at about 4.95 m/s in 0.22 to 0.44 s.
  - Top: `mount_vel` jumped when the position write ended. With `brake_assist` at 2.8 to 3.1 at the end, it went from 5.4
    to 6.0 m/s up to 7.0 to 7.9 m/s. At 2.2 to 2.3, 1 m/s. At 1.6, 0.5 m/s. At 1.0, about 0.1 m/s.
