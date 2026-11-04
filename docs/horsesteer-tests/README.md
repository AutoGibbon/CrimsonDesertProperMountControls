# HorseSteer tests

Give each new test its own file, `stage-N.md`, with its steps and results, and add a row here.

## Logs

`bin64\HorseSteer.log` records the hook install, mount changes and which actor each input slot belongs to. `bin64\HorseSteer.csv` is written about 10 times a
second while mounted. Slot `i0` is the rider's input component and `i1` is the horse's. With W held and the mod working, `mount_phys_yaw` and `travel_yaw` stay
steady while `i0_camera_yaw` changes, and `travel_yaw` is about 180° from `mount_phys_yaw`. `i0_relative` tracks the mount's heading, not 0.

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
| [taming-minigame](taming-minigame.md) | Controls with the mod paused | Controls normal. The horse won't move after a win until a remount, a known issue |
