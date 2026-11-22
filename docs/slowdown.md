# Slowing a mount

How Proper Horse Controls slows a mount. Chart evidence is in
[charts-and-braking.md](findings/charts-and-braking.md) (Slowing a mount with S, Slowing a mount with the brake key,
Slowing while steering) and, for the wagon, [mounts-and-vehicles.md](findings/mounts-and-vehicles.md) (Wagon slowdown).

## Inputs

Tapping S with W held, pulling the stick back, or pressing the game's brake key with forward input held slows the mount
by one gait, with steering held or not. S alone stays the backward walk, and on the wagon the brake reverse.

- **S** is read with `GetAsyncKeyState('S')`, which ignores rebinding. Accepted until players report it.
- **Brake key:** `BrakeKeyHeld` calls the game's key query, so Alt, mouse X2 and gamepad B all start the slowdown.
- **Gamepad:** the stick pulled back works wherever S does, so gamepad users are no worse off.
- **`TapBrake=0`:** `UpdateBrakeGap` isn't called.
- **`CameraDetached=0`:** the slowdown starts from S with W held, or the brake key with any stick input, with no
  steering check, because a sideways stick there says nothing about steering.

The code is in `UpdateBrakeGap` and `OnCachedCondition`, which answers chart conditions for the mount through slot 77 of
`ClientCharacterControlActorComponent`'s vtable. `OnStickRead` runs on more than one game thread, so `RunBrakeGap` runs
`UpdateBrakeGap` for the mount's input only, under a try-lock, and other calls read `g_brakeGap` and `g_stepDown`.

## Horse and wagon: slowdown

`g_brakeGap` makes the chart read `T10` ("no move input") as holding and `TE` ("move input") as not, until the second
consecutive change into a chart state whose value is 0 or missing, or until speed falls below `kBrakeGapStopSpeed`
(0.3 m/s). The stick is left as is, so steering carries on. If slot 77 isn't found, the slowdown zeroes the stick
instead, and steering blocks it.

- **Horse, held brake:** its chart reads `key_brake` (`OR T10 key_brake`), so holding the brake key keeps it slowing
  until it stops, without the mod. It isn't a step-down mount.
- **Wagon:** the same rule, except that a tap from `mount_value` above `kBrakeMountLowValue` (5.5) also ends on the
  first slowing state below `kBrakeMountEndSpeed` (5 m/s). The wagon's chart reads `key_brake` only at idle, so the
  brake key doesn't reverse a moving wagon. With Shift held at top speed a tap does little, which is accepted.

## Other mounts: step down

`g_stepDown` makes the chart read `T66` as holding until the state changes, and every non-horse ride chart then moves to
the gait below. `GroundMount::stepDown` marks these mounts, and `STEP_DOWN` in `tools/check_charts.py` must match it. If
the state hasn't changed 200 ms after its first 500 ms (the chart's `TBA(0.5,4)`), the mount is at its lowest gait and
the slowdown runs instead, logged as `source=no step`.

- **S during a step down:** W with S, or a stick pulled back, reads as no move input. From a step down started by
  either, `g_holdInput` makes the chart read move input until it's released, or for at most `kHoldInputMs` (500 ms)
  after the step, so that holding S still slows the mount.
- **Holding the brake:** once the input that started a step down has been held for `kHoldBrakeMs` (500 ms), each gait
  change starts the next step down, down to the slowdown at the lowest gait. Released partway, the mount stays at the
  gait it reached. A hold released just after 500 ms drops two or three gaits, which is kept as a feature.
- **Steering:** a hold while steering keeps stepping down, as a tap does.

## Ruled out

- **Position write (`AssistPerSecond`):** floated the horse, and made the wagon's speed jump back up when it ended.
- **Answering slot 77 for `T10` alone:** the chart flips in and out of the slowing state, because it goes back up on
  `TE`.
- **Learning gaits as the mount is ridden:** players can't be asked to run through every gait first.
- **`T167` as the gear Shift selects:** it reads 0 all session on every mount.

## Not handled

- **Brake key taps that keep their gait:** if they come back, the untried fallback is answering `key_brake` as not held
  for the mount during the gap, in `OnKeyCondition`, so that Alt gives the chart the same input as S.
- **A tap passing through `C2B2`:** resets the count in `UpdateBrakeGap` and drops two gaits. Seen once, on a slow horse
  the tester thinks had hit something. Left as is unless it happens on open ground.
