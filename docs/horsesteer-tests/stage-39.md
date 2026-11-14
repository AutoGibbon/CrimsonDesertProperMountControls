# Stage 39 test

Braking while turning, by answering `T10` ("no move input") as holding for the mount during a slowdown, instead of
zeroing the stick. The slowdown now starts and carries on with steering held, and the stick is left as is, so the mount
should keep turning while it slows. This runs whatever `Logging` is set to. Background is in
`findings/charts-and-braking.md` (Slowing while steering) and [stage 38](stage-38.md).

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. The build is already in `DMM\mods\`. In DMM, turn `ProperHorseControls` off and on again and apply.
3. In `bin64\ProperHorseControls.ini`, set `Logging=1`, `CameraDetached=1` and `TapBrake=1`.
4. Load save slot 1. Use the horse, then the bear, camel or wolf.

## Steps

On the horse, with the keyboard. Repeat each tap 3 times, at a canter and at a gallop:

1. **Brake key while turning:** hold W and D, and tap Alt.
2. **S while turning:** hold W and A, and tap S.
3. **S with A:** hold W and A, release W, and hold S with A for 3 s. Then once more, tapping S with A instead.
4. **Straight taps:** hold W and tap Alt, then hold W and tap S. Each should drop one gait, as before.
5. **Alt with A, W released:** at a canter, release W and press Alt with A. Note whether it drifts.
6. **Backward walk:** from standing, hold S, then S with A and S with D.

With a gamepad, if one is to hand:

7. **B while turning:** at a canter, hold the stick forward and to one side, and tap B.
8. **Stick back while turning:** at a canter, pull the stick back and to one side.

On another mount from save slot 1:

9. Repeat steps 1, 3 and 4.

Also walk a few steps on foot after dismounting. Close the game and copy the log and CSV to `scratch/probe-runs/stage39/`.

For each step, note whether the mount kept turning while it slowed, and how many gaits it dropped.

## Logs

- **Setup:** `lookup cachedConditionSlot: 77` and `no input answer installed`, before `MOD ACTIVE: all features
  enabled.`
- **Slowdown:** `brake gap start ... source=...` now appears for presses with `steering=1`, and `brake gap end` never
  gives `reason=steering`.
- **`T10`:** `T10 R game G tick=... raw stick x, y sent x, y` logs each change of the mount's answer `R`, with the game's
  own answer `G`. During a slowdown with input held, `R` is 0 and `G` is 1. `sent` is no longer zeroed.
- **CSV:** `t10` is the answer the chart got, and `mount_phys_yaw` shows whether the mount kept turning.

## Reading the results

| Step | Expected |
| --- | --- |
| 1, 2, 7 | `T10 0 game 1` at the press. The horse goes `C5AF` to `B7CC` with no bounce back to `C2B2`, dropping one gait, and keeps turning |
| 3 | The tap drops one gait while turning. On the hold, the slowdown ends at the next slowing state, after which sideways input reads as input again, so the horse may stay at the lower gait rather than stop. Note which |
| 4 | One gait per tap on the horse. Other mounts land where they did before: the bear on 5.2 and the camel on 3.5 |
| 5 | Whatever happens is accepted: drift is not a concern |
| 6 | Unchanged |
| 8 | As step 3 |
| 9 | Other mounts slow while turning, landing where their straight taps land |

Other outcomes:

- **The mount stops turning while slowing:** its slowing state ignores x. Note which mount and state.
- **The chart bounces out of a slowing state with `T10 0` logged:** something other than `T10` holds it there, such as
  the other mounts' `OR T122(0, 50) T122(0, 51)` branch.
- **`T10` doesn't change on foot or for other actors:** expected, as the answer only covers the ground mount.


Tester notes for each step:

mounts did keep turning while they slowed, but their animations went wacky, and sometimes they did not slow at all and just got stuck in a weird animation state while turning.

1. all mounts go into what looks like an unknown state or one that the charts cannot handle correctly
2. ^
3. ^
4. wolf sometimes sped up here instead of dropping to lower gait.
5. drift activates at gallop. tested drift activation at gallop once. worked.
6. backward walk not impacted by changes.

pad:

7. pad behaviour consistent with KBM behaviour, mounts get caught in state that appears unhandled.
8. this was tricky to do, I hope I captured the data!

## Results

Failed. Run on 2026-11-14 with the horse and elite wolf. Logs are in `scratch/probe-runs/stage39/`.

- **Setup:** `no input answer installed`, and `MOD ACTIVE: all features enabled.`
- **Flipping:** with `T10` answered, the chart entered the slowing state and left it again on the next frame, every 30 to
  50 ms, until the gap ended. On the horse that's `C2B2` and `C5AF` from a canter. On the wolf it's `D16B` and `CA3A`, and
  `CE49` and `CBCB`. This is the stuck animation. 51 of 73 gaps ended on the 3 s time cap, mostly with no speed lost.
- **Not about steering:** straight canter taps with W held flipped too, and so did S taps where the stick read 0, 0.
- **Gallop taps worked:** `D099`, `D85C`, `C5AF`, ending one gait down, in 47 to 78 ms.
- **Steps 5 and 6:** drift and the backward walk were unaffected.

Entering the slowing state reads the answer, but the check that keeps the chart there sees "input". With the stick
zeroed, both see no input, which is why the old gap doesn't flip. [Stage 40](stage-40.md) logs who reads `T10` during a
gap.