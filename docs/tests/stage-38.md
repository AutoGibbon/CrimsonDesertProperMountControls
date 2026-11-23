# Stage 38 test

A probe for the chart condition `T10`, which every ride chart tests on its branches into slowing. With `Logging=1`, the
mod replaces slot 77 of `ClientCharacterControlActorComponent`'s vtable, which returns cached results for condition types
below `0x83`. It logs what the ground mount's chart reads there without changing any answer. This shows how the result
is encoded, whether `T10` means "no move input", and whether a sideways-only input counts as input. Background is in
`findings/charts-and-braking.md` (Slowing while steering).

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. The build is already in `DMM\mods\`. In DMM, turn `ProperHorseControls` off and on again and apply.
3. In `bin64\ProperHorseControls.ini`, set `Logging=1` and `CameraDetached=1`.
4. Load save slot 1. Use the horse, then the bear, camel or another of the save's other mounts.

## Steps

On the horse, with the keyboard:

1. **Standing:** stand still for 3 s with no input.
2. **Straight:** hold W to a gallop for 3 s, then release W and let the horse coast to a stop.
3. **Sideways only:** at a canter, hold W and D, then release W and keep D held for 3 s.
4. **S with steering:** at a canter, hold W and A, then release W and hold S with A for 3 s.
5. **Tap S:** at a canter, tap S with W held, going straight.
6. **Brake key while turning:** at a canter, hold W and D and tap Alt once, then hold Alt for 2 s.

With a gamepad, if one is to hand:

7. **Stick:** stand still, then push the stick slightly forward, fully forward, fully sideways, and release it, about
   2 s each.
8. **B while turning:** at a canter, hold the stick forward and to one side and tap B once.

On another mount from save slot 1:

9. Repeat steps 2 and 3.

Close the game and copy the log and CSV to `scratch/probe-runs/stage38/`.

## Logs

- **Setup:** the log has `lookup vtControlComponent`, `lookup cachedCondition`, `lookup cachedConditionSlot: 77` and
  `condition probe installed`, before `MOD ACTIVE`.
- **Types read:** `mount condition 0xNN first read: R` lists each low condition type the mount's chart reads, with its
  first result, on each mount, including a remount of the same animal.
- **`T10`:** `T10 R tick=... raw stick x, y sent x, y` logs each change of the mount's result. `raw stick` is the input
  before the mod's edit, and `sent` is what the game's update read after it, which is what the chart sees. The CSV's
  `t10` column has the latest result, or -1 before the first read and after a dismount.

## Reading the results

If 0 means "holds", as it does for the key handler:

| Step | Expected `T10` |
| --- | --- |
| 1, 2 | 0 with no input, including after W is released. 1 with W held |
| 3 | Shows whether a sideways-only input counts as input. The mod sends x = ±1, y = 0 for D alone, so read `sent`, not the keys |
| 4 | 1 throughout, which is why the chart keeps cantering with S and A |
| 5 | 0 while the brake gap runs, since it zeroes the stick |
| 6, 8 | 1 through the tap's bounce out of `C5AF`. On the hold, the chart reaches `B7CC` on `key_brake` alone, with `T10` still 1 |
| 7 | Shows whether a slight push counts as input |

Other outcomes:

- **`T10` reads 2:** slot 77 returns 2 when a type's flag is 0 and the actor check `exe+0x19979A0` fails, and `T10`'s
  flag is 0. The cache isn't consulted for the mount, so `T10` can't be answered through slot 77 as it stands. Next is
  tracing that check.
- **No `T10` lines and no first read of `0x10`:** `T10` is answered before slot 77, probably by the interpreter's
  override list (`findings/offsets.md`, Chart condition evaluation). Next is tracing that list.

## Results

Run on 2026-11-14 with the horse, bear, camel and elite wolf. Logs are in `scratch/probe-runs/stage38/`.

- **Setup:** every lookup was found, the probe installed at slot 77, and `MOD ACTIVE` followed.
- **Types read:** `0x04`, `0x05`, `0x0E`, `0x10`, `0x13`, `0x22`, `0x24`, `0x65`, `0x66` and `0x68`. `0x10` was read
  on every mount, first as 1, and never as 2, so slot 77 answers `T10` for every mount tested.
- **Encoding:** slot 77 returns 0 when `T10` holds, which is "no move input", and 1 otherwise. Every change to 0 came on
  the same tick as the chart's move into a slowing state: `C5AF`, `B7CC` or `A73F` on the horse, `1492` on the bear,
  `CA7A` on the camel and `CA3A` or `CBCB` on the wolf. Every change to 1 came on the move back out.

| Step | `T10` |
| --- | --- |
| 1, 2 | As expected |
| 3 | 1 throughout. Sideways only counts as input. The horse held D alone for 8.6 s in `C2B2` at 4.2 m/s. The bear (`1A1A`), camel (`D056`) and wolf (`CFC6`) stayed in one state with D alone, but slowed in it, the bear from 6.7 to 2.1 m/s. The tester saw them turn in very tight circles with steering alone, which accounts for the lost speed |
| 4 | 1 for 5.9 s in `C2B2`, from 6.0 to 5.0 m/s |
| 5 | 0 during the 766 ms gap from S, through `C5AF` to `B7CC`, then 1 |
| 6, 8 | 1 throughout, on keyboard and gamepad. With `steering=1` no gap started. Taps went `C5AF` to `C2B2`, and holds went on to `B7CC` and `A73F` |
| 7 | A sent y of 0.03 reads 1, so a slight push counts. Fully sideways, sent (1.00, 0.00), reads 1 and stays in `A56E` |
