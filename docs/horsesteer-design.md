# HorseSteer design

How `src/HorseSteer`, published as Proper Horse Controls, works. Requirements are in [discovery.md](discovery.md),
evidence in [findings/](findings/), and slowing a mount in [slowdown.md](slowdown.md).

## What it does

Forward steering and free look work at walk, trot and gallop, S with A or D pivots, and S alone walks the mount
backward, on the horse and the 12 other mounts reached, with no setup. On the wagon, S reverses it through the game's
brake key, and A or D steer as in the game. Tapping S with W held, the brake key or back on the stick slows any of them.
The mod pauses during the taming minigame.

`ResolveRide` takes the rider from `[[manager+0x58]+0xD8]`, the controlled actor, which follows a switch to a follower,
and only the rider's and its mount's input is rewritten.

## Startup checks

The mod finds its addresses at startup ([offsets.md](findings/offsets.md)), and setup ends with a log verdict:

- **Required:** `vtActorManager`, `vtVehicleComponent`, `vtRodeoUi`, `worldSystemSlot`, `physicsSlot`, `hookSite` and
  `stickSite`. If one fails, the log reads `FAILED (required)`, no hooks are installed, and setup ends with
  `MOD INACTIVE`.
- **Optional:** `keyCheck`, `conditionTableCall` and the slot 77 lookup. If one fails, or the rodeo switch doesn't
  install, the log reads `not found (optional)` and setup ends with `MOD ACTIVE without <features>`. The feature drops
  out or falls back, such as the zeroed-stick slowdown when slot 77 isn't found.
- **Planned:** any failed check turns the whole mod off, instead of running with parts missing (`regression-shutdown`
  in [state/tasks.json](state/tasks.json)).

The lookups don't cover every field the mod reads. On `0x6AF5D198` the physics object moved from `+0x2C0` to `+0x2D0`
with every lookup passing, which is why `g_physicsSlot` is looked up and `tools/check_fields.py` exists. After a game
update, follow the `game-update` skill.

## Settings

- **Ini delivery:** DMM copies the zip's ini to `bin64` only when there's none, and keeps an existing one on update.
  `LoadSettings` writes any missing setting with its default at startup, so a release that adds settings needs no
  player steps. `tools/pack.py` sets `Logging=0` in the zipped ini, so `src/HorseSteer/ProperHorseControls.ini` keeps
  `Logging=1` for tests.
- **`Reload`** under `[Keys]`, End by default, reads the ini again in game. It applies everything but `Logging`, a new
  backward speed applies to the current mount, and a setting deleted from the ini takes its default.
- **`Toggle`** under `[Keys]` sets the toggle key, which doesn't beep. **`InvertSteer`** under `[Reverse]`, default 1,
  swaps A and D during the backward walk and the wagon's brake reverse. `SteerInput` is fixed at 0.56 in the code.
- **`CameraDetached`** and **`TapBrake`** under `[Features]` turn off the mount-relative controls and the slowdown
  separately. With both off, the mod matches the toggle being off.
- **`StickDeadzone`:** `OnStickRead` zeroes x on an analog stick within this share of the angle from straight back to
  sideways, 0.30 or about 27°, because the game turns a mount at its full rate of about 64°/s for any non-zero x. Past
  the deadzone the turn is on or off.
- **`KeyRampMs`** under `[Steering]`, 0 to 500, default 300: A or D ramp in over this time, and back out after release.
  Below `kStandingSpeed` (0.2 m/s) the ramp is at least `kStandingRampMs` (100 ms), so that a low `KeyRampMs` doesn't
  stick the horse turning on the spot.
- **Steering bias:** `LowSpeed`, `LowSpeedBias`, `HighSpeed` and `HighSpeedBias` under `[Steering]`, defaults 2.2, 0.89,
  4.8 and 0.61. A bias is W+A's angle as a share of 90°, capped at 85° (0.94), and the angle blends between the two
  speeds. `HighSpeed` at or below `LowSpeed` switches at `LowSpeed`, which is untested.
- **`PHC_SetSuspended`:** an export that lets another mod turn this one off while it holds it, per owner. It's
  documented in the README and on the mod page.
