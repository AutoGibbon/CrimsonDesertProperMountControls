# Crimson Desert: horse-relative controls

## Problem

All riding movement is camera-relative: forward input moves the horse towards the camera heading, and left/right input
moves it sideways relative to the camera instead of steering it.

## Objective

Replicate Red Dead Redemption 2's horse-relative controls, through a native ASI mod.

## Requirements

- **Forward/back:** W moves the horse along its own facing. S from standing walks the horse backward, as in RDR2. S is not a brake or a drift trigger: the tester dropped both, because players press
  S while riding to turn, which would start unwanted drifts, and it makes escaping a drift harder.
- **Steering:** A and D change the horse's heading while held. With no other input the horse walks in a small radius,
  and releasing W while steering at speed gives a tighter turn. The user wants to keep both behaviours.
- **Pivot:** S with A or D turns the horse nearly on the spot, at about 52°/s and 0.5 m/s, and the user wants to keep it.
- **Camera:** looking around freely never changes the horse's course.
- **Input:** keyboard and mouse. Controller support is optional for now.
- **Mounts:** a public release needs every legged ground mount to work. Carriages should get similar controls if possible. The exosuit/mech suit is excluded.

## Done when

- All requirements have been tested in game on the current build.
- The hook points and evidence are documented, with verified findings kept separate from hypotheses.

## Approach

`src/HorseSteer` hooks `ClientInputActorComponent`'s per-frame update while the player is mounted, and replaces its copied
camera forward and right vectors with ones aligned to the mount. The game's own angle calculation then resolves WASD
against the mount's facing, so the camera stays free. Test status is in `docs/horsesteer-test.md`.

| Unknown | Status |
| --- | --- |
| Forward, steering and free look | Working at walk, trot and gallop, tested in game |
| Brake on S | Dropped and removed after stage 5. Alt still brakes and drifts as in the base game |
| Backward walk | Working in game on the horse, in v0.0.1 (stage 6). The ASI moves the horse backward through its physics position, which plays the game's own blended backward walk. Other mounts are next. See `backward-walk-idea.md` |
| Drift recovery | Not a confirmed problem. The tester saw rougher recovery in stage 5 and suspects needing S to start the drift made the escape inputs harder. Removing S as the trigger should remove that factor; recheck only if it's still noticed with Alt |
| Taming minigame | Working in game: the mod pauses while the rodeo UI's frame slot is being called. Known issue: the horse ignores input after a win until the rider dismounts and remounts |
| Mount check | The action package name at `[[[[mount+0x68]+0x40]+0x28]+0x18]+0x48` must be in the land-mount list in `HorseSteer.cpp`. Working on the horse in game; other mounts are untested and use their own charts. `CD_M0003_CucuBird` is included; it has gliding states that are untested |
| Game camera follow and lookahead on | Not tested; the game hit a GPU reset when they were turned back on |

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

### From HorseSteer test runs

Logs are in `scratch/probe-runs/steer1/` and `steer2/`.

- **Result:** during a W hold the camera yaw swung 79° while the horse's heading stayed at 57°. With the mod off, the horse
  turned towards the camera before moving off.
- **Forward:** the horse travels 180° from its physics Z axis, so its forward is minus Z.
- **Mount:** this save's horse key is `0xB0100002`, and the second input component belongs to the horse.
- **Gaits:** trot and gallop steer as expected. Releasing W while steering gives a tighter turn.
- **Turn rates at walking speed:** about 34°/s for W with A or D, and about 66°/s for A alone.
- **Settings crash (14:47):** `user_engine_option_save.xml` was written at 14:47:24, the Nvidia driver logged GPU engine
  resets (`nvlddmkm` event 153) at 14:47:26 and 14:47:30, and the game then stopped with `DXGI_ERROR_DEVICE_HUNG`. The horse
  had been idle for over 2 minutes. HorseSteer's hook is CPU-only, so it's an unlikely cause, but it was active.
- **Camera settings:** with follow and lookahead on from launch, the game starts and runs normally, so the GPU reset is assumed
  not to be caused by the mod.
- **Mount scan:** the horse's action package name is `CD_R0001_Horse`. Its appearance path is at a different chain from the protagonists'.
- **S and steering rates:** S coasts from 6–9 m/s to a stop in about 2.5–4 s. W with A or D turns at 16–30°/s when setting off and
  37–42°/s at 6–10 m/s. A or D alone turns at 60–66°/s from standing and 83–90°/s at 4 m/s.
- **Turn rate and stick angle (stage 3, `scratch/probe-runs/stage3/`):** the rate follows the angle and levels off near 60°/s. At
  under 2.5 m/s, 45° gives about 31°/s and 80° about 57°/s. At 5 m/s and up, 45° gives about 43–50°/s and 55° about 53–59°/s. It also
  follows below 45°: with `kSteerRampMs` at 300, at 2.5 m/s and up the rate is about 16°/s at 0.1 s, 36°/s at 0.2 s and 58°/s at
  0.3 s after a steer press (stage 4, `scratch/probe-runs/stage4/`).
- **Stuck pivot (stage 3):** at the 80° low-speed angle, pressing W and A or D at the same moment can leave the horse turning at
  about 1 m/s or less for as long as the keys are held. Pressing either key first sets off normally, including A or D first, so a
  steep angle alone isn't the cause. The chart cause is untraced. The steering ramp prevents it: all 7 simultaneous starts logged in
  stage 4 reached walking speed within 2 s.
- **`key_brake`:** bound to Alt, mouse X2 and gamepad B, in the same `CharacterAction` group that on-foot charts read, so S can't be
  added to it in `inputmap.xml`. The game's text describes it as "Drift ... while dashing". Most horse uses are `OR T10 key_brake`,
  and `T10` is probably "no stick input", which fits the tester finding that Alt at normal gaits feels the same as releasing W. The
  slide stop, `gallop_f_slip_end`, is only entered from the drift-end and `faststart` clips and two special gallop states. In those
  gallop states, the `T10` branch is tested before `key_brake`, so braking needs a stick input. Drift starts from the slide stop, with
  `key_brake`, a steering angle past 45° and `SKILL_USABLE(12007)`. These skill IDs have not been matched to names or unlock rules.
  The tester reports that brake with A or D drifts in game, quickly slowing the horse and turning it to that side.
- **`key_brake` on other mounts:** of the land-mount ride charts, only the wolf and warthog read it, in one walk branch each,
  `OR T10 key_brake`. The bear, camel, reindeer, elephant, iguana, Carma bird and cucu bird ride charts don't read it.
- **Key check:** `exe+0x3B0150` is the handler for condition type `0xEA`. The handler table comes from `exe+0x3A9FC0`, at index
  `type - 0x83`. The handler reads the key hash from the condition at `+8` and the actor from the context at `+8`. `key_brake`'s
  hash is `0x377B9DB8`. Its live code starts with a jump to a non-module address (`0x13FFD003C` in the logged runs), so something
  hooks it at runtime. It returns 0 when the key is pressed: the game's answer was 1 except during brief presses, and answering 0
  on S started drifts in game. Replacing the table slot worked alongside the existing hook. That code was removed with the brake and is saved in
  `scratch/probe-runs/stage5-brake/s-brake.patch`.
- **Stage 5 test chart crash (`scratch/probe-runs/stage5-backward/`):** the game crashed during loading on the only run with
  `HorseSteer Backward Walk Test` on, with the same `HorseSteer` build that loaded in part 1. It read a null vtable on the main thread
  at `exe+0x15D3E2A`, calling through `[rdi+0x160]`, which held the static object `exe+0x6E87008`. No mod is on the logged stack.
  In stage 5b the same 7 edits loaded and played without a crash as a JSON byte patch, and the built chart differed from the original
  by only those bytes, so the edits aren't the cause. Whether the loose file caused the crash or it was intermittent is unknown.
  DMM's support bundle, extracted to `support/` there, shows the chart in its manifest, and it's the only `actionchart/bin__` file in
  that manifest, so no other enabled mod shows that DMM can deliver a binary chart. `DmmRuntime.log` stops at 23:04:49, 25 s before the
  crash, and doesn't list the chart, so it doesn't show whether the game read it. The dump doesn't include the object at `rdi`.
  In stage 5b the chart was served 33 s after the same textures that ended the crash run's log, and the crash came 25 s after them, so
  the game may not have read the chart before crashing. CDMW's research notes report edited charts in DMM group folders loading and
  working on this exe with DMM 1.0, which also counts against the loose file.
- **Stage 5 S brake (`scratch/probe-runs/stage5-brake/`):** speeds are noisy, and each comparison below has few samples.
  - **S alone:** 8.1 m/s to under 1 m/s in 2.3 s, against 2.3 s from 7.1 m/s for one clean W release. From 13.2 m/s, it reached
    1.4 m/s in 3.2 s. The tester saw no difference from releasing W, and no slide without steering.
  - **S with A or D:** at 13–15 m/s the horse turned at 110–200°/s on average during the press, and the tester saw a drift. At
    11–12 m/s it turned at 70–85°/s with no drift.
  - **Standing:** S does nothing, and S with D pivoted at about 50°/s and 0.4 m/s.
  - **Drift recovery:** many drifts fall to about 0 m/s within 2 s, then reach about 10 m/s by 6 s, even with W held. Speed fell
    below 3 m/s within 5 s in 6 of 17 Alt drifts with the mod on and 1 of 7 with it off, with W held about as long in both. It also
    did in 6 of 8 S drifts, but W was held for less of those. The cause is untraced.
- **Taming minigame:** it needs input pointing at the mount's tail, and it reads the movement data the mod edits: holding S wins it
  with ease while backward input reaches the game. The package name can't separate wild horses from tamed ones.
- **Rodeo signal:** slot 47 of `UIGamePlayControlRootMiniGameRodeo`'s vtable (`exe+0x5A6CBF8`) ran about 70 times a second during
  each minigame logged, and was silent otherwise. Slot 17 fired at each open and slot 19 at each close. Slot 47 starts 0.3–0.65 s after
  mounting.
- **Freeze after a win:** the horse ignores input after a won minigame until a remount. It happens with both input hooks installed,
  even when they write nothing and without the rodeo stub, and doesn't happen with the hooks left out or `HorseSteer` disabled. Which
  hook causes it, and how, is unknown. No branch in the exe lands inside either hook's patched bytes. Logs are in
  `scratch/probe-runs/` (`f5-off-win`, `no-stub-win`, `no-hooks-win`).
- **Probe freeze (13:57):** Windows logged it as an application hang (`AppHangB1`), not a crash.
- **GPU crash while alt-tabbing (stage 5h, `scratch/probe-runs/stage5h/`):** a device removed error (`887A0006`) inside D3D12, with `renodx` and the Nvidia driver on the
  stack and no mod code. DMM reported a GPU hang or driver loss. The tester then removed OptiScaler, and no crash dump has been written since, through stage 6.
- **Stowing under a carriage:** riding the underside of a carriage counts as mounted, with key `0xB01004A7`. `ReadPackageName` returned unreadable text there, so the mod
  stayed inactive, which is correct. That chain doesn't always lead to a name, which matters for carriage work.

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
