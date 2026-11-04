# HorseProbe

`src/HorseProbe` is a finished diagnostic, kept as a template for watches and scans. It logs riding data and finds the WASD input
value in memory, without changing game memory or controls. Its conclusions are in [findings/](../findings/).

Copy `bin64\HorseProbe.log` and `bin64\HorseProbe.csv` to `scratch/` before the next launch or disabling it in DMM: a launch overwrites
them and disabling deletes them.

## Running it

Enable it in DMM, mount on flat, open ground, and wait until at least 20 seconds after launch. It ignores F8 and F10 until then.

- **Heading log:** always on. It's useful to hold W with the mouse still, hold W and swing the camera, rotate the camera when stopped, then
  hold each of A, D and S from a standstill, for about 10 seconds each.
- **F8 input scan:** keep off the mouse, because moving the camera can discard the right value. Press F8, then hold W, D, S and A in turn,
  one key at a time, until each beeps. Wait for three rising beeps. Press F8 again to restart.
- **F10 write watch:** it could crash the game, so do it last and don't save afterwards. Press F10, then tap W, A, S and D in turn until two
  quick beeps about 5 seconds later. Each press checks the next four scan results, up to four presses. It needs a finished F8 scan.

## Reading the log

`HorseProbe loaded` with PE timestamp `0x6AE6E3E8`, then `ready`, means the offsets match the build. `telemetry:` lines report broken chains.

**CSV:** about 10 rows per second, paused during scans. Yaw is in degrees, as `atan2(x, z)`, and `-9999` means unreadable.

- `w`, `a`, `s`, `d`: physical key state, even if the game isn't focused.
- `cam_yaw`: camera forward vector, from block `+0x90`.
- `player_phys_yaw`, `horse_phys_yaw`: physics Z axis row at `+0x80`.
- `player_sync_yaw`, `horse_sync_yaw`: TransformSync quaternion at `+0xA4`, assumed to be x, y, z, w.
- `horse_x`, `horse_y`, `horse_z`, `rider_horse_dist`: physics position plus chunk origin, and metres from the rider.
- `speed`, `travel_yaw`: from the horse's position between rows, with jumps at chunk origin changes. `travel_yaw` needs over 0.1 m/s.

`kHorseKey` is `0xB0100003`, from build 2976, which isn't this save's horse (see [mounts-and-vehicles.md](../findings/mounts-and-vehicles.md)),
so update it for the horse columns to work.

**Input scan:**

- `scan W`, `scan D`, `scan S`, `scan A`, `scan neutral`: candidates left after each pass. On 0, find the pass where it dropped.
- `result N: ADDRESS W=(x,y) D=(x,y)`: a surviving address, for the first 256. `W=(0,1)` with `D=(1,0)` is the expected raw form, and other
  consistent signs or axes are fine. Several results are normal, and addresses are heap addresses that change each launch.
- It only covers private read/write memory and two adjacent floats that are axis-aligned while W is held, so it misses separate X and Y fields
  and vectors already rotated by the camera.

**Write watch:**

- `watch: slot N = ADDRESS`: the watched addresses.
- `debug registers armed on N threads, M failed`: failures mean some writers may be missed.
- `writer ADDRESS (exe+0x...) hits=N slots=0x...`: code that wrote a watched address, with `slots` a bitmask of which. The address is the
  instruction after the write, so read backwards. Hits near the frame rate suggest a per-frame update. A very large `exe+` value means
  code outside the exe. Code in `.xtext` or `.xtls` may be mutated by Denuvo, so disassemble it from live memory if the on-disk code doesn't
  make sense.

Other mods may also use hardware breakpoints, so check them if F10 crashes.

## Run 1

Logs are in `scratch/probe-runs/run1/`. In the unmodded game, the horse moved relative to the camera, tracking slowly to a new heading as the
camera swung. A, D and S from standing turned the horse to face that direction relative to the camera and walked it there, and it only turned on the
spot when not yet facing the wanted heading. F8 ran with no hitch. F10 froze the game (see [stability.md](../findings/stability.md)).

The published vtable addresses for `ClientActorManager`, `ClientChildOnlyInGameActor` and `ClientCharacterControlActorComponent` resolve through
RTTI in this exe. The published camera and physics ones don't, and the published camera slot's sentinel never matched in run 1, so the probe checks
the camera with its block sentinel instead.
