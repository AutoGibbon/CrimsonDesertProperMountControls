# HorseProbe analysis and next steps

Use this after a [HorseProbe test](horseprobe-test.md) to interpret the results and choose the next step. The goal and
findings so far are in [discovery.md](discovery.md).

## Inputs

- `docs/horseprobe-results.md`: what the tester saw and heard.
- `Z:\SteamLibrary\steamapps\common\Crimson Desert\bin64\HorseProbe.log`: load status, scan results and write watch
  results.
- `bin64\HorseProbe.csv`: telemetry at about 10 rows per second, paused during scan passes.

The probe overwrites both files on each launch, and disabling it in DMM deletes them, so copy them to `scratch/` before
either.

## Telemetry columns

Yaw is in degrees, calculated as `atan2(x, z)`. `-9999` means the value could not be read.

| Column | Meaning |
| --- | --- |
| `tick_ms` | `GetTickCount64` |
| `w`, `a`, `s`, `d` | Physical key state, even if the game is not focused |
| `cam_yaw` | Camera forward vector, from block `+0x90` |
| `player_phys_yaw`, `horse_phys_yaw` | Physics object Z axis row, at `+0x80`, assumed to be forward |
| `player_sync_yaw`, `horse_sync_yaw` | TransformSync quaternion at `+0xA4`, assumed to be x, y, z, w |
| `horse_x`, `horse_y`, `horse_z` | Physics position plus chunk origin |
| `rider_horse_dist` | Metres between rider and horse; low while mounted |
| `speed`, `travel_yaw` | Calculated from the horse's position between rows; `travel_yaw` needs a speed above 0.1 m/s |

Offset jumps in `speed` can come from chunk origin changes. The horse is looked up with key `0xB0100003`, which comes
from captures on build 2976.

## Checks, in order

### 1. Probe status

The log should show `HorseProbe loaded` with PE timestamp `0x6AE6E3E8`, then `ready`. Other values mean the game
updated, so telemetry is off and the offsets must be ported.

`telemetry:` lines report broken chains. A camera failure can repeat on every row.

### 2. Telemetry

| Check | Steps to use | Expected |
| --- | --- | --- |
| Rider is on the horse | All | `rider_horse_dist` is low and steady |
| Horse heading is correct | Hold W, mouse still | `horse_*_yaw` is within about 10° of `travel_yaw`; pick the steadier column |
| Camera is independent | Stopped, rotate camera | `cam_yaw` changes and horse yaw doesn't |
| Problem is reproduced | Hold W, swing camera | Horse yaw follows `cam_yaw` |

A constant offset, such as 90° or 180°, means the forward axis assumption is wrong. Correct the axis before building on
it.

### 3. Turning and reversing

Use the A, D and S steps, along with the tester's description.

| Observation | Meaning |
| --- | --- |
| A/D: yaw changes with `speed` near 0 | The game can turn in place; try input only |
| A/D: horse only turns while travelling | Steering in place needs a yaw write or a change to the animation chart |
| S: `travel_yaw` is about 180° from a steady heading | Reverse can be reached |
| S: horse turns around, then walks away | Reverse needs another route to the `walk_b` clips |

### 4. Input scan

Log lines:

- `scan W`, `scan D`, `scan S`, `scan A`, `scan neutral`: candidates left after each pass.
- `result N: ADDRESS W=(x,y) D=(x,y)`: a surviving address and its values. Only the first 256 are logged.

`W=(0,1)` with `D=(1,0)` is the expected raw form. Other sign or axis combinations are fine if consistent. Several results
are normal, because the game may keep copies. Addresses are heap addresses and change each launch, so they can't be
hard-coded.

The scan only covers private read/write memory, and only two adjacent floats that are axis-aligned while W is held. It
won't find separate X and Y fields, or vectors already rotated by the camera.

### 5. Write watch

Log lines:

- `watch: slot N = ADDRESS`: the watched addresses.
- `debug registers armed on N threads, M failed`: failures mean some writers may be missed.
- `writer ADDRESS (exe+0x...) hits=N slots=0x...`: code that wrote a watched address. `slots` is a bitmask of the watched
  addresses it wrote.

The logged address is the instruction after the write, so read backwards to find the writer. Hits close to the frame rate
suggest a per-frame update. A very large `exe+` value means the code is outside the exe, probably in another mod or a
runtime DLL. Code in `.xtext` or `.xtls` may be mutated by Denuvo, so disassemble it from live memory if the on-disk code
doesn't make sense.

## Decisions

| Result | Next step |
| --- | --- |
| Game updated | Check [CrimsonDesertCoop](https://github.com/macauleywilliams96-dotcom/CrimsonDesertCoop) for new offsets, or port them with its `PORTING-3479.md` method using `scratch/exe-baseline/` |
| Horse telemetry missing | Rider `MoveData` holds the mount key; find the live key or horse actor, then re-test |
| Camera telemetry missing | Re-derive the camera slot with `docs/ENGINE_CAMERA_RESEARCH.md` from CrimsonDesertTelemetry |
| Heading doesn't match travel | Log the X and Y axes too and pick the right one |
| Scan: 0 results | Find where it dropped to 0, then widen it: separate X and Y floats, other memory types, or the `LeftStick*` chart variables |
| Scan: many results | Repeat the scan, keep addresses with the same values and pointer region, and use the write watch to group them |
| Good results, no exe writer | Watch the remaining results; if none help, trace from `exe+0x3B0150` or `ClientInputActorComponent` |
| F10 crashes | Check whether other mods use hardware breakpoints, then use Ghidra from `ClientInputActorComponent` and the `MoveAnalog` string instead |
| Good results and an exe writer | Build the minimal mod below |

Don't ask the tester to disable mods as a first step, because some of their saves may depend on them. Disable only mods
they confirm are safe, one at a time.

## Minimal mod

Hook the writer or the first reader with [SafetyHook](https://github.com/cursey/safetyhook), not a polling thread, so the
value is replaced between the game's write and its read.

For a desired world yaw `want` and camera yaw `cam`:

```text
angle = want - cam
x = sin(angle)
y = cos(angle)
```

Confirm the signs in game, as they depend on the axis order the scan finds.

Build and test one stage at a time. Each stage must be tested in game before the next one.

1. **W only:** `want` = horse yaw. While riding straight, swinging the camera must not change the horse's course. This
   tests the whole approach.
2. **W with A or D:** `want` = horse yaw plus or minus a turn offset. Tune the offset for the feel of RDR2.
3. **A or D alone:** turn without moving, using the method from check 3. This is the highest risk.
4. **S:** reverse, using the method from check 3.
5. **Mount check:** replace rider-to-horse distance with a real check for horse-type mounts, starting from
   `ClientVehicleActorComponent` and the `RideOnHorse` chart slot. Confirm which mounts share it, such as donkeys, and
   exclude the exosuit/mech suit.
6. **Updates:** replace fixed addresses with signature scans, and disable the mod safely if one fails.
7. **Compatibility:** test alongside the tester's usual mods. Controller support is optional, after this stage.

## Resuming work

- **Status:** `src/HorseSteer` is the working mod; the "Minimal mod" section above is superseded, because the mod replaces the
  camera vectors rather than the stick. Forward steering and free look work at walk, trot and gallop, S with A or D pivots, and S alone walks the horse
  backward.
- **Next:**
  1. Get every legged ground mount working, which a public release requires. v0.0.1 is the working horse prototype, and how its backward walk works is in
     `backward-walk-idea.md`. For each mount in `kGroundMountPackages`, check the controls and whether that technique applies, and confirm the list has every legged
     mount. This also decides how S should work on them: clearing backward input may block backsteps and backward walks on mounts such as the wolf and elephant, so limit
     that to the horse where the technique doesn't apply. The test save has only one basic mount, and the tester is working out how to reach others, so wait for that
     before continuing. Then work through these, starting with mounts whose data and mechanisms are closest to the horse's:
     - Confirm each listed package's ride chart and blend files. `character/binary/motionblending/horse/` also has `donkey_move`, so check how donkeys and other horse
       variants are covered. `tools/mount_census.py` lists the vehicle types and `Riding_` characters, and `discovery.md` has the open questions on the ibex and donkeys.
     - Look for backward clips in those blend files, and for a stop state like horse node 186, which holds while the mount keeps moving.
     - Look for existing backward moves that clearing S would block.
     - Sort the mounts into those where the horse method should work as it is, those needing changes, and those with no backward clips, then test in game.
  2. Investigate carriages, which should get similar controls if possible. This is expected to be separate work, with its own round of probe logging and code execution
     capture. They aren't in `kGroundMountPackages`, and the wagon has its own charts (`r0012_wagon_upper`, which reads `key_brake`, and `ride_carriage_lower` and
     `ride_carriage_upper`). The first question is whether replacing the camera vectors applies to them.
  3. Before any public release, find which input hook causes the freeze after a won minigame, by testing each alone, then try moving
     it. The tester accepts a remount until then.
  4. Replace fixed addresses with signature scans.
  5. Update `g_mountSpeed` separately from `WriteTelemetry`, which stops updating it if the CSV can't open or passes 500,000 rows. Forward steering and the backward walk
     both depend on it. Remove the CSV and other test logging for release.
  6. Retest the taming minigame on the current build. Stage 6 didn't reach it, because wild horses take a long time to find.
- **Chart decoding:** `scratch/analysis/chart_branches.py CDMW_SRC CHART_DIR` prints the horse chart's `key_brake` branches, the
  branches into the slide and drift states, and its skill conditions, with conditions decoded.
- **Steering tuning:** stage 4 passed in game, and the tester rated `kSteerRampMs`, `kTightSteerFactor` and `kWideSteerFactor`
  about right, so leave them as they are. To test a new build, turn `HorseSteer` off and on in DMM and apply, so that it's
  copied to `bin64`.
- **Probe:** `HorseProbe` and its docs are finished diagnostics. Its source stays as a template for watches and scans. The mount
  scan was removed from `HorseSteer`; its output is in `scratch/probe-runs/steer5/` and the last version with it is not committed.
- **HorseSteer deployment:** it needs a DMM off/on cycle after each new build. If a save won't load, check for a stale
  `bin64\dmm\manifest.tsv` before suspecting the mod, as described in `discovery.md`.
- **HorseSteer logs:** `bin64\HorseSteer.log` records the hook install, mount changes and which actor each input slot
  belongs to. `bin64\HorseSteer.csv` is written about 10 times a second while mounted. Slot `i0` is the rider's input
  component and `i1` is the horse's. With W held and the mod working, `mount_phys_yaw` and `travel_yaw` stay steady while
  `i0_camera_yaw` changes, and `travel_yaw` is about 180° from `mount_phys_yaw`. `i0_relative` tracks the mount's heading,
  not 0.
- **Static analysis:** `tools/disasm.py` reads the on-disk exe, and addresses in `.sxdata` disassemble cleanly there, so
  Ghidra has not been needed.
- **Not installed:** Cheat Engine, Ghidra and x64dbg. Ghidra 12.2 needs Java 25 and this machine has JRE 17, so add a
  portable JDK under `scratch/` if it's needed. Installing Cheat Engine needs the user's admin prompt:
  `choco install cheatengine -y`.
- **CDMW:** the tools use its source as a library. `scratch/cdmw-release/CDMW.exe` is downloaded but has never been run.
- **Requirements:** in `discovery.md`. Free look is mandatory, which rules out camera-only fixes.
- **Mods:** other mods stay enabled during tests unless the tester agrees otherwise.
- **Updates:** the game can update at any launch, and Steam's update setting doesn't prevent it. The current exe is saved
  at `scratch/exe-baseline/CrimsonDesert_3479.exe`.
- **Repo state:** check `git log` and `git status`.
- **Git-ignored local data:** `scratch/` is about 1 GB, and the agent manages it. Don't commit it or `build/`.
  - `exe-baseline/`: can't be recreated after a game update, so don't delete it.
  - `venv/`: Python 3.12 with `lz4 cryptography Pillow numpy capstone`. Run the tools with
    `scratch/venv/Scripts/python`.
  - `probe-runs/`: saved probe logs. Copy each test's logs here before they can be deleted or overwritten.
  - `caballo.md`: CrimsonDesertCoop's horse research (`docs/research/caballo-coop.md`), written for build 2976.
  - `cdmw-src/`: CDMW source at commit `585ac51`, with no `.git`. To recreate it, run
    `git clone https://github.com/Ratty123/CDMW-Full scratch/cdmw-src`.
  - `out/`: archive index (`index.tsv`), all action charts (`charts/`), English text (`loc_eng.json`) and other extracts.
    These can be regenerated with `tools/`.
- **Build and deploy:** these are the probe's commands. For the mod, replace `HorseProbe` with `HorseSteer` throughout:

  ```bash
  cmake -S src/HorseProbe -B build/HorseProbe -G "Visual Studio 17 2022" -A x64
  cmake --build build/HorseProbe --config Release
  cp build/HorseProbe/Release/HorseProbe.asi "E:/Tools/mods/crimson desert/DMM/mods/"
  ```

  The tester then enables it in DMM.
