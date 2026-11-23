# HorseSteer tests

What HorseSteer's tests add to the gibbon plugin's test template. Name each new test file `stage-N.md`, after the last
stage, and its `tests.json` id `stage-N`.

## Writing steps

Holding W only keeps the mount at its current gait, and each Shift tap raises it one gait. A step that expects the mount
to speed up again after a slowdown must say to tap Shift, not to hold W. Name gaits as the tester does:

| Gait     | Horse state | Horse `mount_value` |
| -------- | ----------- | ------------------- |
| Walk     | `A56E`      | 2.8                 |
| Trot     | `B4A7`      | 5.2                 |
| Canter   | `C2B2`      | 9.5                 |
| Gallop 1 | `D099`      | 12                  |
| Gallop 2 | `E38E`      | 14                  |

Holding Shift past gallop 2 is a mount skill that goes faster still for a while.

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

After stage 28, the CSV's `brake_assist` column and the `ini` line's `assist per second` are gone.

From stage 22, the `ini` and `marker` lines add `release on speed`, `brake gap start` adds `target`, `brake gap end` can have the reason `speed`, and the log adds `gait value` lines.

From stage 23, `[BrakeTest]`, Insert, the CSV's `marker` column and the log's `marker` and `gait value` lines are gone. The `ini` line drops `brake test` and `release on speed`, and `brake gap start` drops `target`.

From stage 24, the log adds `brake key press` lines, and `brake gap start` adds `source`.

From stage 37, the log adds a `lookup physicsSlot` line, a failed lookup reads `FAILED (required)` or
`not found (optional)`, and setup ends with a `MOD ACTIVE` or `MOD INACTIVE` line. `rider_sync_yaw` and `mount_sync_yaw`
read 0.00 on every row, in stage 36 too, so don't use them.

From stage 30, the `ini` line adds `deadzone`. From stage 31, it adds `camera detached` and `tap brake`. From stage 32, it adds `reload key`, and the log adds a `reloading settings` line before each `ini` line after the first. From stage 33, the log adds `suspended by` and `resumed by` lines with the owner and the number of owners holding. From stage 34, the `ini` line adds `key ramp`.

From stage 38, the log adds `lookup cachedConditionSlot`, `mount condition 0xNN first read` lines and `T10` lines with
the mount's `T10` answer, and the CSV ends with `t10`. From stage 39, setup logs `no input answer installed`, and `T10`
lines read `T10 R game G`, the mod's answer and the game's. From stage 44, the log adds `step down start` and `step down
end` lines, and `brake gap start` can have `source=no step`. From stage 45, it adds `hold input end` lines. The `gap
read` lines of stages 40 to 42 and the `T167` lines of stage 43 are gone after stage 45.

