# Offsets and addresses

Steam build 26423880, PE timestamp `0x6AEA07A6`. All values are offsets from the image base unless shown as a field of a chain. What they
do is explained in the topic files.

`HorseSteer` finds the addresses below at startup and logs them. If a required lookup fails, it installs no hooks. `tools/check_lookups.py EXE...` runs the same lookups on saved exes and compares them with each build's known addresses in `KNOWN`. Change the tool and the mod together.

The lookups don't cover field offsets, apart from the ones in the signatures (`+0x154`, `+0x160`, `+0x48`), or the rodeo slot number. After a game update, check those by hand. If a lookup fails, check [CrimsonDesertCoop](https://github.com/macauleywilliams96-dotcom/CrimsonDesertCoop)
for new offsets, or port them with its `PORTING-3479.md` method using `scratch/exe-baseline/`. To re-derive the camera slot, use
`docs/ENGINE_CAMERA_RESEARCH.md` from CrimsonDesertTelemetry.

## Used by `HorseSteer` and working in game

[stage-10](../horsesteer-tests/stage-10.md) found all of these in live memory on this build, and tested them in game, including the rodeo slot.

| Variable in `HorseSteer.cpp` | Address | Lookup | What it is |
| --- | --- | --- | --- |
| `g_worldSystemSlot` | `0x71BDEF0` | Reference count, at least 3 times the next | WorldSystem global |
| `g_vtActorManager` | `0x5944B38` | RTTI | `ClientActorManager` vtable, checked before use |
| `g_vtVehicleComponent` | `0x5944B90` | RTTI | `ClientVehicleActorComponent` vtable |
| `g_hookSite` | `0xB6D526` | Signature | Input update, right after the camera vectors are copied |
| `g_stickSite` | `0xB6D626` | Signature | Input update, right before the raw stick is read from `+0x48` |
| `g_keyCheck` | `0x3B0750` | Signature, not required | Chart key condition handler, type `0xEA`. `InstallBrakeKey` checks the table slot holds it |
| `g_conditionTableCall` | `0x3AA79C` | Signature, not required | A call to the chart condition handler table's getter, `0x3AA5C0`, which is Denuvo-mutated. Found in [stage 15](../horsesteer-tests/stage-15.md) |
| `g_vtRodeoUi` | `0x5A75218` | RTTI | `UIGamePlayControlRootMiniGameRodeo` vtable; slot 47 runs each frame of the minigame |

```text
manager    = [[g_worldSystemSlot]+0x30]              rider = [manager+0x50]
components = [actor+0x68]
vehicle    = [components+0x48]                       mount key = [[vehicle+0x18]+0x10], seat = +0x14
control    = [components+0x40]
package    = [[[control+0x28]+0x18]+0x48]            name string
physics    = [[control+0x140]+0x2C0]                 Z axis +0x80, position +0x90; forward is minus Z
sync       = [components+0x1A0]                      rotation quaternion +0xFC
camera     = [[WorldSystem+0x18]+0x50]               the block the update copies from
```

`ClientInputActorComponent` fields: actor `+0x08`, stick `+0x38`, raw stick `+0x48`, move angle `+0x88`, relative move angle `+0x8C`, camera forward
`+0x154`, camera right `+0x160`.

## Traced in the exe

- **`ClientInputActorComponent`:** vtable `0x59564C0`, update at slot 33, `exe+0xB6D470`. Angle calculation at `exe+0xB6CDB0`.
- **Input state:** updated by `exe+0x30B3B50`, with the previous vector at `+0x38` and current at `+0x40`.
- **`RideOn::Enter`:** `exe+0x74FBB0`.
- **Chunk origin:** `exe+0x71AA150`. `HorseSteer` no longer reads it.
- **From other projects, not yet checked on this install:**
  - The image base is fixed at `0x140000000`, and part of the code is mutated by Denuvo, so some functions may only be
    readable in live memory (CrimsonDesertCoop).
  - Hardware breakpoints and bounded write watches have worked in this game from a probe (CrimsonDesertTelemetry). Whether
    an attached x64dbg also works is unknown.

## From Horse Tap Brake 1.0.2

A published ASI that makes a brake tap drop one gait. It was analysed statically and never loaded, and nothing here has been tested in game with `HorseSteer`. Its files and disassembly are in
`scratch/mods/HorseTapBrake-1.0.2/`. `tools/apply_htb.py ASI EXE` repeats its lookups on an exe and prints its key query signature, and `tools/asi_dis.py` disassembles an ASI. Its key query and
key hash are in [charts-and-braking.md](charts-and-braking.md).

Checked against this build's exe:

- **Vtable:** RTTI finds `NormalMovementMethod`'s vtable at `exe+0x5958C10`. Slots 4, 5 and 6 are all the empty stub `exe+0x3A3450`.
- **World global:** counting references to `mov rax,[rip+x]; mov rcx,[rax+0x30]` in code gives `g_worldSystemSlot` with 204 references, against 21 for the next.
- **Key query:** the signature has one match, `exe+0x30CAFD0`.
- **Chart runtime path:** the code at `exe+0xBC6B38` reads `control = [[actor+0x68]+0x40]`, `index = byte [[control+0x30]+0x1B]` and `runtime = [[control+index*8+0x88]+0x18]`.

Logged on the mount in `HorseSteer` in [stage 8](../horsesteer-tests/stage-8.md), and readable on both horses it rode:

- **Chart state:** `state = [runtime+0x28]`. If byte `[state+0x7A]` is 1–32, `value` is the float at `[runtime+0x38] + word [state+0x78] + 0x18`. How a pointer maps to a node number is unknown, and both horses showed
  the same pointers. `state` holds through a steady gait, with short excursions to other states. `value` is constant within a state and looks like a target speed, though what it means is unconfirmed. It's missing in idle states.
  `value == 0` is not only braking, as Horse Tap Brake treats it: it also appears in the backward walk, and in states that held with W and a steering key down, at a standstill and at a gallop.

  | Gait | `state`, last 5 hex digits | `value` | Median velocity, m/s |
  | --- | --- | --- | --- |
  | Walk | `3A56E` | 2.8 | 1.6 |
  | Trot | `3B4A7` | 5.2 | 3.6 |
  | Canter | `3C2B2` | 9.5 | 6.7 |
  | Gallop | `3D099` | 12 | 10.1 |
  | Sprint | `3E38E` | 14 | 11.9 |
  | Stop from a walk, and the backward walk | `3A73F`, then `3A8FC`, `3AAE5` | 0 | Falling, or 1.0 in the backward walk |

  The gait names are inferred from their speeds.
- **Velocity:** the horizontal size of the vector at `[[actor+0x68]+0x1A0]+0x188` (`x` at `+0x188`, `z` at `+0x190`) is the mount's speed in m/s. It correlates at 0.98 with speed from position, and it reads 0.000 at rest
  (p99), where position speed went over 0.2 in 33 of 990 rows. Its row-to-row change is 8 to 21 times smaller in a steady gait. It includes the backward walk's position writes, reading 1.0, and takes about a second to fall after
  they stop. Horse Tap Brake finds the horse by picking the fastest non-player movement object, while `HorseSteer` reads the mount directly.

From the mod only, unconfirmed:

- **Sector position:** the mod treats the rider as mounted when `+0xB4` and `+0xBC`, plus the 16-bit `+0xC0` and `+0xC2` values × 1000, are all within 20 of zero on the rider's `+0x1A0` object. This suggests a per-actor sector
  position, relative to the mount while riding.
- **Callback:** the mod replaces slots 5 and 6 to get a call for each movement object without patching code. It expects slot 5 to run at least every 500 ms while riding. What calls these slots isn't traced.
