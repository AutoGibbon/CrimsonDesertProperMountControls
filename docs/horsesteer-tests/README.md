# HorseSteer tests

Give each new test its own file, `stage-N.md`, with its steps and results, and add a row here.

## Logs

Tracing is off by default. Before a test, set `Logging=1` under `[Debug]` in `bin64\HorseSteer.ini`, or the test leaves no logs. DMM doesn't copy the ini from `DMM\mods`, so editing that copy has no effect. With `Logging=0`, `HorseSteer.log` only exists if the mod had a problem at startup, such as a failed lookup, and launching deletes the log and CSV from any earlier run.

`bin64\HorseSteer.log` records each address lookup, the hook install, mount changes and which actor each input slot belongs to. `bin64\HorseSteer.csv` is written about 10 times a
second while mounted. Slot `i0` is the rider's input component and `i1` is the horse's. With W held and the mod working, `mount_phys_yaw` and `travel_yaw` stay
steady while `i0_camera_yaw` changes, and `travel_yaw` is about 180° from `mount_phys_yaw`. `i0_relative` tracks the mount's heading, not 0. `mount_state` is a chart state pointer in hex, 0 if unreadable, and `-9999` marks a missing `mount_value`, velocity or `rider_y`. `rider_y` is the rider's height from its physics position. From stage 10, `speed`, `travel_yaw` and `rider_y` no longer include the chunk origin, so `rider_y` isn't world height, and `speed` can spike for one row if the origin moves. `HorseSteer.log` records each `UpdateBackwardRecovery` pulse with the state it started from and the speed.

From stage 11, the CSV ends with `mount_package`, `marker` and `backward_rate`. `HorseSteer.log` adds `mount package` and `mount identity` lines for each mount, `state` lines with the low 16 bits of `mount_state` at each change, and `marker` and `backward rate` lines. These lines carry `tick=`, which matches the CSV's `tick_ms`. A `?` in `mount identity` means that name couldn't be read.

From stage 11b, `bin64\HorseSteer_scan_<key>a.txt` and `...b.txt` hold each mount's scan, 5 and 30 seconds after mounting. Each line is a text or RTTI class name and the chain that reached it, such as `mount->+68->+40 inline+18`, with up to 3 chains per name. `HorseSteer.log` records `controls forced on` when End is pressed, and a `mount scan` line for each scan. A forced mount's `mount_package` still shows what `ReadPackageName` read.

From stage 12, the scan files and `mount identity` lines are gone. `HorseSteer.log` adds an `ini` line with the number of values read, a `mount appearance` line per mount, and a `backward release pulse` line at each release. From stage 12b, `backward lost pulse` lines replace the release pulse lines, each with the state it started from. From stage 13, the CSV's `marker` column and the log's `marker`, `backward rate` and `controls forced on` lines are gone.

## Tests

| Test | Checks | Result |
| --- | --- | --- |
| [stage-1](stage-1.md) | Forward steering and free look | Passed |
| [stage-2](stage-2.md) | S as a brake, horse-only check | Failed: the check switched the mod off, so the other steps weren't run |
| [stage-2b](stage-2b.md) | Steering restored, mount scan, S | Steering good. S felt no different from releasing W |
| [stage-3](stage-3.md) | Land-mount check, tighter steering | Passed. W with A or D at the same moment could stick in a pivot, and steering felt instant |
| [stage-4](stage-4.md) | Steering ramp | Passed |
| [stage-5](stage-5.md) | S brake, backward clips | S brake abandoned. The clip test crashed during loading |
| [stage-5b](stage-5b.md) | Clips through a JSON patch | Clips played, but the horse didn't move |
| [stage-5c](stage-5c.md) | Move speed data on the backward loop | No movement |
| [stage-5d](stage-5d.md) | Same, with a key held | No movement |
| [stage-5e](stage-5e.md) | Fixed patch field | No movement |
| [stage-5f](stage-5f.md) | Moving the horse from the mod | Smooth and stops at walls, but too fast for the clips |
| [stage-5g](stage-5g.md) | Slower movement, restored clip data | Good match on clip 2. Also found a backward walk that plays by itself |
| [stage-5h](stage-5h.md) | That backward walk without the test chart | Loops once started, but the timing window is small |
| [stage-5i](stage-5i.md) | Forward pulse to start the walk | Nearly always works from standing, not from a walk or trot |
| [stage-5j](stage-5j.md) | Slow down first, then start | Works from all gaits. Steering in reverse looked clunky |
| [stage-5k](stage-5k.md) | Later start, smaller reverse steering | Good |
| [stage-6](stage-6.md) | Final backward walk (v0.0.1) | Passed. Taming minigame and other mounts not tested |
| [stage-7](stage-7.md) | New addresses after the game patch | Passed. Taming minigame not tested |
| [stage-8](stage-8.md) | Logging the mount's chart state and velocity | Passed. Velocity is usable as speed, and the state during a freeze was logged |
| [stage-9](stage-9.md) | Speed from velocity | Passed, except that releasing A or D in reverse usually slid instead of returning to the walking animation. Velocity wasn't the cause (stage 9c) |
| [stage-9b](stage-9b.md) | Reverse steering release with position speed put back, on flat ground | Passed: 14 of 14 releases in the log returned to the walking animation |
| [stage-9c](stage-9c.md) | The same with velocity restored, to separate the speed change from the ground | Velocity isn't the cause: 15 of 15 releases returned to the walking animation. One state drop at a grade of about +0.5, with no release involved, lasted until S was released |
| [stage-9d](stage-9d.md) | Forward pulse when the chart state leaves the walk during the backward walk | Passed. Both state drops at about 1 m/s recovered with no forward movement. Another 19 pulses fired while the horse was stopped by a wall or object, with no problem noticed |
| [stage-10](stage-10.md) | Addresses found at startup, with riding, backward walk and taming | Passed. All lookups matched in live memory, and the rodeo switch worked. Of two tames, only the one with keys pressed before the UI closed moved afterward |
| [stage-10b](stage-10b.md) | A failed lookup leaves the mod idle, and the post-win freeze with no hooks | Passed: the log named the failed lookup, and the game played as unmodded. The freeze check was inconclusive: one tame moved, but the tester wasn't sure the keys were released in time |
| [stage-11](stage-11.md) | Controls and backward walk on each ground mount, with mount identity logging | Horse passed. Camel and reindeer untested: `ReadPackageName` failed on them, so the mount check left the mod inactive |
| [stage-11b](stage-11b.md) | Memory scan of each mount for a name that works, and the stage 11 steps with a key that forces the controls on | Passed on all 9 non-horse mounts reached, with End. Scans found the package at `+0x18`. Elephant and cucubird not reached |
| [stage-12](stage-12.md) | Mounts recognised without End, speeds from `HorseSteer.ini`, and a pulse when A or D is released in reverse | Recognition, the `.ini` and speeds passed on 13 mounts, with no lurch. Mounts still lost the backward walk and slid, so the pulse was replaced |
| [stage-12b](stage-12b.md) | Pulses when the mount's chart value goes missing during the backward walk | Passed on the camel, boar, cucubird, reindeer and horse: slides ended within 0.4 s, with no lurch, including at walls. Cucubird gliding also steers correctly |
| [stage-13](stage-13.md) | Controls and backward walk with the test keys removed | Passed on the horse and iguana |
| [stage-14](stage-14.md) | Controls and backward walk on wagons | Passed. Reverse steering was poor, as with the game's brake key |
| [stage-15](stage-15.md) | S as the wagon's brake reverse, reverse steering ramp | Brake reverse passed. The ramp delayed the horse's reverse steering by about 0.44 s, so it was removed |
| [taming-minigame](taming-minigame.md) | Controls with the mod paused | Controls normal. The horse won't move after a win until a remount, a known issue |
