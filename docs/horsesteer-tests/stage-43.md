# Stage 43 test

A probe for `T167` (type `0x167`), which ride charts test on every change of gait, such as `T167(4, 0)` to stay at
the horse's canter. It may be the gear that Shift selects. With `Logging=1`, the mod replaces `T167`'s handler table slot
with a function that logs the mount's reads without changing them. Braking works as in [stage 42](stage-42.md).

If `T167` is the gear, the bear and other mounts could drop one gait per tap by lowering it, with no speed tables.
Background is in `findings/charts-and-braking.md` (Slowing while steering).

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. The build is already in `DMM\mods\`. In DMM, turn `ProperHorseControls` off and on again and apply.
3. In `bin64\ProperHorseControls.ini`, set `Logging=1`, `CameraDetached=1` and `TapBrake=1`.
4. Load save slot 1 and use the horse, then the bear.

## Steps

On each mount, with the keyboard, going straight:

1. **Up:** from standing, hold W to walk. Then tap Shift once for each gait up to the highest, holding W for about 2 s at
   each gait.
2. **Down:** from the highest gait, tap Alt once, wait about 2 s with W held, and repeat until the mount is walking.
3. **Stop:** release W and let it stop.

Close the game and copy the log and CSV to `scratch/probe-runs/stage43/`.

## Logs

- **Setup:** `gear probe installed, T167 handler at exe+0x...`, before `MOD ACTIVE`.
- **First reads:** `T167 first read: P0 P1 P2 P3 result=R` once for each parameter set the mount's chart reads. `P2` and
  `P3` should be the two numbers in `T167(4, 0)`.
- **Changes:** `T167(k, 0) R tick=... speed=...` each time the answer for one parameter set changes. 0 means it holds.
- **State lines** show the gait at each change, and the CSV's `shift` column shows Shift.

## Reading the results

| Outcome | Meaning |
| --- | --- |
| `T167(k, 0)` changes to 0 on each Shift tap, one `k` higher each time, and back to 1 one `k` at a time on each brake tap that drops a gait | It's the gear. Next is finding where it's stored, from the handler, and lowering it by one on a tap |
| The bear's `T167` changes several steps at once on a tap | The gear follows the slowing node, which is why it lands low. Lowering it is still the next step |
| `T167` follows speed rather than Shift | It isn't the gear |
| No `T167` lines for the mount | The chart reads it for another actor, or not through the table |

## Results

Run on 2026-11-14 with the horse, elite wolf, camel, bear, battle boar and cucu bird. Logs are in
`scratch/probe-runs/stage43/`.

- **Handler:** `T167`'s handler is `exe+0x3BB840`.
- **Not the gear:** each mount logged one `T167 first read`, answering 0, and no changes all session, through every
  Shift tap and brake tap. The condition's first 16 bytes were `00000167`, then `00000001` on the horse and `00000003`
  on the others, then two zeros, so the `(4, 0)` arguments are further in and the probe merged every `k` into one entry.
  With no change logged for any `k`, every read answered 0. `T167(k, 0)` is probably "gait `k` is unlocked", which
  this levelled save has for every gait.
- **Gait landings, unchanged from stage 42:** wolf 15 to 5.2 to 5.4 m/s, camel 9.0 to 5.4, bear 13.3 to 4.8, battle boar
  10.8 and 7.4 to 5.3 to 5.4, cucu bird 9.5 to 3.7 and 5.9 to 3.5 to 3.8. From each one's lowest moving gait, a tap
  stopped it. The horse dropped one gait from every gait.

The fresh charts show a branch from each gait to the one below on `T66`, in every non-horse ride chart. See
`findings/charts-and-braking.md` (Slowing while steering).
