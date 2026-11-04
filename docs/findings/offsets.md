# Offsets and addresses

Build 1.0.0.3479, PE timestamp `0x6AE6E3E8`. All values are offsets from the image base unless shown as a field of a chain. What they
do is explained in the topic files.

## Used by `HorseSteer` and working in game

| Name in `HorseSteer.cpp` | Address | What it is |
| --- | --- | --- |
| `kWorldSystemSlot` | `0x71B6C50` | WorldSystem global |
| `kChunkOrigin` | `0x71A2EB0` | Chunk origin |
| `kVtActorManager` | `0x593ECF0` | `ClientActorManager` vtable, checked before use |
| `kVtVehicleComponent` | `0x593E9D8` | `ClientVehicleActorComponent` vtable |
| `kHookSite` | `0xB6C4A6` | Input update, right after the camera vectors are copied |
| `kStickSite` | `0xB6C5A6` | Input update, right before the raw stick is read from `+0x48` |
| `kKeyCheck` | `0x3B0150` | Chart key condition handler, type `0xEA` |
| `kVtRodeoUi` | `0x5A6CBF8` | `UIGamePlayControlRootMiniGameRodeo` vtable; slot 0 is `0x11EF6B0`, slot 47 runs each frame of the minigame |

```text
manager    = [[kWorldSystemSlot]+0x30]               rider = [manager+0x50]
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

- **`ClientInputActorComponent`:** vtable `0x59504A0`, update at slot 33, `exe+0xB6C3F0`. Angle calculation at `exe+0xB6BD30`.
- **Input state:** updated by `exe+0x30AE1C0`, with the previous vector at `+0x38` and current at `+0x40`.
- **`RideOn::Enter`:** `exe+0x74F270`.
- **Published vtables:** `ClientActorManager`, `ClientChildOnlyInGameActor` and
  `ClientCharacterControlActorComponent` resolve through RTTI at their published addresses in this exe. The published
  camera and physics vtable addresses do not, so `HorseProbe` checks the camera with its block sentinel instead.
- **Failed telemetry:** the published camera slot's sentinel never matched in HorseProbe run 1.
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

- **Vtable:** RTTI finds `NormalMovementMethod`'s vtable at `exe+0x5952BF8`. Slots 4, 5 and 6 are all the empty stub `exe+0x3A2E50`.
- **World global:** counting references to `mov rax,[rip+x]; mov rcx,[rax+0x30]` in code gives `kWorldSystemSlot` with 203 references, against 21 for the next.
- **Chart runtime path:** the code at `exe+0xBC5AB8` reads `control = [[actor+0x68]+0x40]`, `index = byte [[control+0x30]+0x1B]` and `runtime = [[control+index*8+0x88]+0x18]`.

From the mod only, unconfirmed:

- **Chart state:** `state = [runtime+0x28]`. If byte `[state+0x7A]` is 1–32, `value` is the float at `[runtime+0x38] + word [state+0x78] + 0x18`. The mod treats a changed `state` as a state change and `value == 0` as a
  braking state. What `value` means, and how a pointer maps to a node number, are unknown.
- **Velocity:** the mod takes the horizontal size of the vector at `[[actor+0x68]+0x1A0]+0x188` (`x` at `+0x188`, `z` at `+0x190`) as the mount's speed, and picks the fastest non-player movement object as the horse.
- **Sector position:** the mod treats the rider as mounted when `+0xB4` and `+0xBC`, plus the 16-bit `+0xC0` and `+0xC2` values × 1000, are all within 20 of zero on the rider's `+0x1A0` object. This suggests a per-actor sector
  position, relative to the mount while riding.
- **Callback:** the mod replaces slots 5 and 6 to get a call for each movement object without patching code. It expects slot 5 to run at least every 500 ms while riding. What calls these slots isn't traced.
