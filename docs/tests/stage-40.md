# Stage 40 test

A probe for the flipping in [stage 39](stage-39.md). The build answers `T10` the same way, and with `Logging=1` it also
counts every slot 77 read during a gap: `T10` for any actor, and every type for the mount, with the code that asked.
This shows whether the check that sends the chart back out of its slowing state reads `T10` for the rider, reads it
through slot 77 at all, or reads the game's cache some other way.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. The build is already in `DMM\mods\`. In DMM, turn `ProperHorseControls` off and on again and apply.
3. In `bin64\ProperHorseControls.ini`, set `Logging=1`, `CameraDetached=1` and `TapBrake=1`.
4. Load save slot 1 and use the horse.

## Steps

On the horse, with the keyboard, at a canter:

1. **Straight:** hold W and tap Alt 3 times, letting each gap run out.
2. **Turning:** hold W and D and tap Alt 3 times.
3. **From a gallop:** hold W and tap Alt twice, as a comparison that doesn't flip.

Close the game and copy the log and CSV to `scratch/probe-runs/stage40/`.

## Logs

- **Gap reads:** after each `brake gap end`, one `gap read type=0xNN actor=WHO ADDRESS caller=exe+0x... result=R game=G
  count=N` line per combination seen during the gap. `WHO` is `mount`, `rider` or `other`. `result` is what slot 77
  returned after the mod's answer, and `game` is what the game had.
- **Dropped:** `gap reads dropped: N` means more than 48 combinations, so some weren't listed.

## Reading the results

Compare a flipping gap from steps 1 and 2 with a working one from step 3.

- **A `T10` read for `rider` with `game=1`:** the check that sends the chart back reads the rider's input. Next is
  answering for the rider too, only during a gap.
- **Only `mount` reads of `T10`, all `result=0`, from one or two callers:** the check that sends the chart back doesn't go
  through slot 77. It reads the cache at `[component+0xD0] + 8 + 0x10` some other way. Next is writing 0 to that byte
  during a gap, and testing that the mount still turns and speeds up afterwards, since other code may read the byte as
  input.
- **A `T10` caller only seen in flipping gaps:** disassemble it to find the check.

## Results

Run on 2026-11-14 on the horse. Logs are in `scratch/probe-runs/stage40/`.

- **One caller:** every read came from `exe+0x2691BE2`, the chart interpreter's call to slot 77, for the mount. The rider
  never read `T10`. Two other actors read it, getting 2.
- **`T10` reads:** 47 to 60 in each 3 s flipping gap, which is one per entry into the slowing state, and 1 in each working
  gap. The check that sends the chart back doesn't go through slot 77.
- **Other types:** the mount also read `0x0E`, `0x13`, `0x22`, `0x24` and `0x68`, unchanged by the mod.
- **Gaits:** all 4 taps from `E38E` (14) worked. 2 of 3 from `D099` (12) flipped, unlike stage 39. Every canter tap
  (`C2B2`) flipped, straight or turning.
- **Static search:** the only code that loads `[x+0xD0]` and then reads the byte at `+0x18` is in
  `UIGamePlayControlCommonTimerGaugeAlert`, so nothing reads `T10`'s cache byte at a fixed offset. A read with the type
  as an index, as slot 77 does, wasn't searched for.
- **Duplicate lines:** a few `brake gap end` and `gap read` lines were logged twice, so `UpdateBrakeGap` may run on two
  threads at once.

That leaves two explanations: the check reads the cache with the type as an index, or it doesn't read the cache and
checks the stick itself. [Stage 41](stage-41.md) writes the cache byte to tell them apart.
