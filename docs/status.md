# Status and next steps

Where the mod stands and what to do next. Requirements are in [discovery.md](discovery.md) and evidence is in
[findings/](findings/).

## Status

`src/HorseSteer` (v0.0.1) is the mod. Forward steering and free look work at walk, trot and gallop, S with A or D pivots,
and S alone walks the mount backward, on the horse and the 12 other mounts reached, with no setup. On the wagon, S
reverses it through the game's brake key, and A or D steer as in the game. The mod pauses during the taming minigame.

It finds its addresses at startup, and every lookup matched on the two game updates since build 26423880, up to PE timestamp
`0x6AED2E85` (2026-11-06). Exe copies for build 26423880 (`0x6AEA07A6`) and 3479 are in `scratch/exe-baseline/`.

## Resume here

Work is on `main` and committed. The build in `DMM\mods` matches the source. DMM doesn't copy `HorseSteer.ini` to
`bin64`, so check settings in `bin64\HorseSteer.ini`, which currently has `Logging=1`.

**Next session: publishing.** The mod is feature complete for a first release. Start with the open items in "Before
release", then the release itself. Still to decide with the user: where it's published (Nexus, GitHub or both) and what
goes out (a player-facing readme, a zip of `HorseSteer.asi` and `HorseSteer.ini`, or both). The player-facing readme should cover the controls (F5 toggle, Shift+F5 flip), the `.ini`
settings, the mounts supported and the known limitations below.

## Before release

1. **Recovery speed limit:** done, passed [stage 18](horsesteer-tests/stage-18.md).
2. **`Logging=0`:** `src/HorseSteer/HorseSteer.ini` has `Logging=1`, so set it to 0 for release. Then check that the
   controls work and that launching leaves no `HorseSteer.log` or `HorseSteer.csv` in
   `bin64`.
3. **Exe baseline:** copy the current exe (`0x6AED2E85`) to `scratch/exe-baseline/` and add it to `KNOWN` in
   `tools/check_lookups.py`. The `0x6AEB7D59` exe can no longer be copied.

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
