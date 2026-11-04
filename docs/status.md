# Status and next steps

Where the mod stands and what to do next. Requirements are in [discovery.md](discovery.md) and evidence is in [findings/](findings/).

**Status:** `src/HorseSteer` is the working mod, and v0.0.1 is the working horse prototype. Forward steering and free look work at walk, trot and gallop, S with A or D pivots, and S alone
walks the horse backward. The mod pauses during the taming minigame. It runs on Steam build 26423880 (PE timestamp `0x6AEA07A6`) and finds its addresses at startup, and the exe copies for this build and 3479 are in `scratch/exe-baseline/`.

## Resume here

Work is on branch `tap-brake-improvements`. The build in `DMM\mods` matches the source. The Logging, Speed and **Lookups** steps are done, passing [stage 10](horsesteer-tests/stage-10.md) and [stage 10b](horsesteer-tests/stage-10b.md), so the mount work in step 2 below is next. The freeze after a won minigame is accepted, with a remount as the workaround, and no further work is planned on it (evidence in `findings/stability.md`). `kStateWalk` and the other state constants stay as low pointer bits: `mount_value` can't separate the walk from nearby states, and failure only disables recovery.

The start of the backward walk is otherwise fine at `kBackwardMaxStartSpeed` 0.2: it came slightly later than the times in `findings/backward-walk.md`, and the tester reported no problem.

## Next

1. Make these early improvements to the horse, using the Horse Tap Brake findings in `findings/offsets.md`. Build and test them in this order, one at a time:
   - **Logging:** done in [stage 8](horsesteer-tests/stage-8.md). The results are in `findings/offsets.md`.
   - **Speed:** done. `g_mountSpeed` comes from velocity, updated in `OnStickRead` every frame ([stage 9](horsesteer-tests/stage-9.md) to [stage 9d](horsesteer-tests/stage-9d.md)). The state drops those tests found are in `findings/backward-walk.md`.
   - **Lookups:** done. Fixed addresses are replaced with RTTI for vtables, the reference count for `g_worldSystemSlot`, and signatures with exactly one match for code. A failed lookup makes the mod go idle and logs it, except `g_keyCheck`, which is log only. This
     protects the mount work below from a game update.

   Once the logging passes, the same columns can show each mount's stop states during the work below. The key query, slot callback and other movement classes are leads for later, not part of this phase.
2. Get every legged ground mount working, which a public release requires. How the horse's backward walk works is in
   `findings/backward-walk.md`. For each mount in `kGroundMountPackages`, check the controls and whether that technique applies, and confirm the list has every legged
   mount. This also decides how S should work on them: clearing backward input may block backsteps and backward walks on mounts such as the wolf and elephant, so limit
   that to the horse where the technique doesn't apply. The test save has only one basic mount, and the tester is working out how to reach others, so wait for that
   before continuing. Then work through these, starting with mounts whose data and mechanisms are closest to the horse's:
   - Confirm each listed package's ride chart and blend files. `character/binary/motionblending/horse/` also has `donkey_move`, so check how donkeys and other horse
     variants are covered. `tools/mount_census.py` lists the vehicle types and `Riding_` characters, and `findings/mounts-and-vehicles.md` has the open questions on the ibex and donkeys.
   - Look for backward clips in those blend files, and for a stop state like horse node 186, which holds while the mount keeps moving.
   - Look for existing backward moves that clearing S would block.
   - Sort the mounts into those where the horse method should work as it is, those needing changes, and those with no backward clips, then test in game.
3. Investigate carriages, which should get similar controls if possible. This is expected to be separate work, with its own round of probe logging and code execution
   capture. They aren't in `kGroundMountPackages`, and the wagon has its own charts (`r0012_wagon_upper`, which reads `key_brake`, and `ride_carriage_lower` and
   `ride_carriage_upper`). The first question is whether replacing the camera vectors applies to them.
4. Remove the CSV and other test logging for release.
