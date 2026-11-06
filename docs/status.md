# Status and next steps

Where the mod stands and what to do next. Requirements are in [discovery.md](discovery.md) and evidence is in [findings/](findings/).

**Status:** `src/HorseSteer` is the working mod, and v0.0.1 is the working horse prototype. Forward steering and free look work at walk, trot and gallop, S with A or D pivots, and S alone
walks the horse backward. The controls and backward walk also work on the 12 other mounts reached, with no setup. The mod pauses during the taming minigame. It runs on Steam build 26423880 (PE timestamp `0x6AEA07A6`) and finds its addresses at startup, and the exe copies for this build and 3479 are in `scratch/exe-baseline/`.

## Resume here

Work is on `main`, and nothing since `d30134e` is committed. The build in `DMM\mods` matches the source, but DMM doesn't
copy `HorseSteer.ini` to `bin64`, so check settings in `bin64\HorseSteer.ini`. That file still has a
`SteerRampSeconds` line, which the mod now ignores.

The game updated to PE timestamp `0x6AEB7D59` on 2026-11-05, and every lookup still matched. Copy this exe to
`scratch/exe-baseline/` and add it to `KNOWN` in `tools/check_lookups.py`.

Since the last commit:

- **Wagon:** `CD_R0012_Wagon_0001` is in `g_groundMounts` with `brakeReverse`. On it, S answers the chart's `key_brake`
  as pressed, which drives the wagon's own reverse, and A or D pass through as with the brake key. This passed
  [stage 15](horsesteer-tests/stage-15.md). The table slot is found with the new `g_conditionTableCall` lookup.
- **Stick input:** only a raw stick Y below −0.5 counts as backward, and a smaller negative Y is cleared. Reverse steering
  scales with the stick's sideways push. The forward ramp skips analog input. Keyboard play is unchanged, and no gamepad
  has been tested.
- **Reverse ramp:** removed after stage 15, where it delayed the horse's reverse steering by about 0.44 s. The build
  without it hasn't run in game, so check that the horse steers in reverse at once.

The findings are in `findings/mounts-and-vehicles.md` (Wagon), `findings/charts-and-braking.md` and
`findings/input-and-steering.md`. The tester rates the wagon's reverse steering poor, in the game as well as through S.
A or D turns the animal off the wagon's axis, so it stops pushing through the harness, backs into the wagon and the
two circle in place. The rate of 55 to 60°/s is too fast to hold a small angle, so whether a small angle backs along a
curve isn't known.

Plan: keep the angle between the animal and the wagon small while S brakes the wagon.

1. **Check the push stops:** in `scratch/probe-runs/stage15/HorseSteer.csv`, the wagon's speed in `E14A` should fall
   toward 0 soon after A or D starts, while `mount_phys_yaw` keeps changing. The mount actor is the wagon, so
   `mount_phys_yaw` is the wagon's heading.
2. **Find the animal:** it isn't the mount actor, and only the wagon's chart reads `key_brake`
   (`key_brake read for actor` named only the wagon in stage 15), so it can't be found through the condition hook.
   Scan from the wagon actor for pointers to actors whose package reads as an animal (`ReadPackageName`, at `+0x18`),
   using `HorseProbe`'s scan as the template. If none link from the wagon, search the actors for the one with an animal
   package nearest the wagon. Log its address and chain, and check it's the same across remounts and a relaunch.
3. **Log the angle:** read the animal's yaw the same way as `mount_phys_yaw`, and add the animal's yaw and its angle to
   the wagon to the CSV. A test reverses straight, then with short taps of A or D, to see the angle where the push
   stops and whether the link pulls the animal back into line.
4. **Limiter:** while S brakes the wagon, pass A or D only while the angle is under a limit, for example 10 to 15°, and
   past it clear A or D or steer back toward the wagon's axis. Make the limit an ini value.

Fallbacks if the animal can't be found or the limiter doesn't hold the angle:

1. **Chart patch:** lower 1.047 in node 43's `Rotate` event in `r0012_wagon_upper`, for example to 0.2, through a DMM
   patch, so a small angle is easier to reach by hand. The field's meaning isn't confirmed, node 75 shares the event,
   and it isn't known whether it rotates the wagon or the animal.
2. **Smaller A or D input in the brake reverse:** scale A or D by `SteerInput` while S brakes the wagon. Input size made
   no difference in `C7EF`, but `E14A` hasn't been tried.
3. **Pulsed steering:** send A or D for part of each short interval, for example 20%, so the angle grows more slowly.
   It may show as a jerky turn.
4. **Mod-written rotation:** clear A or D and turn the wagon from the mod. This is the most control and the most risk,
   because the link may fight it.

The fallbacks slow the angle's growth but don't stop it, so with A or D held they still end in the same circling.

Still owed: with `Logging=0`, check that the controls work and that launching leaves no `HorseSteer.log` or
`HorseSteer.csv` in `bin64`. Whenever another vehicle can be taken, such as a cow-drawn wagon (`cd_m0002_00_wagoncow`),
the carriage (`cd_r0012_00_carriage`) or the bomb cart (`cd_r0011_00_bombcart`), drive it with `Logging=1` and check
`mount package` in the log. No package covers the bomb cart, so the mod should stay inactive on it.

If another save is used, its mount keys and available mounts may differ, and a stale `bin64\dmm\manifest.tsv` is the
first thing to check if it won't load. The freeze after a won minigame is accepted, with a remount as the workaround, and
no further work is planned on it (evidence in `findings/stability.md`). `kStateWalk` and the other state constants stay as
low pointer bits for the horse and ibex: a missing `mount_value` catches most lost walks, but not `8FC`, whose value is 0,
and failure only disables that part of the recovery.

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
3. Investigate carriages, which should get similar controls if possible. The wagon steers forward and reverses with S
   through its brake key ([stage 15](horsesteer-tests/stage-15.md)). What's left is reverse steering without the animal
   folding against the wagon, and the other vehicles, both in "Resume here".
4. Before release, add a speed limit to the horse's state-ID recovery in `UpdateBackwardRecovery`, which pulses once a second when the horse is stopped against something. Keep the limit below the 0.8 to 0.91 m/s where it still fires usefully.
