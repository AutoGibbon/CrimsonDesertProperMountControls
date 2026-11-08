# HorseSteer tests

Give each new test its own file, `stage-N.md`, with its steps and results, and add a row here.

## Logs

Tracing is off by default. Before a test, set `Logging=1` under `[Debug]` in `bin64\ProperHorseControls.ini`, or the test leaves no logs. DMM doesn't copy the ini from `DMM\mods`, so editing that copy has no effect. With `Logging=0`, `ProperHorseControls.log` only exists if the mod had a problem at startup, such as a failed lookup, and launching deletes the log and CSV from any earlier run. Up to stage 18 the files were named `HorseSteer.*`, which the notes below and the stage files still use.

`bin64\HorseSteer.log` records each address lookup, the hook install, mount changes and which actor each input slot belongs to. `bin64\HorseSteer.csv` is written about 10 times a
second while mounted. Slot `i0` is the rider's input component and `i1` is the horse's. With W held and the mod working, `mount_phys_yaw` and `travel_yaw` stay
steady while `i0_camera_yaw` changes, and `travel_yaw` is about 180° from `mount_phys_yaw`. `i0_relative` tracks the mount's heading, not 0. `mount_state` is a chart state pointer in hex, 0 if unreadable, and `-9999` marks a missing `mount_value`, velocity or `rider_y`. `rider_y` is the rider's height from its physics position. From stage 10, `speed`, `travel_yaw` and `rider_y` no longer include the chunk origin, so `rider_y` isn't world height, and `speed` can spike for one row if the origin moves. `HorseSteer.log` records each `UpdateBackwardRecovery` pulse with the state it started from and the speed.

From stage 11, the CSV ends with `mount_package`, `marker` and `backward_rate`. `HorseSteer.log` adds `mount package` and `mount identity` lines for each mount, `state` lines with the low 16 bits of `mount_state` at each change, and `marker` and `backward rate` lines. These lines carry `tick=`, which matches the CSV's `tick_ms`. A `?` in `mount identity` means that name couldn't be read.

From stage 11b, `bin64\HorseSteer_scan_<key>a.txt` and `...b.txt` hold each mount's scan, 5 and 30 seconds after mounting. Each line is a text or RTTI class name and the chain that reached it, such as `mount->+68->+40 inline+18`, with up to 3 chains per name. `HorseSteer.log` records `controls forced on` when End is pressed, and a `mount scan` line for each scan. A forced mount's `mount_package` still shows what `ReadPackageName` read.

From stage 12, the scan files and `mount identity` lines are gone. `HorseSteer.log` adds an `ini` line with the number of values read, a `mount appearance` line per mount, and a `backward release pulse` line at each release. From stage 12b, `backward lost pulse` lines replace the release pulse lines, each with the state it started from. From stage 13, the CSV's `marker` column and the log's `marker`, `backward rate` and `controls forced on` lines are gone.

From stage 19, the CSV's `flip` column and the log's `flip` lines are gone, and the `ini` line ends with the reverse steering `invert` and the `toggle key`. From stage 20, the `ini` line adds the number of missing settings written to the ini.

From stage 21, the CSV ends with `raw_stick_x` and `raw_stick_y`, the mount input component's stick before the mod edits it, then `shift`, `brake_gap` (1 during a stick gap), `brake_assist` (the speed in m/s the position write is opposing) and `marker` (Insert presses so far). The `ini` line adds `brake test` and `assist per second`, and the log adds `brake gap start`, `brake gap end` and `marker` lines with `tick=`. With `Logging=1`, Insert also reloads `[BrakeTest]` from the ini, unless Insert is the toggle key.

From stage 22, the `ini` and `marker` lines add `release on speed`, `brake gap start` adds `target`, `brake gap end` can have the reason `speed`, and the log adds `gait value` lines.

From stage 23, `[BrakeTest]`, Insert, the CSV's `marker` column and the log's `marker` and `gait value` lines are gone. The `ini` line drops `brake test` and `release on speed`, and `brake gap start` drops `target`.

From stage 24, the log adds `brake key press` lines, and `brake gap start` adds `source`.

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
| [stage-16](stage-16.md) | Finding the animal pulling the wagon and logging its angle, and the horse's reverse steering without the ramp | Animal found, but its physics reads as the wagon's, so no angle. Short taps in reverse turn the wagon 9 to 18° and then reverse straight. Horse steers at once |
| [stage-17](stage-17.md) | Rotations around the wagon and its animal, for the animal's angle to the wagon | No rotation in the scanned memory showed an angle between the animal and the wagon. Every one that moved followed the root heading |
| [stage-18](stage-18.md) | Horse's state recovery pulse limited to above 0.3 m/s | Passed. Contact jitter against objects can still pass the limit, unnoticed in play |
| [stage-19](stage-19.md) | Shift+F5 removed, `Toggle` key setting, `InvertSteer` on the horse, another mount and the wagon | Passed on the horse and wagon. The tester prefers `InvertSteer=1` |
| [stage-20](stage-20.md) | Missing ini settings added with defaults at startup | Passed, run with no ini: the mod wrote every setting |
| [stage-21](stage-21.md) | W+S raw stick, gait after a stick gap, and a position write against forward motion | W+S reads 0. On the horse, a stick gap dropped one gait in 48 of 48 taps. The position write reached at most 1.1 m/s, and only ran during the gap. On the bear and camel, taps from high gaits landed two or three gaits down |
| [stage-22](stage-22.md) | Ending a stick gap at the next lower gait's speed, on the bear and camel, with and without the position write | Bear: 12 of 12 taps landed one gait lower. Camel: 3.5 to 2.6 worked, but 9.0 landed on 3.5. The lowest gait still runs to the 3 s cap. Feet looked fine with the write |
| [stage-23](stage-23.md) | Tap and hold of S to slow every ground mount, the `[Slowdown]` setting, and the position write on longer drops | Passed on the horse, bear and wagon. No camel ride was logged |
| [stage-24](stage-24.md) | The game's brake key starting the slowdown, with drift kept | Passed on the horse, camel and wagon, and drift kept, except that 7 of 22 horse taps from 5.2 to 12 kept their gait when the chart bounced out of the slowing state |
| [stage-25](stage-25.md) | Brake key taps on the horse, needing two consecutive slowing states | Passed. 18 of 18 taps from 5.2 to 12 dropped one gait, 5 of them after a bounce |
| [taming-minigame](taming-minigame.md) | Controls with the mod paused | Controls normal. The horse won't move after a win until a remount, a known issue |
