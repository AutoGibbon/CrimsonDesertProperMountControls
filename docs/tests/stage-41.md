# Stage 41 test

Writing 0, "no move input", into the mount's condition cache for `T10` during a gap, at `[component+0xD0] + 8 + 0x10`, on
every stick update and every slot 77 read, as well as answering slot 77. In [stage 40](stage-40.md) the check that keeps
the chart in its slowing state didn't go through slot 77. If it reads the cache, the flipping stops. If it checks the
stick itself, nothing changes. Other game code may read the same byte as input, so this also checks that the mount
still responds.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. The build is already in `DMM\mods\`. In DMM, turn `ProperHorseControls` off and on again and apply.
3. In `bin64\ProperHorseControls.ini`, set `Logging=1`, `CameraDetached=1` and `TapBrake=1`.
4. Load save slot 1 and use the horse.

## Steps

On the horse, with the keyboard:

1. **Canter, straight:** hold W and tap Alt 3 times.
2. **Canter, turning:** hold W and D and tap Alt 3 times.
3. **Gallop 1 and 2:** hold W and tap Alt twice from each.
4. **After each tap:** keep W held and check the horse speeds up again, and that A and D still steer.
5. **Backward walk:** from standing, hold S, then S with A.

If the flipping has stopped, repeat steps 1 and 2 on the bear or wolf.

Close the game and copy the log and CSV to `scratch/probe-runs/stage41/`.

## Logs

- **Gap reads:** as in stage 40, after each `brake gap end`.
- **Cache writes:** `gap cache writes=N found set=M` after each gap. `N` is how often the mod checked the byte, and `M`
  how often it found the game's 1 there and wrote 0. `M` near `N` means the game refreshes the byte every frame.

## Reading the results

| Outcome | Meaning |
| --- | --- |
| The chart stays in `C5AF` and reaches `B7CC`, with `reason=next state` | The check reads the cache. This is the fix, if steps 4 and 5 still work |
| Flipping as in stage 39, with `found set` above 0 | The check doesn't read the cache, or reads it before the mod's write. Next is finding when the game fills the cache |
| `found set=0` and flipping | The byte the check reads isn't this one |
| The horse doesn't speed up or steer after a gap | Other code reads the byte as input. Note how long it lasts |

Tester notes:

tap brake from both gallops now drops gait correctly.
tap brake from canter/trot still gets the flipflop state

## Results

Run on 2026-11-14 on the horse. Logs are in `scratch/probe-runs/stage41/`.

- **The write took:** slot 77's own answer for `T10` became 0 (`game=0`) during gaps. The game set the byte back to 1
  about 115 times in each 3 s gap, about once a frame.
- **Still flipping:** every canter (`C2B2`) and trot (`B4A7`) tap flipped until the time cap, but one of each. All taps
  from `D099` and `E38E` ended on `reason=next state` within 47 to 78 ms.

So the cache's `T10` isn't what sends the chart back. The horse's slowing states have a branch back up to a gait on
`TE` and not `key_brake`, such as node 0's to node 82: `AND T167(4, 0) TC9(4.2,4)`, then `AND OR TE AND TE key_run NOT
key_brake`. `TE`, type `0x0E`, reads 0 for the mount throughout, so with the stick left as is the chart goes straight
back up. A zeroed stick fails `TE`, and a held brake key fails `NOT key_brake`, which is why both stay down. Gallop taps
end the gap after two quick slowing states, before this shows. [Stage 42](stage-42.md) answers `TE` as well.