# Stage 17 test

Searches the memory around the wagon and the animal pulling it for rotations, and logs each one's yaw while S reverses
the wagon, to find one that shows the animal's angle to the wagon. Nothing in the controls changed. The animal search now
prefers an actor with a mount package, so it picks the horse instead of the driver.

## Setup

1. Copy `bin64\HorseSteer.log` and `bin64\HorseSteer.csv` to `scratch/probe-runs/` if they're still there.
2. Close the game. Check that `bin64\HorseSteer.ini` has `Logging=1` under `[Debug]`. In DMM, turn `HorseSteer` off and
   on again and apply.
3. Launch and wait until at least 25 seconds after launch.

## Steps

On a wagon, on open flat ground. Taking a wagon can set the animal running, so let it come to a stop first.

1. **Mount, stop, and wait 5 seconds** without pressing anything. The scan runs 3 seconds after mounting, and may
   freeze the game for a moment.
2. **Drive to open, flat ground** away from NPCs and obstacles, and stop.
3. **Hold A and D together for 1 to 2 seconds** without W or S, as a marker that testing starts here.
4. **Hold S for 3 seconds.** Keep holding S through step 6.
5. **Tap A for about 0.2 s, then wait 2 s.** Do this 3 times, then the same with D.
6. **Hold A for 2 seconds**, then release A and S.

The rotation log is only written while S reverses the wagon, so it can include braking on the drive in step 2. The
marker in step 3 shows in `HorseSteer.csv` as rows with both `a` and `d` at 1, and its `tick_ms` lines up with the
rotation log's. Close the game and leave the logs in place until they've
been read.

## Logs

`HorseSteer.log` adds a `rotation scan` line with the wagon's and the animal's physics objects, `(same object)` if
they're one object, and the number of objects read and rotations found.

`bin64\HorseSteer_rot_0.txt` lists each rotation found: its column, its layout (`rows16`, `rows12` or `quat`), its
address, its yaw at the time of the scan, and the chain that reached it, such as `mountphys+0x88->+0x60`.
`HorseSteer_rot_0.csv` has a row about 10 times a second while S reverses the wagon, with `tick_ms`, `mount_phys_yaw`
and each rotation's yaw. A column is empty where it no longer reads as a rotation. Each new scan, after a remount, writes
the next number. Launching deletes the files from earlier runs.

## Stage 17 results

| 1. Scan | 4. S straight | 5. Taps | 6. A held |
| --- | --- | --- | --- |
| Ran, stopped at both limits | Logged | 3 A and 3 D taps logged | Logged, then D held for about 8 s |

- Any crash or freeze? no
- Notes: the game was still open when the logs were copied, so the last row of the rotation CSV was cut off. Logs are in
  `scratch/probe-runs/stage17/`. Marker at `tick_ms` 3693312.

From the logs:

- **Animal:** the search chose the horse (`mount package`), key = wagon + 2 again. Its physics object
  (`…AE2BED80`) is a separate object from the wagon's (`…9FD15000`), but holds the same transform.
- **Scan:** 30,000 objects and 8,000 rotations, both at their limits. After the marker, 4,691 columns were mostly empty
  (memory freed or reused), 3,194 never moved, 18 matched `mount_phys_yaw` exactly, and 97 moved otherwise.
- **No separate body angle:** every rotation that moved during the taps followed `mount_phys_yaw`, either as a copy,
  as the same heading in another convention (a fixed offset or mirror), or one frame behind (r897, r999, ±1 to 3°
  during a turn, back to 0 when it settles). None settled at a new angle to the wagon after a tap. A few columns only
  changed when a new S press started, holding the heading from that moment.
