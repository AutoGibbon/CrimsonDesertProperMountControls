# Horse backward walk

S from standing walks the horse backward, as in RDR2. It's built into `HorseSteer` with no chart patch, and is limited to `CD_R0001_Horse`. Analysed on
build 1.0.0.3479. Stage tests and results are in `../horsesteer-tests/`, with logs in `scratch/probe-runs/stage5*/`.

## How it works

1. **Clip:** `character/binary/motionblending/horse/locomotion.motionblending` is a `ParameterizedMotionSpace` whose clips include
   `cd_rd_prh_00_00_nor_move_walk_b_step3_00`, `bl_step3` and `br_step3`. The `walk_s` stop nodes (15, 28, 186, 271, 279) and many walk nodes use it. Idle (134)
   and the end of a stop (20) use no blend, so moving the horse there slides it in its pose.
2. **Movement:** `UpdateBackwardWalk` adds to the physics object's position at `+0x90`. A rate of 0.5 moves the horse at about 1 m/s, and the feet match at that
   speed. Why it doubles is unknown. Most writes don't read back unchanged on the next update, so the game also changes the position. The horse follows slopes and is
   stopped by walls.
3. **State:** with no stick input, walk nodes such as 137 go to node 186. It loops on `T83`, and with no forward input it only leaves on `TC9(0.1,2)`, which looks
   like speed under 0.1 m/s. So it holds while the horse keeps moving, and the blend appears to follow the backward movement. This is inferred: the blend's parameters
   aren't decoded, and no state logging confirms the node.
4. **Start:** `UpdateBackwardPulse` waits for the horse to drop below 0.2 m/s, then reads S as forward for 150 ms, once per press, while the movement starts. This gets
   it from idle or a stop into a blended node.
5. **Steering:** A or D during the walk becomes a steering input of 0.56, and the node's `Rotate` turns the horse with no forward walk. The head turns the way of the key.
   The horse turns at about 62°/s whatever the input's size. Larger inputs curve the path, at about 0.7 m/s at this setting. A full input leaves the blend.

## Limits

- At a wall, the horse backs in until the rider's collider stops it, so its rear goes into the wall.
- From a walk, trot or gallop, the walk starts after about 1.7, 3.4 or 4.2 s, because the horse stops first. The start can jitter slightly.
- A or D first from standing depends on the state the horse last left: a forward tight turn after moving forward, or a pivot after reversing or mounting.
- The walk is limited to the horse for now. Other land mounts have their own charts and are untested. Next, check whether the same technique works on the mounts in
  `kGroundMountPackages`: which blend files have backward clips, and which stop states hold while the mount keeps moving.

## What didn't work

- **The `walk_b` nodes:** nodes 4, 43 and 226 have clips but no branch in from another state, and no move speed data. Routed through a stop with a chart patch, they
  animate on the spot.
- **Chart move speed data:** an unused animation with a move speed record (node 204), with speed -1 and animation header `+0x6C` at -3, didn't move the horse with no keys,
  W held or S held. On moving animations, `+0x6C` equals speed times clip duration.
- **Loose chart file:** the patched chart in a DMM group folder crashed during loading once. The same bytes as a JSON byte patch (`patches` with `offset`, `original` and
  `patched`) loaded every time, so use that for any future chart edit.
