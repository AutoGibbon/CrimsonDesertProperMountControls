# Input, steering and camera

Build 1.0.0.3479. Addresses and chains are collected in [offsets.md](offsets.md).

## Verified from game data

- **Input:** `ui/inputmap.xml` binds `MoveAnalog` to the left stick and WASD as a plain 2D vector, with no coordinate
  frame setting. Remapping input cannot produce horse-relative movement.
- **Horse camera presets:** `playercamerapreset.xml` defines `Player_Ride_Horse`, `_Fast_Run`, `_Dash`, `_Dash_Att`,
  `_Att_Thrust`, `_Att_R` and `_Att_L`, with `FollowYawSpeedRate` and `FollowStartTime` values. For example,
  `Player_Ride_Horse` uses 0.8 and 1. None of the horse presets sets `FollowCameraEnable`, while
  `Player_Weapon_Default_RunFast_Follow` sets it with a rate of 8 and a start time of 0.
- **Patch delivery:** DMM already merges a partial patch into this same file on this install
  (`playercamerapreset__cclite.xml`, from Heroine of Pywel).
- **Game setting:** the English strings include "Camera Auto-follow": "Turn the camera automatically to the direction
  the character is moving." The exe has a matching `_enableCameraFollowYawPitch` field. The saved options file doesn't
  store a value for it.

## Ruled out

- **`MoveControlOptionType` (`Classic`/`Standard`):** this appears in the horse chart, but patch 1.02.00 describes the
  setting as sprint input behaviour, not steering.

## Hypotheses

- **Confirmed, see "Update function" and "Angle calculation" below:** camera-relative direction is computed in native code.
- The `Player_Ride_Horse*` presets are active while riding. No chart stores those names as plain text, so this needs an
  in-game test.
- Exe strings `LeftStickCharacterForward`, `LeftStickCharacterRight` and `MoveDirectionYaw` are likely starting points
  for tracing the movement code.

## From HorseProbe run 1 and disassembly

Logs are in `scratch/probe-runs/run1/`.

- **Update function:** `exe+0xB6C3F0` is slot 33 of the `ClientInputActorComponent` vtable (at `0x59504A0`, confirmed
  through RTTI) and looks up the `MoveAnalog` input by name.
- **Camera vectors:** it copies two vectors from `[[WorldSystem+0x18]+0x50]` into `+0x154` (forward) and `+0x160` (right).
- **Angle calculation:** `exe+0xB6BD30` builds `stick.x * right + stick.y * horizontal forward` and returns
  `atan2(-x, -z)`. The result goes in `+0x88`. A second call, with both vectors rotated by the inverse of the actor rotation
  at TransformSync `+0xFC`, goes in `+0x8C`.
- **Move input:** raw values are in an input state object, with the previous vector at `+0x38` and current at `+0x40`,
  updated by `exe+0x30AE1C0`. Each input component holds its own copy at `+0x38` and `+0x48`. The scan found two such
  components; the second belongs to the horse (see below).
- **Headings:** with the camera held still, the rider's physics Z axis yaw swung about 90° left for A, 90° right for D
  and 180° for S, and followed the horse while mounted.

## From HorseSteer test runs

Logs are in `scratch/probe-runs/steer1/` and `steer2/`.

- **Result:** during a W hold the camera yaw swung 79° while the horse's heading stayed at 57°. With the mod off, the horse
  turned towards the camera before moving off.
- **Forward:** the horse travels 180° from its physics Z axis, so its forward is minus Z.
- **Input components:** the second input component belongs to the horse.
- **Gaits:** trot and gallop steer as expected. Releasing W while steering gives a tighter turn.
- **Turn rates at walking speed:** about 34°/s for W with A or D, and about 66°/s for A alone.
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
- **Steering tuning:** stage 4 passed in game, and the tester rated `kSteerRampMs`, `kTightSteerFactor` and `kWideSteerFactor`
  about right, so leave them as they are.

## Taming minigame

- **Input:** it needs input pointing at the mount's tail, and it reads the movement data the mod edits: holding S wins it
  with ease while backward input reaches the game. The package name can't separate wild horses from tamed ones.
- **Rodeo signal:** slot 47 of `UIGamePlayControlRootMiniGameRodeo`'s vtable (`exe+0x5A6CBF8`) ran about 70 times a second during
  each minigame logged, and was silent otherwise. Slot 17 fired at each open and slot 19 at each close. Slot 47 starts 0.3–0.65 s after
  mounting.

The freeze after a win is in [stability.md](stability.md).
