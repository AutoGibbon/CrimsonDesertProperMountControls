# Status and next steps

Where the mod stands and what to do next. Requirements are in [discovery.md](discovery.md) and evidence is in [findings/](findings/).

**Status:** `src/HorseSteer` is the working mod, and v0.0.1 is the working horse prototype. Forward steering and free look work at walk, trot and gallop, S with A or D pivots, and S alone
walks the horse backward. The controls and backward walk also work on the 12 other mounts reached, with no setup. The mod pauses during the taming minigame. It runs on Steam build 26423880 (PE timestamp `0x6AEA07A6`) and finds its addresses at startup, and the exe copies for this build and 3479 are in `scratch/exe-baseline/`.

## Resume here

Work is on `main`. The build in `DMM\mods` matches the source. The Logging, Speed and **Lookups** steps are done, passing [stage 10](horsesteer-tests/stage-10.md) and [stage 10b](horsesteer-tests/stage-10b.md). The mount work in step 2 below is done on every reachable mount, passing [stage 12](horsesteer-tests/stage-12.md), [stage 12b](horsesteer-tests/stage-12b.md) and [stage 13](horsesteer-tests/stage-13.md) on a modified save with a range of mounts. Tracing is behind `Logging` in `HorseSteer.ini`, off by default, and `LogProblem` always writes startup problems. That change compiles but hasn't run in game, so check it at the next launch.

Carriages in step 3 are next, but they're blocked: the test save shows wagons with no option to steal or mount them. The tester plans to get another save from community resources, or progress the current one. To resume:

1. In DMM, turn `HorseSteer` off and on again and apply, so that `bin64` gets the current build. Check that with `Logging=0` the controls still work, and that `bin64` has no `HorseSteer.log` or `HorseSteer.csv`, because launching deletes any left from earlier runs. Saved logs in `scratch/probe-runs/` aren't affected.
2. Once a wagon can be mounted, set `Logging=1` in `DMM\mods\HorseSteer.ini` and apply. Drive one for about a minute with W, A, D and S while moving the camera, and try a cow-drawn one too. No new build is needed, because the mod stays inactive on a package it doesn't recognise.
3. Read `mount package` and `mount appearance` in the log and the CSV. Then add the wagon's package to a test build, to find out whether replacing the camera vectors steers it.

If another save is used, its mount keys and available mounts may differ, and a stale `bin64\dmm\manifest.tsv` is the first thing to check if it won't load. The freeze after a won minigame is accepted, with a remount as the workaround, and no further work is planned on it (evidence in `findings/stability.md`). `kStateWalk` and the other state constants stay as low pointer bits for the horse and ibex: a missing `mount_value` catches most lost walks, but not `8FC`, whose value is 0, and failure only disables that part of the recovery.

The start of the backward walk is otherwise fine at `kBackwardMaxStartSpeed` 0.2: it came slightly later than the times in `findings/backward-walk.md`, and the tester reported no problem.

## Next

1. Make these early improvements to the horse, using the Horse Tap Brake findings in `findings/offsets.md`. Build and test them in this order, one at a time:
   - **Logging:** done in [stage 8](horsesteer-tests/stage-8.md). The results are in `findings/offsets.md`.
   - **Speed:** done. `g_mountSpeed` comes from velocity, updated in `OnStickRead` every frame ([stage 9](horsesteer-tests/stage-9.md) to [stage 9d](horsesteer-tests/stage-9d.md)). The state drops those tests found are in `findings/backward-walk.md`.
   - **Lookups:** done. Fixed addresses are replaced with RTTI for vtables, the reference count for `g_worldSystemSlot`, and signatures with exactly one match for code. A failed lookup makes the mod go idle and logs it, except `g_keyCheck`, which is log only. This
     protects the mount work below from a game update.

   Once the logging passes, the same columns can show each mount's stop states during the work below. The key query, slot callback and other movement classes are leads for later, not part of this phase.
2. Get every legged ground mount working, which a public release requires. This is done on every mount that could be reached, with results in `findings/mounts-and-vehicles.md` and `findings/backward-walk.md`. What's left:
   - The elephant can't be reached, as its only save record didn't appear in the mount list, and donkeys have no save record to add. If a player reports a problem with either, recheck them.
   - Consider a true reverse turn rate setting, which would be new work (see `findings/backward-walk.md`).
3. Investigate carriages, which should get similar controls if possible. This is expected to be separate work, with its own round of probe logging and code execution
   capture. They aren't in `g_groundMounts`, and the wagon has its own charts (`r0012_wagon_upper`, which reads `key_brake`, and `ride_carriage_lower` and
   `ride_carriage_upper`). The first question is whether replacing the camera vectors applies to them.
4. Before release, add a speed limit to the horse's state-ID recovery in `UpdateBackwardRecovery`, which pulses once a second when the horse is stopped against something. Keep the limit below the 0.8 to 0.91 m/s where it still fires usefully.
