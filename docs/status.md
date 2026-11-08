# Status and next steps

Where the mod stands and what to do next. Requirements are in [discovery.md](discovery.md) and evidence is in
[findings/](findings/).

## Status

`src/HorseSteer` (v1.0.0, matching `mod_page/changelog.txt`) is the mod. Forward steering and free look work at walk, trot and gallop, S with A or D pivots,
and S alone walks the mount backward, on the horse and the 12 other mounts reached, with no setup. On the wagon, S
reverses it through the game's brake key, and A or D steer as in the game. The mod pauses during the taming minigame.

It finds its addresses at startup, and every lookup matched on the two game updates since build 26423880, up to PE timestamp
`0x6AED2E85` (2026-11-06). A copy of that exe is in `scratch/exe-baseline/`,
and it's the only build in `KNOWN` in `tools/check_lookups.py`.

## Resume here

Work is on `main` and committed. v1.0.0 is the first Nexus release, as Proper Horse Controls: the zip from `tools/pack.py`,
with the page text in `mod_page/description.bbcode`. `DMM\mods\ProperHorseControls.asi` matches the source. The old
`HorseSteer.*` files are still in `DMM\mods` and `bin64`, so `HorseSteer` must stay off in DMM to avoid loading two
copies.

DMM copies the zip's ini to `bin64` only when there's none, and keeps an existing one on update. The mod writes any
missing setting at startup, so a release that adds settings needs no player steps. Check settings in
`bin64\ProperHorseControls.ini`.

**Next session:** finish the S slowdown for release, starting with the two additions under "Slowing a mount with S".
After that, player reports and the next game update. After an update, run `tools/check_lookups.py` on the new exe.

## In progress: slowing a mount with S

Tapping S with W held, or pulling the stick back, slows the mount by writing 0 to the raw stick until the second change
into a chart state whose value is 0 or missing, with no `key_brake`. The code is in `UpdateBrakeGap`, on every ground
mount but the wagon, with the position write in `UpdateBrakeAssist`. Evidence is in `findings/charts-and-braking.md` (Slowing a mount with S), with tests in
stages 21 to 23.

- **Horse:** drops one gait, 48 of 48 taps in stage 21.
- **Other mounts:** every other ride chart has one slowing node for all gaits, so a tap from the top gaits lands where
  that chart lands (bear 5.2, camel 3.5). The tester accepted this over learning gaits at runtime.
- **Position write:** `AssistPerSecond` under `[Slowdown]`, default 0.4. Stage 23 passed with it on the longer drops.
- **S key:** read with `GetAsyncKeyState('S')`, which ignores rebinding, accepted until players report it.
- **Stage 23:** passed on the horse, bear and wagon. The camel wasn't logged on this build.
- **Limits accepted:** on non-horse mounts a tap lands where the chart lands, and the camel's 9.0 lands on 3.5.
- **To add, brake key:** the game's brake key (Alt, mouse X2, gamepad B) should also start the slowdown. The tester
  reversed the earlier decision, since a player who wants to drift does it on purpose. `OnKeyCondition` can't detect the
  press, because it only runs when a chart reads `key_brake`, and most ride charts don't. The likely route is calling the
  key query (slot 8, `findings/charts-and-braking.md`) with hash `0x377B9DB8` from `OnStickRead`, which also respects
  rebinding and gamepads, but it's untested in this mod. Keep drift working: the drift branches need `key_brake` and
  forward stick input, and `UpdateBrakeGap` already won't start with A or D held, and ends if they're pressed. Test Alt
  with A or D at a gallop.
- **To add, wagons:** the tester wants the slowdown on wagons. Remove the `brakeMount` exclusion for S with W held, and
  keep S alone as reverse. W+S reads 0 on the stick, so it doesn't reach the brake reverse path. The wagon's chart treats
  `key_brake` as reverse, so what the real brake key should do on a moving wagon needs checking first, against node 43's
  entry conditions (`findings/mounts-and-vehicles.md`). The position write on a wagon is untested, because its chart
  moves a root transform.
- **Preference:** don't make gamepad users worse off. Use the back stick wherever it's available.
- **Before release:** credit Horse Tap Brake for the idea and release condition (`findings/offsets.md`),
  and update the mod page, README and changelog.

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

- A true reverse turn rate setting, which would be new work (see `findings/backward-walk.md`).
- The key query and slot callback (`findings/offsets.md`, `findings/charts-and-braking.md`) and the other movement
  classes (`findings/mounts-and-vehicles.md`, Leads) are leads, not planned work.
