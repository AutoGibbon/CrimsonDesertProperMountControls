# Stage 16 test

Finds the animal pulling the wagon and logs its angle to the wagon, for the reverse steering limiter. On the wagon, the
mod logs every actor near the wagon, picks the animal, and logs where the wagon and the animal point to each other. The
CSV adds the animal's yaw and its angle to the wagon. Nothing in the controls changed, except that the horse's reverse
steering no longer ramps in.

## Setup

1. Copy `bin64\HorseSteer.log` and `bin64\HorseSteer.csv` to `scratch/probe-runs/` if they're still there.
2. Close the game. Check that `bin64\HorseSteer.ini` has `Logging=1` under `[Debug]`. In DMM, turn `HorseSteer` off and
   on again and apply.
3. Launch and wait until at least 25 seconds after launch.

## Steps

On a wagon, on open flat ground:

1. **Mount and wait 5 seconds** without pressing anything.
2. **Hold S for 3 seconds**, then release.
3. **Hold S, and tap A for about 0.2 s, then wait 2 s.** Do this 3 times while still holding S, then release S. Repeat
   with D.
4. **Hold S, and hold A for 3 seconds**, then release both.
5. **Dismount, remount the same wagon and wait 5 seconds.**

Dismount, reload a save, and:

6. **Mount the wagon and wait 5 seconds.**

On the horse:

7. **From standing, hold S, then add A or D:** the backward walk should steer as soon as A or D is pressed.

Close the game and leave the logs in place until they've been read.

## Logs

`HorseSteer.log` adds, each time the wagon is mounted:

- `near brake mount` for each actor within 12 m of the wagon, with its key, package, distance, offset ahead of and to
  the side of the wagon, height, yaw relative to the wagon, vehicle link key and seat, and appearance.
- `animal search` with the chosen actor and whether it was chosen because its vehicle link names the wagon, or because
  it's the nearest. The search repeats every 2 seconds until it finds one.
- `link mount to animal` and `link animal to mount` for each place in the first 0x400 bytes of one actor, or of an
  object it points to there, that holds the other's address or key. `+0x68->+0x40` means at `+0x40` in the object that
  `+0x68` points to.
- `animal ... lost` if the chosen actor stops being readable.

The CSV ends with `animal_key`, `animal_phys_yaw`, `animal_angle` (the animal's yaw minus the wagon's, from −180 to
180) and `animal_distance` (from the wagon's position to the animal's, flat), with `-9999` when there's no animal.

## Stage 16 results

| 1. Search | 2. S straight | 3. S with taps | 4. S with A held | 5. Remount | 6. Reload | 7. Horse reverse steering |
| --- | --- | --- | --- | --- | --- | --- |
| Animal found, but its physics reads as the wagon's | Straight, 1.0 m/s | 3 taps of A, 9 to 18° each, then straight again | Circled at 62°/s | Same animal | Same layout on a second wagon | Steers from the first row |

- Any crash or freeze? no
- Notes: n/a. Stealing a wagon sometimes sets the animal sprinting, so some rows are messy. The tester sent the logs
  without filling in the table. Logs are in `scratch/probe-runs/stage16/`.

From the logs:

- **The animal** is a `cd_r0001_horse` actor whose key is the wagon's plus 2 (`B0100228` to `B010022A`, and `B0100AFE`
  to `B0100B00`). The driver NPC is the wagon's key plus 1, with a vehicle link to the wagon at seat 0, so the first
  search picked the driver. When the player took the reins, the driver's actor became unreadable (`animal ... lost`) and
  the next search picked the horse as the nearest. The horse's own vehicle link is empty.
- **Links:** the horse holds the wagon's key at `+0xAC`, and the driver does too. The wagon's `+0xB0` points to a list
  that holds the driver's key at `+0x0` and the horse's at `+0x4`, followed by actor pointers at offsets that differed
  between the two wagons.
- **The horse's physics object reads exactly as the wagon's:** distance 0.00 and `animal_angle` 0.00 on every row. Either
  the chain reaches the same object, or the horse is carried on the wagon's transform. Its visible angle to the wagon
  doesn't come from that physics object.
- **`mount_sync_yaw`** reads 0.00 on every row, on the horse as well, so `ReadSyncYaw` doesn't work on this build.
- **Wagon taps (step 3):** A held for 0.2 to 0.3 s turned the wagon 15° and 18°, and a shorter tap 9°. The rate ramps up
  to 60°/s over about 0.3 s and back to 0 over about 0.3 s after release. Speed dips to about 0.8 m/s and returns to
  1.0, and the wagon then reverses straight along its new heading. Only A was tapped.
- **A held (step 4):** 62°/s from 0.3 s after the press, with speed falling from 1.0 to 0.5 m/s over about 1 s.
- **Horse (step 7):** `i1_stick_x` is −0.56 or 0.56 on the first row after A or D, and the horse turns at about 63°/s.
