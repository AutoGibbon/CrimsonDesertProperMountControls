# Stage 48 test

Holding the brake to stop on non-horse mounts ([slowdown.md](../slowdown.md)). On a mount that steps down one gait
per tap, the next step down now starts while the input that started the first one is still held: the brake key with
forward input, S with W, or the stick pulled back. At the lowest gait the existing slowdown stops the mount. Taps, the
horse and the wagon are unchanged.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. The build is already in `DMM\mods\`. In DMM, turn `ProperHorseControls` off and on again and apply.
3. In `bin64\ProperHorseControls.ini`, set `Logging=1` and check `TapBrake=1`.

## Steps

On the bear, camel and wolf, then the horse as a control. Do steps 1 to 3 with KBM, then again with a gamepad, where
the stick pulled back stands in for S with W. Use open flat ground.

1. **Brake key hold:** from the highest gait with forward held, hold the brake key until the mount stops.
2. **S hold:** from the highest gait with W held, hold S until the mount stops. With a gamepad, pull the stick back and
   hold it instead.
3. **Taps:** tap the brake key once at each gait. Expect one gait per tap, as in v1.4.0.
4. **Horse:** do steps 1 and 3 on the horse. Expect the same as v1.4.0.

Close the game and copy the log and CSV to `scratch/probe-runs/stage48/`.

## Logs

- **Holds:** one `step down start` with the first input's source, then one `step down start ... source=hold` per gait
  below it, each after a `step down end ... reason=gait changed`. At the lowest gait, `step down end ... reason=no step`,
  `brake gap start ... source=no step`, then `brake gap end ... reason=stopped`.
- **Taps:** no `source=hold` lines.
- **Horse:** `brake gap start` only, never `step down start`.

## Reading the results

| Outcome | Meaning |
| --- | --- |
| A hold steps down to a stop, and taps drop one gait each | The fix |
| A hold stops after one gait with no `source=hold` line | The step down didn't end on `gait changed`, or forward input dropped |
| `source=hold` lines repeat in the same gait | The chart didn't move on the wait and went to `no step` early |
| An S or stick hold goes back up a gait between step downs | Move input wasn't held between step downs |

Tester notes:

observed a few instances of what appeared to be brake taps dropping more than one gait on bear and wolf? everything else worked as advertised.

## Results

Holds passed, taps failed, on 2026-11-16. Logs are in `scratch/probe-runs/stage48/`. Four mounts were ridden
(`B0100005`, `B0100008`, `B010000A`, then the horse `B0100002`).

- **Holds:** one `source=hold` step down per gait, then `no step`, the slowdown and `reason=stopped`, with the brake key,
  S with W and the stick.
- **Taps chained (13 times):** in a gait held for a while, the step down ends on `gait changed` about 30 ms after the
  press, while the tapped key is still down, so a `source=hold` step down started on the next gait:

  ```
  105: brake key press  speed=9.73 value=9.200
  106: step down start  state=1A1A in state 3672ms source=brake key
  108: step down end    after=32ms reason=gait changed
  109: step down start  value=5.200 source=hold
  ```

  Lines 352, 436 and 590 chained to three or four gaits, with the key still down about 500 ms after the press.
- **Thread race:** the rider's and mount's `UpdateBrakeGap` calls ran at once (interleaved lines, e.g. line 44). At the
  lowest gait one started a `source=hold` step down alongside the `no step` slowdown (lines 55, 81, 602). It ended on
  `gait changed` about 30 ms later with no visible effect.

Fixed for [stage 49](stage-49.md): a hold only steps down the next gait once the input has been down for 500 ms
(`kHoldBrakeMs`), and `RunBrakeGap` runs `UpdateBrakeGap` for the mount's input only, under a try-lock.
