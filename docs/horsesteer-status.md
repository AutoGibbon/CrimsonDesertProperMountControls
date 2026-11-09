# HorseSteer status

Where the land mount mod stands and what to do next. Requirements are in [discovery.md](discovery.md) and evidence is in
[findings/](findings/). Boat work is tracked in [boats/status.md](boats/status.md).

## Status

`src/HorseSteer` (v1.3.0, matching `mod_page/changelog.txt`) is the mod. Forward steering and free look work at walk, trot and gallop, S with A or D pivots,
and S alone walks the mount backward, on the horse and the 12 other mounts reached, with no setup. On the wagon, S
reverses it through the game's brake key, and A or D steer as in the game. Tapping S with W held, the brake key or
back on the stick slows any of them. The mod pauses during the taming minigame. `CameraDetached` and `TapBrake` under
`[Features]` turn off the mount-relative controls and the slowdown separately, and `Reload` under `[Keys]`, End by
default, reads the ini again in game.

It finds its addresses at startup, and every lookup matched on the two game updates since build 26423880, up to PE timestamp
`0x6AED2E85` (2026-11-06). A copy of that exe is in `scratch/exe-baseline/`,
and it's the only build in `KNOWN` in `tools/check_lookups.py`.

## Resume here

Work is on `main`. v1.0.0 is the first Nexus release, as Proper Horse Controls: the zip from `tools/pack.py`,
with the page text in `mod_page/description.bbcode`. v1.1.0 adds the slowdown. It's tagged `1.1.0` and packed as
`build/ProperHorseControls-1.1.0.zip`, and is on Nexus. v1.2.0 adds `StickDeadzone`, the feature flags and `Reload`. It's
tagged `1.2.0` and packed as `build/ProperHorseControls-1.2.0.zip`. v1.3.0 adds `PHC_SetSuspended`. It's tagged `1.3.0`
and packed as `build/ProperHorseControls-1.3.0.zip`, and is on Nexus.
`DMM\mods\ProperHorseControls.asi` is the stage 33 build, which differs from v1.3.0 only in `kVersion`. The old
`HorseSteer.*` files are still in `DMM\mods` and `bin64`, so `HorseSteer` must stay off in DMM to avoid loading two
copies.

DMM copies the zip's ini to `bin64` only when there's none, and keeps an existing one on update. The mod writes any
missing setting at startup, so a release that adds settings needs no player steps. Check settings in
`bin64\ProperHorseControls.ini`.

**Next:** player reports and the next game update. After an update, run `tools/check_lookups.py` on the new exe.

## Slowing a mount with S

Tapping S with W held, pulling the stick back, or pressing the game's brake key with forward input held slows the mount
by writing 0 to the raw stick until the second consecutive change into a chart state whose value is 0 or missing, or
until speed falls below 0.3 m/s, which passed [stage 29](horsesteer-tests/stage-29.md). The code is in `UpdateBrakeGap`, on every ground
mount. Evidence is in `findings/charts-and-braking.md` (Slowing a mount with S), with tests in
stages 21 to 29.

- **Horse:** drops one gait, 48 of 48 taps in stage 21.
- **Other mounts:** every other ride chart has one slowing node for all gaits, so a tap from the top gaits lands where
  that chart lands (bear 5.2, camel 3.5). The tester accepted this over learning gaits at runtime.
- **Position write:** removed, with `AssistPerSecond`. It floated the horse and made the wagon's speed jump back up when
  it ended, for about a quarter off the bear's and camel's longest drops.
- **S key:** read with `GetAsyncKeyState('S')`, which ignores rebinding, accepted until players report it.
- **Stage 23:** passed on the horse, bear and wagon. The camel wasn't logged on this build.
- **Limits accepted:** on non-horse mounts a tap lands where the chart lands, and the camel's 9.0 lands on 3.5.
- **Brake key:** done, passed [stages 24](horsesteer-tests/stage-24.md) and [25](horsesteer-tests/stage-25.md).
  `BrakeKeyHeld` calls the key query, and Alt, mouse X2 and gamepad B all start the slowdown. Details are in
  `findings/charts-and-braking.md` (Slowing a mount with the brake key). If taps ever fail to drop a gait again, the
  untried fallback is answering `key_brake` as not held for the mount during the gap, in `OnKeyCondition`, so that Alt
  gives the chart the same input as S.
- **Wagons:** enabled with the same rule, except that a tap from `mount_value` 5.5 also ends on the first slowing state
  below 5 m/s. Passed [stage 28](horsesteer-tests/stage-28.md). S alone stays as reverse. The wagon's chart reads `key_brake` only at
  idle, so the brake key doesn't reverse a moving wagon. With Shift held at top speed a tap does little, accepted.
- **Preference:** don't make gamepad users worse off. Use the back stick wherever it's available.
- **Credit:** the mod page and README credit Horse Tap Brake for the idea and release condition (`findings/offsets.md`),
  say how this mod differs, and list it as not compatible. The changelog is at 1.1.0.

## Settings added in v1.2.0

- **`StickDeadzone`:** `OnStickRead` zeroes x on an analog stick within this share of the angle from straight back to
  sideways, 0.30 or about 27°, because the game turns a mount at its full rate of about 64°/s for any non-zero x. Passed
  [stage 30](horsesteer-tests/stage-30.md) on the horse. The wagon wasn't reached. Past the deadzone the turn is on or off.
- **`CameraDetached=0`:** the only stick edit left is the slowdown's zeroed stick. The slowdown then starts from S with W
  held, or the brake key with any stick input, with no steering check, because a sideways stick there says nothing about
  steering. Passed [stage 31](horsesteer-tests/stage-31.md).
- **`TapBrake=0`:** `UpdateBrakeGap` isn't called. Passed stage 31. With both flags off the mod matches the toggle being
  off.
- **`Reload`:** applies everything but `Logging`, and a new backward speed applies to the current mount. Worked twice mid
  ride in stage 31. The rest of [stage 32](horsesteer-tests/stage-32.md) hasn't been run.
- **Stage 31's one odd slowdown:** a tap from 12.0 dropped two gaits, because the chart passed through `C2B2` and reset the
  count in `UpdateBrakeGap`. The horse was already slow, and the tester suspects a collision. Left as is unless it happens
  on open ground.

## Added in v1.3.0

- **`PHC_SetSuspended`:** an export that lets another mod turn this one off while it holds it, per owner, documented in
  the README and on the mod page. No mod calls it yet. Passed [stage 33](horsesteer-tests/stage-33.md) on the horse.

## Release work

All done for v1.0.0.

1. **Recovery speed limit:** done, passed [stage 18](horsesteer-tests/stage-18.md).
2. **`Logging=0`:** done. `tools/pack.py` sets `Logging=0` in the zipped ini, so
   `src/HorseSteer/ProperHorseControls.ini` keeps `Logging=1` for tests.
3. **Exe baseline:** done. `scratch/exe-baseline/` holds only the current exe (`0x6AED2E85`), and `KNOWN` only its
   addresses. The 3479 and 26423880 copies were discarded.
4. **Shift+F5, toggle key and reverse steering invert:** done, passed [stage 19](horsesteer-tests/stage-19.md) on the
   horse and wagon. Shift+F5 and `g_flip` are gone, `Toggle` under `[Keys]` sets the toggle key, and `InvertSteer`
   under `[Reverse]` swaps A and D during the backward walk and the wagon's brake reverse, with 1 as the default.
   `SteerInput` is fixed at 0.56 in the code, and the toggle no longer beeps.
5. **Missing ini settings:** done, passed [stage 20](horsesteer-tests/stage-20.md). `LoadSettings` writes any missing
   setting with its default.

## Known limitations

- **Wagon reverse steering** is as poor as the game's own: A or D held circles the wagon in place at 60°/s. The chart
  moves and turns a root transform directly, and the wagon body's angle to the horse wasn't found in stages 16 and 17.
  Finding it would need a scan of all game memory by position, then a chain for it, then a steering controller, which
  was judged not worth the effort. If it's reopened, the cheapest step is a reduced A or D input during the brake reverse
  (`E14A`), never tried there. Details are in `findings/mounts-and-vehicles.md` (Wagon).
- **Untested mounts:** the elephant (`CD_M0002_Elephant`, in `g_groundMounts`, with the same chart and blend layout as
  the mounts that work) couldn't be reached, as its only save record didn't appear in the mount list. Donkeys have no
  save record. If a player reports a problem with either, recheck them.
- **Other vehicles** haven't been reached: the cow-drawn wagon (`cd_m0002_00_wagoncow`), the carriage
  (`cd_r0012_00_carriage`) and the bomb cart (`cd_r0011_00_bombcart`). Whenever one can be taken, drive it with
  `Logging=1` and check `mount package` in the log. No package covers the bomb cart, so the mod should stay inactive on
  it.
- **Freeze after a won taming minigame:** accepted, with a remount as the workaround (`findings/stability.md`).
- **State constants:** `kStateWalk` and the other state constants are low pointer bits for the horse and ibex. A
  missing `mount_value` catches most lost walks, but not `8FC`, whose value is 0, and failure only disables that part of
  the recovery.
- **Saves:** another save's mount keys and available mounts may differ. If a save won't load, check for a stale
  `bin64\dmm\manifest.tsv` first.

## Done

- **Logging, speed and lookups:** chart state and velocity logging ([stage 8](horsesteer-tests/stage-8.md)), speed from
  velocity ([stage 9](horsesteer-tests/stage-9.md) to [9d](horsesteer-tests/stage-9d.md)), and addresses found at
  startup by RTTI, reference counts and unique signatures, with the mod idle if a required lookup fails
  ([stage 10](horsesteer-tests/stage-10.md)). Evidence is in `findings/offsets.md` and `findings/backward-walk.md`.
- **Every reachable legged ground mount** ([stages 11 to 13](horsesteer-tests/stage-13.md)), with results in
  `findings/mounts-and-vehicles.md` and `findings/backward-walk.md`.
- **Wagon:** forward steering and reverse through the brake key ([stages 14](horsesteer-tests/stage-14.md) and
  [15](horsesteer-tests/stage-15.md)).

## Ideas

- A true reverse turn rate setting, which would be new work (see `findings/backward-walk.md`). Writing the mount's
  rotation is ruled out, as it wouldn't work well with the reverse turn animation.
- The key query and slot callback (`findings/offsets.md`, `findings/charts-and-braking.md`) and the other movement
  classes (`findings/mounts-and-vehicles.md`, Leads) are leads, not planned work.
