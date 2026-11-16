# Holding the brake to stop on every mount

Not started. A player report (gamepad, uses only the brake feature, no logs) said that on the horse, holding the brake
stops it, but on other mounts only pressing it does. That matches the code and the charts.

## Current behaviour

- **Horse:** its chart reads `key_brake` (`OR T10 key_brake`), so holding the brake key keeps it in the slowing state
  until it stops. The game does this without the mod.
- **Other land mounts:** the bear, camel, reindeer, elephant, iguana, Carma bird and cucu bird charts don't read
  `key_brake`. The wolf and warthog read it in one walk branch only
  ([charts-and-braking.md](findings/charts-and-braking.md), `key_brake` on other mounts).
- **The mod reacts to the press only:** `UpdateBrakeGap` starts on `brakePressed = brake && !brakeWas`. On
  `g_stepDownMount` that is one step down on the `T66` ladder, and a slowdown only from the lowest gait (`no step`).
  Holding the brake key, or holding the stick back, doesn't start another, so the mount drops one gait and carries on.
- **Earlier evidence:** with W and Alt held, the camel slowed but didn't stop, and the tester accepted it
  ([charts-and-braking.md](findings/charts-and-braking.md), Hold). Stages 24, 31, 43 and 44 test taps only.

## Proposed change

In `UpdateBrakeGap`, when a step down ends on `gait changed` and the brake key is still held with forward input, start
the next step down. Use the same `stepDeadline` wait (`kStepDownStateMs` + `kStepDownWaitMs`) so that the chart's
`TBA(0.5,4)` is met. At the lowest gait this already falls through to the `no step` slowdown, which stops the mount.

- **Which inputs:** apply this to the brake key and the stick held back. Decide whether S held with W counts too. S
  alone is the backward walk, and on `g_brakeMount` it is the brake reverse, so neither must change.
- **Horse:** leave it alone. Its chart already handles a hold, and it isn't `g_stepDownMount`.
- **Steering:** a hold while steering should keep stepping down, consistent with v1.4.0 braking while turning.
- **`TapBrake=0`:** no change, because `UpdateBrakeGap` isn't called.

## Test

Write a new stage in `docs/horsesteer-tests/`, run with a gamepad and with KBM, on the bear, camel and wolf, plus the
horse as a control:

1. From the highest gait with forward held, hold the brake key until the mount stops. Expect one `step down start`
   per gait, then `brake gap start ... source=no step`, then `reason=stopped`.
2. Repeat with the stick pulled back and held.
3. Tap the brake key at each gait. Expect one gait per tap, as in v1.4.0.
4. On the horse, hold the brake key. Expect the same as v1.4.0.
