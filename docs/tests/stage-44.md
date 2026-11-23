# Stage 44 test

One gait down per tap on mounts other than the horse. On those mounts, a tap answers `T66` (type `0x66`) as holding
until the chart state changes, which every non-horse ride chart takes as a step down to the gait below, when Shift
isn't held. The gaits come from the charts, with no speed tables. If the state hasn't changed 200 ms after the mount has
been in it for 500 ms, there's no gait below, and the [stage 42](stage-42.md) slowdown runs instead. The horse and wagon
are unchanged. Background is in `findings/charts-and-braking.md` (Slowing while steering).

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. The build is already in `DMM\mods\`. In DMM, turn `ProperHorseControls` off and on again and apply.
3. In `bin64\ProperHorseControls.ini`, set `Logging=1`, `CameraDetached=1` and `TapBrake=1`.
4. Load save slot 1. Use the bear, then as many other mounts as time allows, then the horse.

## Steps

Holding W keeps the current gait, and each Shift tap raises it one gait.

On each mount, with the keyboard:

1. **Straight:** tap Shift up to the highest gait. Tap Alt, wait about 2 s with W held, and repeat until the mount
   stops.
2. **Turning:** tap Shift up to the highest gait. Hold D and repeat step 1's taps. Note whether the mount keeps turning.
3. **Quick tap:** tap Shift up one gait and tap Alt within half a second. Note how long it takes to drop.
4. **S:** tap Shift up to the highest gait, then tap S with W held.

On the horse, repeat step 1 as a check that it still drops one gait per tap.

Close the game and copy the log and CSV to `scratch/probe-runs/stage44/`.

## Logs

- **Step down:** `step down start ... state=XXXX in state Nms ... source=...` at the tap, then `step down end ...
  state=YYYY value=V reason=R`. `reason=gait changed` is a step, `stopped` is a stop, and `no step` means no gait below.
- **Fallback:** after `reason=no step`, `brake gap start ... source=no step` and the usual `brake gap end`.
- **State lines** show the gait at each change, and the CSV's `mount_value` the gait's value.

## Reading the results

| Outcome | Meaning |
| --- | --- |
| Each tap drops one gait, `reason=gait changed`, and the lowest gait's tap ends `no step` and stops the mount | The fix |
| A tap lands two gaits down, or `gait changed` into a state with no value | The first change wasn't the step. Compare the state with the chart |
| `no step` above the lowest gait | `T66` isn't read there, or Shift was held |
| The mount stops turning, or behaves oddly after a step | `T66` does more than step down. Note what |

tester notes:

general note: the step 3 quick tap demonstrates that each mount will first respond to the speed up request, and then will respond to the slowdown request. the time this takes varies per mount but the behaviour is good.

horse - all good
bear - S tap behaviour inconsistent.
camel - S tap behaviour inconsistent.
wolf - S tap behaviour inconsistent.
boar - S tap behaviour inconsistent.
cucu - S tap behaviour inconsistent.

## Results

Run on 2026-11-14 with the horse, bear, camel, elite wolf, battle boar and cucu bird. Logs are in
`scratch/probe-runs/stage44/`.

- **Step downs:** every `step down start` moved exactly one gait, from both Alt and S, ending `reason=gait changed` in
  15 to 32 ms, such as the bear's 13.3, 9.2, 5.2 and 1.8, and the camel's 9.0, 6.2, 3.5 and 2.6. A tap within 0.5 s of a
  Shift tap waited, ending in 188 to 282 ms. At each mount's lowest gait the step ended `reason=no step` after about
  200 ms and the slowdown stopped the mount.
- **Horse:** unchanged, one gait per tap.
- **S taps:** about half of the S taps on non-horse mounts stepped down and then went back up, such as the bear from
  9.2 to 13.3, the camel from 3.5 to 6.2 and the cucu bird from 5.9 to 9.5. Some passed through a state with value 0
  first, and some landed two gaits down. Alt taps never did this.

With W and S held the stick reads 0, so for the rest of an S tap, about 70 ms after the step, the chart reads `T10`
from the game and goes into its shared slowing node. When S is released it picks a gait by speed, as in
[stage 22](stage-22.md), often the higher one. A stick pulled back does the same, because the mod clears its backward
part.
