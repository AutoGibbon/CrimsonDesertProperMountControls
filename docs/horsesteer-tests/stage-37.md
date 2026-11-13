# Stage 37 test

Game update `0x6AF5D198` (2026-11-13) broke two things. `conditionTableCall` found no match, because the condition type
count in its signature went from `0x286` to `0x28A`. The physics object moved from `+0x2C0` to `+0x2D0` in the object at
`[[[actor+0x68]+0x40]+0x140]`, which no lookup covered, so steering and the backward walk stopped working with every
lookup passing. The count is now a wildcard, and `LookupPhysicsSlot` finds the physics object's offset. This checks both
on the new build.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. Copy `build/HorseSteer/Release/ProperHorseControls.asi` to `DMM\mods\`. In DMM, turn `ProperHorseControls` off and on
   again and apply.
3. In `bin64\ProperHorseControls.ini`, set `Logging=1`.
4. Use the horse.

## Steps

1. Ride at a walk, trot and gallop with W, and with W and A or D, turning the camera away from the horse's heading.
2. Tap S with W held at a canter and a gallop.
3. Stop, then hold S alone.

Close the game and copy the log and CSV to `scratch/probe-runs/2026-11-13-physics-slot/`.

## Logs

- The lookups include `lookup physicsSlot: +0x2D0` and `lookup conditionTableCall: exe+0x3AE0CC`, and setup ends with
  `MOD ACTIVE: all features enabled.`
- `mount_phys_yaw` reads a heading on every mounted row, not `-9999`, and `i0_applied` or `i1_applied` is 1.
- `speed`, from the physics position, is close to `mount_vel`, from TransformSync. This shows `+0x80` and `+0x90` in the
  moved physics object are still the axis and position.
- With W alone, `travel_yaw` is about 180° from `mount_phys_yaw`. With S alone after a stop, it's within 60°.

## Stage 37 results

Passed on the horse. Logs are in `scratch/probe-runs/2026-11-13-physics-slot/`. The first run, with only the
`conditionTableCall` fix, is in `scratch/probe-runs/2026-11-13-update-verify/`: `mount_phys_yaw` was `-9999` and
`rider_phys_yaw` 0.00 on every row, and nothing was applied.

- **Lookups:** `physicsSlot` found `+0x2D0` with 15 loads and no other aligned offset, and every other lookup matched.
  The log ended setup with `MOD ACTIVE: all features enabled.`
- **Steering:** `mount_phys_yaw` was valid on all 1,271 mounted rows and the input was applied on 1,270. Rider and mount
  yaw agreed within about 2° while riding.
- **Physics fields:** `speed` tracked `mount_vel`, such as 9.90 against 9.52 and 4.19 against 4.16 m/s.
- **Backward walk:** with W alone, 617 of 618 moving rows travelled opposite `+Z`. With S alone, 105 of 166 moving rows
  travelled along `+Z`; the 31 opposite were the horse still slowing. The log has 4 backward recovery pulses.
- **Brake gaps:** three, at 9.4, 7.3 and 4.2 m/s, each ending on the next state. An S press at 13.2 m/s with A or D held
  started none, as designed.
- **Stutter:** the tester saw periodic stutter during the first run and got the same with the mod off, so it isn't the mod.
