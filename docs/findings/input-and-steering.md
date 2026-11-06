# Input, steering and camera

Build 1.0.0.3479. Addresses and field offsets are in [offsets.md](offsets.md).

## Game data

- **Input:** `ui/inputmap.xml` binds `MoveAnalog` to the left stick and WASD as a plain 2D vector, with no coordinate
  frame setting. Remapping input cannot produce horse-relative movement.

## Ruled out

- **`MoveControlOptionType` (`Classic`/`Standard`):** this appears in the horse chart, but patch 1.02.00 describes the
  setting as sprint input behaviour, not steering.
- **Camera presets:** the `Player_Ride_Horse*` presets in `playercamerapreset.xml` set `FollowYawSpeedRate` and
  `FollowStartTime` but not `FollowCameraEnable`; `Player_Weapon_Default_RunFast_Follow` sets it, with rate 8 and start
  time 0. Whether the ride presets are active in game is unconfirmed. DMM can merge a partial patch into this file, as
  `playercamerapreset__cclite.xml` does.
- **"Camera Auto-follow" setting:** matches the exe's `_enableCameraFollowYawPitch` field. The saved options file doesn't
  store a value for it.

## From HorseProbe run 1 and disassembly

Logs are in `scratch/probe-runs/run1/`.

- **Update function:** slot 33 of the `ClientInputActorComponent` vtable, confirmed through RTTI, looks up the `MoveAnalog`
  input by name and copies the camera's forward and right vectors into the component.
- **Angle calculation:** builds `stick.x * right + stick.y * horizontal forward` and returns `atan2(-x, -z)`, stored as the
  move angle. A second call, with both vectors rotated by the inverse of the actor rotation at TransformSync `+0xFC`, gives
  the relative move angle.
- **Move input:** raw values are in an input state object. Each input component holds its own copy. There are two such
  components: slot `i0` is the rider's and `i1` is the horse's.
- **Headings:** with the camera held still, the rider's physics Z axis yaw swung about 90° left for A, 90° right for D
  and 180° for S, and followed the horse while mounted.
- **Untraced exe strings:** `LeftStickCharacterForward`, `LeftStickCharacterRight` and `MoveDirectionYaw`.

## From HorseSteer test runs

Logs are in `scratch/probe-runs/steer1/` and `steer2/`.

- **Result:** during a W hold the camera yaw swung 79° while the horse's heading stayed at 57°. With the mod off, the horse
  turned towards the camera before moving off.
- **Forward:** the horse travels 180° from its physics Z axis, so its forward is minus Z.
- **Gaits:** trot and gallop steer as expected. Releasing W while steering gives a tighter turn.
- **A or D alone:** turns at 60–66°/s from standing and 83–90°/s at 4 m/s.
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

- **Raw keyboard values (stage 14 to 15):** the raw stick at `+0x48` reads 0 or ±1 on each axis, including diagonals, such
  as (−1, 1) for W with A. `DigitalStick` treats those, and equal axes above 0.7, as keys, so the forward ramp skips a stick.
  Gamepad input hasn't been logged.
- **Reverse steering ramp (removed):** ramping A or D during the backward walk from 0 to `SteerInput` over 0.4 s delayed the
  horse's steering state `A56E` by about 437 ms on all 3 presses in stage 15, against 0 ms in stage 13. The stick logged
  about 0 until the ramp ended, so the horse got no gradual turn. On the wagon the same ramp logged values in between. The
  difference is untraced. Reverse turn rates don't follow the input size, so the ramp was removed.

## Taming minigame

- **Input:** it needs input pointing at the mount's tail, and it reads the movement data the mod edits: holding S wins it
  with ease while backward input reaches the game. The package name can't separate wild horses from tamed ones.
- **Rodeo signal:** slot 47 of `UIGamePlayControlRootMiniGameRodeo`'s vtable ran about 70 times a second during each minigame
  logged, and was silent otherwise. Slot 17 fired at each open and slot 19 at each close. Slot 47 starts 0.3–0.65 s after
  mounting. The mod pauses while slot 47 is being called.

The freeze after a win is in [stability.md](stability.md).
