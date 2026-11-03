# Crimson Desert: horse-relative controls

## Problem

All riding movement is camera-relative: forward input moves the horse towards the camera heading, and left/right input
moves it sideways relative to the camera instead of steering it.

## Objective

Replicate Red Dead Redemption 2's horse-relative controls, through a native ASI mod.

## Requirements

- **Forward/back:** W and S are the only inputs that change the horse's position, and they move it along its own facing.
- **Steering:** A and D change the horse's heading while held and do not move it sideways.
- **Camera:** looking around freely never changes the horse's course.
- **Input:** keyboard and mouse. Controller support is optional for now.
- **Mounts:** horses and ground mounts based on them. The exosuit/mech suit is excluded.

## Done when

- All requirements have been tested in game on the current build.
- The hook points and evidence are documented, with verified findings kept separate from hypotheses.

## Approach

`src/HorseSteer` hooks `ClientInputActorComponent`'s per-frame update while the player is mounted, and replaces its copied
camera forward and right vectors with ones aligned to the mount. The game's own angle calculation then resolves WASD
against the mount's facing, so the camera stays free. Test status is in `docs/horsesteer-test.md`.

| Unknown | Status |
| --- | --- |
| Whether movement uses these angles | Hypothesis; stage 1 test |
| Forward sign | Inferred from the angle code; stage 1 test, with Shift+F5 to flip it |
| Horse-type mount check | Not done; the mod currently applies to anything ridden |
| Turning without moving | Not solved; the horse does rotate in place before moving off when facing away from the input |
| Reversing | `walk_b` clips exist in the horse chart; how to reach them from input is unknown |
| Camera heading in telemetry | Not needed by the mod; `HorseSteer` logs it from the component's copied vector |

Turning without moving is the highest risk.

The game setting and camera preset routes are ruled out: both work by keeping the camera behind the horse, which breaks
free look.

## Findings

Build: game exe 1.0.0.3479, PE timestamp `0x6AE6E3E8`.

### Verified from game data

- **Input:** `ui/inputmap.xml` binds `MoveAnalog` to the left stick and WASD as a plain 2D vector, with no coordinate
  frame setting. Remapping input cannot produce horse-relative movement.
- **Horse chart:** `characteractionpackagedescription.xml` maps the `CD_R0001_Horse` `RideOnHorse` slot to
  `UpperAction/4_Riding/R0001_HorseRD2_MoveTest_03_Upper`. No other package has a `RideOnHorse` slot. Other mounts such
  as `m0002_ride_wolf_upper` have their own charts.
- **Rotate events:** the horse chart has 76 unique `Rotate` frame events. Of 2,911 across all charts, 2,905 share the
  same header bytes, and no value there selects between camera, input and character frames.
- **Horse camera presets:** `playercamerapreset.xml` defines `Player_Ride_Horse`, `_Fast_Run`, `_Dash`, `_Dash_Att`,
  `_Att_Thrust`, `_Att_R` and `_Att_L`, with `FollowYawSpeedRate` and `FollowStartTime` values. For example,
  `Player_Ride_Horse` uses 0.8 and 1. None of the horse presets sets `FollowCameraEnable`, while
  `Player_Weapon_Default_RunFast_Follow` sets it with a rate of 8 and a start time of 0.
- **Patch delivery:** DMM already merges a partial patch into this same file on this install
  (`playercamerapreset__cclite.xml`, from Heroine of Pywel).
- **Game setting:** the English strings include "Camera Auto-follow": "Turn the camera automatically to the direction
  the character is moving." The exe has a matching `_enableCameraFollowYawPitch` field. The saved options file doesn't
  store a value for it.

### Ruled out

- **`MoveControlOptionType` (`Classic`/`Standard`):** this appears in the horse chart, but patch 1.02.00 describes the
  setting as sprint input behaviour, not steering.

- **Existing mods:** the user searched Nexus and found none that do anything like this, and GitHub and web searches found
  none either.

### Hypotheses

- Camera-relative direction is computed in native code. This is inferred because the input map and chart data expose no
  frame setting; the code has not been traced.
- `Rotate` events set turn rates and angle limits while native code supplies the target direction. This is inferred from
  values such as 6.98 rad/s and 180.
- The `Player_Ride_Horse*` presets are active while riding. No chart stores those names as plain text, so this needs an
  in-game test.
- Exe strings `LeftStickCharacterForward`, `LeftStickCharacterRight` and `MoveDirectionYaw` are likely starting points
  for tracing the movement code.

- **Installed mods:** Combat Evolved swaps riding charts such as `ride_upper` and `ride_weapon_upper`, and its strings show
  no movement or steering code. It logs a game key check at `exe+0x3B0150` on this build, which may help with reading
  game input.
- **Published vtables:** `ClientActorManager`, `ClientChildOnlyInGameActor` and
  `ClientCharacterControlActorComponent` resolve through RTTI at their published addresses in this exe. The published
  camera and physics vtable addresses do not, so `HorseProbe` checks the camera with its block sentinel instead.

### From HorseProbe run 1 and disassembly

Logs are in `scratch/probe-runs/run1/`.

- **Update function:** `exe+0xB6C3F0` is slot 33 of the `ClientInputActorComponent` vtable (at `0x59504A0`, confirmed
  through RTTI) and looks up the `MoveAnalog` input by name.
- **Camera vectors:** it copies two vectors from `[[WorldSystem+0x18]+0x50]` into `+0x154` (forward) and `+0x160` (right).
- **Angle calculation:** `exe+0xB6BD30` builds `stick.x * right + stick.y * horizontal forward` and returns
  `atan2(-x, -z)`. The result goes in `+0x88`. A second call, with both vectors rotated by the inverse of the actor rotation
  at TransformSync `+0xFC`, goes in `+0x8C`.
- **Move input:** raw values are in an input state object, with the previous vector at `+0x38` and current at `+0x40`,
  updated by `exe+0x30AE1C0`. Each input component holds its own copy at `+0x38` and `+0x48`. The scan found two such
  components; what the second one belongs to is unknown.
- **Mount link:** rider `[[actor+0x68]+0x48]` is the `ClientVehicleActorComponent` (vtable `0x593E9D8`). Its `[+0x18]+0x10`
  is the mount key and `+0x14` the seat, as `RideOn::Enter` (`exe+0x74F270`) reads them.
- **Headings:** with the camera held still, the rider's physics Z axis yaw swung about 90° left for A, 90° right for D
  and 180° for S, and followed the horse while mounted.
- **Failed telemetry:** the published camera slot's sentinel never matched, and horse key `0xB0100003` was not found.
- **F10 freeze:** the game froze near the end of the write watch, during Esc and Alt+Tab. The cause is unknown, and a probe
  cause is not ruled out.
- **DMM:** a stale `bin64\dmm\manifest.tsv` pointing at a moved mods folder stops saves loading, with a missing
  `decorationparam_cclite_female.xml` error. A full mount rebuilds it. Disabling an ASI in DMM deletes its logs.

### From other projects, not yet checked on this install

- The image base is fixed at `0x140000000`, and part of the code is mutated by Denuvo, so some functions may only be
  readable in live memory (CrimsonDesertCoop).
- While riding, the horse's world transform travels in the rider's `MoveActorReq`, and the horse sends its own only about
  once per second (CrimsonDesertCoop, build 2976).
- Hardware breakpoints and bounded write watches have worked in this game from a probe (CrimsonDesertTelemetry). Whether
  an attached x64dbg also works is unknown.

## Resources

- [CDMW](https://github.com/Ratty123/CDMW-Full): archive and action chart decoding
- [CrimsonDesertCoop](https://github.com/macauleywilliams96-dotcom/CrimsonDesertCoop): research on this build, including
  `camera_reader`, `horse_diag` and `docs/research/caballo-coop.md`
- [CrimsonDesertTelemetry](https://github.com/ZappendusterFX/CrimsonDesertTelemetry): camera layout and write-watch
  tracing in `docs/ENGINE_CAMERA_RESEARCH.md`
- [CrimsonDesertTools](https://github.com/tkhquang/CrimsonDesertTools): maintained native mods and shared code
- [Ghidra](https://github.com/NationalSecurityAgency/ghidra), [x64dbg](https://github.com/x64dbg/x64dbg),
  [Cheat Engine](https://github.com/cheat-engine/cheat-engine), [SafetyHook](https://github.com/cursey/safetyhook)
