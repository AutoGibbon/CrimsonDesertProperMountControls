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

**Next session:** player reports, and the next game update. After an update, run `tools/check_lookups.py` on the new exe.

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
