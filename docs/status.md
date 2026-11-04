# Status and next steps

Where the mod stands and what to do next. Requirements are in [discovery.md](discovery.md) and evidence is in [findings/](findings/).

**Status:** `src/HorseSteer` is the working mod, and v0.0.1 is the working horse prototype. Forward steering and free look work at walk, trot and gallop, S with A or D pivots, and S alone
walks the horse backward. The mod pauses during the taming minigame.

## Next

1. Make these early improvements to the horse, using the Horse Tap Brake findings in `findings/offsets.md`. Build and test them in this order, one at a time:
   - **Logging:** add the mount's chart `state`, `value` and `+0x188` velocity to `HorseSteer.csv`, with no change in behaviour. Test a walk, a gallop, a stop, the backward walk and a freeze after a won minigame. Velocity
     should follow `speed` without the jumps from chunk changes, and `state` should hold steady within a gait and change between gaits. Record which `state` and `value` the frozen horse shows, as a lead for
     the freeze below.
   - **Speed:** if velocity passes, set `g_mountSpeed` from it, updated separately from `WriteTelemetry`, which stops updating it if the CSV can't open or passes 500,000 rows. Forward steering and the backward walk
     both depend on it, so retest them, and recheck `kBackwardMaxStartSpeed`, which is set to the old position noise.
   - **Lookups:** replace fixed addresses with RTTI for vtables, the reference count for `kWorldSystemSlot`, and signatures with exactly one match for code. Have the mod go idle and log which lookup failed. This
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
4. Before any public release, find which input hook causes the freeze after a won minigame, by testing each alone, then try moving
   it. The tester accepts a remount until then.
5. Remove the CSV and other test logging for release.
6. Retest the taming minigame on the current build. Stage 6 didn't reach it, because wild horses take a long time to find.
