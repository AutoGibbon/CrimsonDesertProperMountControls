# Horse backward walk

S from standing walks the horse backward, as in RDR2. It's built into `HorseSteer` with no chart patch, and is limited to `CD_R0001_Horse`. Analysed on
build 1.0.0.3479. Stage tests and results are in `../horsesteer-tests/`, with logs in `scratch/probe-runs/stage5*/`.

## How it works

1. **Clip:** `character/binary/motionblending/horse/locomotion.motionblending` is a `ParameterizedMotionSpace` whose clips include
   `cd_rd_prh_00_00_nor_move_walk_b_step3_00`, `bl_step3` and `br_step3`. Its inputs are `MovingSpeedForwardInLocal` and `MovingSpeedRightInLocal`, so the backward clips probably play because the position writes give the mount a backward velocity. The `walk_s` stop nodes (15, 28, 186, 271, 279) and many walk nodes use it. Idle (134)
   and the end of a stop (20) use no blend, so moving the horse there slides it in its pose.
2. **Movement:** `UpdateBackwardWalk` adds to the physics object's position at `+0x90`. A rate of 0.5 moves the horse at about 1 m/s, and the feet match at that
   speed. Why it doubles is unknown. Most writes don't read back unchanged on the next update, so the game also changes the position. The horse follows slopes and is
   stopped by walls.
3. **State:** with no stick input, walk nodes such as 137 go to node 186. It loops on `T83`, and with no forward input it only leaves on `TC9(0.1,2)`, which looks
   like speed under 0.1 m/s. So it holds while the horse keeps moving, and the blend appears to follow the backward movement. Stage 8 logging showed one chart state held for the whole 16.5 s walk, the
   same state a walk's stop passes through. Which node that state is remains inferred.
4. **Start:** `UpdateBackwardPulse` waits for the horse to drop below 0.2 m/s, then reads S as forward for 150 ms, once per press, while the movement starts. This gets
   it from idle or a stop into a blended node.
5. **Recovery:** the chart state can leave the walk states `A73F` and `A56E` while the position writes continue, a state drop, which slides the horse in its pose. `UpdateBackwardRecovery` then gives the same 150 ms pulse, at most once a second and not during steering. In stage 9d this returned the state to `A73F` in about 0.3 s from about 1 m/s, with no forward movement.
6. **Steering:** A or D during the walk becomes a steering input of 0.56, and the node's `Rotate` turns the horse with no forward walk. The head turns the way of the key.
   The horse turns at about 62°/s whatever the input's size. Larger inputs curve the path, at about 0.7 m/s at this setting. A full input leaves the blend.

## Limits

- The cause of state drops is unknown. Three logged at about 1 m/s came at a grade of about +0.5, but the horse also held `A73F` on stretches up to +0.72, and speed is well above the 0.1 of `TC9(0.1,2)`, so that condition's meaning above is in doubt. Stage 9's state drops within about 0.2 s of releasing A or D, in 8 of 9 cases, didn't reproduce in stages 9b to 9d.
- Against a wall or stuck on an object, `UpdateBackwardRecovery` pulses once a second for as long as S is held. The tester didn't notice this at a wall. During one stuck stretch on a slope, the horse moved forward at up to 0.71 m/s for about 0.9 s in states `BE4D` and `BFCE`, and whether the pulses caused it is unknown.
- `kStateWalk`, `kStateWalkSteer` and `kStateWalkStart` are the low 16 bits of a pointer, not a confirmed identity. If they stop matching, recovery doesn't fire.
- At a wall, the horse backs in until the rider's collider stops it, so its rear goes into the wall.
- From a walk, trot or gallop, the walk starts after about 1.7, 3.4 or 4.2 s, because the horse stops first. The start can jitter slightly.
- A or D first from standing depends on the state the horse last left: a forward tight turn after moving forward, or a pivot after reversing or mounting.

## Other mounts

The same method works on every mount reached in [stage 11b](../horsesteer-tests/stage-11b.md): the camel, reindeer, snow deer, boar, bear, wolf, tiger, iguana, carmabirdsaurus and ibex. The tester passed all steps, including W after reverse and starting from a run. The per-mount nodes are in [mounts-and-vehicles.md](mounts-and-vehicles.md).

| Mount | Best rate | Reverse turn, °/s | Walk state, low 12 bits |
| --- | --- | --- | --- |
| Camel | 0.9 to 1.0 | 147 | `C1F` |
| Reindeer | 0.7 | 59 | `B3B` |
| Snow deer | 0.6 | not enough data | `B3B` |
| Boar or warthog | 0.8 | 59 | `E7A` |
| Bear | 0.9 | 60 | `61B` |
| Wolf | 0.9 | 148 | `BCB` |
| Tiger | 0.8 | 141 | `BCB` |
| Iguana | 0.6 | 60 | `DF1` |
| Carmabirdsaurus | 0.8 | 151 | `BCB` |
| Ibex | 0.6 | 60 | `73F` |

- **Speed:** on every mount, speed in m/s is about twice the rate added to the position, from 0.2 to 1.1.
- **Turn rate:** it differs by mount and comes from the game, so `SteerInput` doesn't set it. Changing it would need a chart patch or the mod writing the mount's rotation.
- **Losing the walk:** the chart can leave the walk for its stop and idle states while the position writes continue, so the mount slides at walking speed. Stage 12 logged slides over 1 s on most mounts, up to 4.6 s on the wolf, after turns and with no turn nearby, on flat ground as well as slopes. In stage 11b, the walk state didn't return before S was released in 8 of 41 releases on the camel, 6 of 14 on the boar and 2 of 5 on the bear.
- **Chart value:** on all 13 mounts, `mount_value` is 0 in the backward walk, above 0 in reverse steering and missing in the stop and idle states. The horse's `8FC` is the exception seen, with a value of 0 during a drop at low speed.
- **Recovery:** `UpdateBackwardRecovery` pulses when the value has been missing for 200 ms at above 0.3 m/s, and repeats every 500 ms, with no state IDs. In [stage 12b](../horsesteer-tests/stage-12b.md), slides on the camel, boar, cucubird and reindeer ended within 0.4 s, with no forward lurch, on turns, slopes and at walls. The horse's `kStateWalk` checks are still needed for `8FC`, and they cover the ibex too.
- **A single pulse on release:** a pulse 100 ms after releasing A or D caused no forward lurch in 77 uses, but it didn't stop the losses, and it delayed the walk state's return from about 110 ms to about 330 ms. In boar failures, the state went from steering straight to stop then idle. The pulse probably landed during the stop clip, which ignores input, though this is inferred.
- **State IDs:** the low 16 bits of a chart state pointer aren't stable on every mount, since the reindeer's walk state was `6B3B` in stage 11 and `0B3B` in stage 11b, while the low 12 bits matched. `kStateWalk` and `kStateWalkSteer` still only match the horse and ibex.
- **S with the mod off:** the camel, reindeer, boar, bear, wolf, iguana and carmabirdsaurus moved forward along their facing, about 175° from the backward axis, so clearing S doesn't block a backward move on them.
- **Stage 12 additions:** the cucubird and lion also work, with feet matching at 1.0 and 1.7 m/s. Their walk states are `B13` and `BCB`.
- **Not tested:** the elephant couldn't be reached, as its only save record didn't appear in the mount list. Its chart and blend have the same layout as the mounts above.

## What didn't work

- **The `walk_b` nodes:** nodes 4, 43 and 226 have clips but no branch in from another state, and no move speed data. Routed through a stop with a chart patch, they
  animate on the spot.
- **Chart move speed data:** an unused animation with a move speed record (node 204), with speed -1 and animation header `+0x6C` at -3, didn't move the horse with no keys,
  W held or S held. On moving animations, `+0x6C` equals speed times clip duration.
- **Loose chart file:** the patched chart in a DMM group folder crashed during loading once. The same bytes as a JSON byte patch (`patches` with `offset`, `original` and
  `patched`) loaded every time, so use that for any future chart edit.
