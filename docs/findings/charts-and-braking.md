# Action charts, keys and braking

`tools/chart_branches.py CDMW_SRC CHART_DIR` prints the horse chart's `key_brake` branches, the branches into
the slide and drift states, and its skill conditions, with conditions decoded.

## Verified from game data

Build 1.0.0.3479.

- **Rotate events:** the horse chart has 76 unique `Rotate` frame events. Of 2,911 across all charts, 2,905 share the
  same header bytes, and no value there selects between camera, input and character frames.

## Hypotheses

Build 1.0.0.3479.

- `Rotate` events set turn rates and angle limits while native code supplies the target direction. This is inferred from
  values such as 6.98 rad/s and 180.

## Brake and drift

Build 1.0.0.3479.

- **`key_brake`:** bound to Alt, mouse X2 and gamepad B, in the same `CharacterAction` group that on-foot charts read, so S can't be
  added to it in `inputmap.xml`. The game's text describes it as "Drift ... while dashing". Most horse uses are `OR T10 key_brake`,
  and `T10` is probably "no stick input", which fits the tester finding that Alt at normal gaits feels the same as releasing W. The
  slide stop, `gallop_f_slip_end`, is only entered from the drift-end and `faststart` clips and two special gallop states. In those
  gallop states, the `T10` branch is tested before `key_brake`, so braking needs a stick input. Drift starts from the slide stop, with
  `key_brake`, a steering angle past 45° and `SKILL_USABLE(12007)`. These skill IDs have not been matched to names or unlock rules.
  The tester reports that brake with A or D drifts in game, quickly slowing the horse and turning it to that side.
- **`key_brake` on other mounts:** of the land-mount ride charts, only the wolf and warthog read it, in one walk branch each,
  `OR T10 key_brake`. The bear, camel, reindeer, elephant, iguana, Carma bird and cucu bird ride charts don't read it.
- **Brake on S (stage 5, dropped, `scratch/probe-runs/stage5-brake/`):** few noisy samples. S alone felt the same as releasing W. With
  the brake answered on S, S with A or D drifted at 13–15 m/s, and turned at 70–85°/s with no drift at 11–12 m/s.
- **Drift recovery:** many drifts fall to about 0 m/s within 2 s, then reach about 10 m/s by 6 s, even with W held. Speed fell
  below 3 m/s within 5 s in 6 of 17 Alt drifts with the mod on and 1 of 7 with it off, with W held about as long in both. The cause is
  untraced. Not a confirmed problem: the tester saw rougher recovery in stage 5 and suspects needing S to start the
  drift made the escape inputs harder. Recheck only if it's still noticed with Alt.

## Slowing a mount with S

Constraints from the tester, for any S-based slowdown:

- **Don't use `key_brake`.** Zeroing the forward stick is the same input as releasing W, so it can't reach the slide and
  drift branches that need `key_brake`, on any mount's chart.
- **Shift and dash are the same input,** so `key_run` and `key_dash` go together. The `T10` branches out of gallop nodes
  `#24` and `#110` need both released, but with Shift held in stage 21 a stick gap still left state `E38E`, and
  `mount_value` returned to 14 within about 1 s.
- **A gait only rises with Shift, and holds once Shift is released.** Holding W keeps the current gait, and each Shift
  tap raises it one gait: walk, trot, canter, gallop 1 and gallop 2 (the gait table is in
  [tests/README.md](../tests/README.md)). A mount skill gives faster travel for a time past
  gallop 2, and needs W and Shift held. After a slowdown, the mount stays at the lower gait until Shift is tapped.
- **Mount speeds can't be a static table.** Mounts differ in stats, several horse breeds have different top speed and
  acceleration, and trust level changes those further. Maximum stats also take riding time to unlock, so logging each
  mount in live play isn't practical. Anything scaled to a mount, such as a position write, must be derived from the
  current mount as it's ridden.

Results from [stage 21](../tests/stage-21.md), on the horse, on build `0x6AED2E85`:

- **Raw stick:** W and S held together read 0 on `+0x48`, so the stick can't tell that from releasing W, and the horse
  stops as it does on a release. Detecting S during W needs a key read.
- **Stick gap:** zeroing the stick until the second change into a `mount_value` 0 state dropped exactly one
  `mount_value` in 48 of 48 taps with W held and no Shift (14 to 12, 12 to 9.5, 9.5 to 5.2, 5.2 to 2.8), and it stayed
  there. From 2.8, the horse nearly stops in about 2 s and walks on.
- **Speed after the gap:** from 12 and 14 the gap lasts 125 to 220 ms and loses under 1 m/s. Speed then falls to the
  lower gait's cruise over about 3 s.
- **Cruise speed:** `mount_value` 12 cruised at 8.4, 9.0 and 9.5 m/s, and 14 at 10.5 to 13.1 m/s, within one session.
- **Position write:** the gaps are too short for it to build up, at most 1.1 m/s, and it didn't shorten the 2 s walk
  tap. The tester saw a slight float with it.
- **Bear and camel:** the second-state rule lands on the bear's 5.2 and the camel's 3.5 from any higher gait, because of
  the shared slowing node below. Under those gaits, the second state has no `mount_value`, so the gap ends on its time
  cap with the mount nearly stopped.
- **Ending the gap at the next lower gait's speed ([stage 22](../tests/stage-22.md)):** W returning partway
  through the shared slowing node does resume a higher gait. On the bear, 12 of 12 taps from 13.3, 9.2 and 5.2 landed
  one gait lower. On the camel, 3.5 to 2.6 worked, but 9.0 released at about 6.1 m/s landed on 3.5, for a cause not
  traced. With no lower gait to target, a tap at the lowest gait still runs to the time cap.
- **Position write at `AssistPerSecond=0.4` on those mounts:** shortens the gaps by about a quarter, peaking at 2.0 m/s,
  and the tester saw no problem with the feet.
- **Position write removed:** on the wagon, `mount_vel` jumped by up to 2 m/s when it ended
  ([stage 27](../tests/stage-27.md)), and the tester could see it on the horse's animation more often than
  not. Without it, the bear's 5.2 tap took the same 2.1 to 2.2 s, on `reason=next state` (`probe-runs/bear-no-assist/`).
- **Stopped mounts:** in 10 saved runs, 25 of 279 taps ended on the time cap, 16 of them on the bear and camel's low gaits
  under the older end rules, after 0.8 to 2.0 s below 0.3 m/s. `UpdateBrakeGap` now also ends below `kBrakeGapStopSpeed`
  (0.3 m/s), so that W works again once the mount has stopped.

Horse slowing chain, with each node's exit speed and the low hex digits of its state: `#38` `E83C` (12), `#119` `D85C`
(9.5), `#0` `C5AF` (5.2), `#36` `B7CC` (2.8), `#186` `A73F` (near 0). A stop after releasing W from each gait, median time
to 0.5 m/s: walk 1.75 s, trot 2.84 s, canter 4.27 s, gallop 4.38 s, top gait 4.54 s. The last 2.8 m/s takes about 2.4 s
of a gallop stop.

Every other ride chart has one slowing node for all gaits, from static charts and 1 to 5 logged stops per mount. The
elephant has no logged stops. The charts have 40 to 50 nodes, against the horse's 281.

| Chart      | Slowing node | Exits below, m/s |
| ---------- | ------------ | ---------------- |
| Bear       | `#39`        | 4.5              |
| Camel      | `#28`        | 5.0              |
| Warthog    | `#37`        | 5.0              |
| Wolf       | `#17`        | 5.0              |
| Reindeer   | `#9`         | 3.0              |
| Elephant   | `#20`        | 3.0              |
| Carma bird | `#36`        | 4.0              |
| Cucu bird  | `#39`        | 3.5              |
| Iguana     | `#7`         | 2.5              |

The landing gait doesn't follow from the exit speed: the bear's 4.5 lands on 5.2, and the camel's 5.0 lands on 3.5.

The wagon has two slowing nodes, split by speed: see [mounts-and-vehicles.md](mounts-and-vehicles.md) (Wagon slowdown).

## Slowing a mount with the brake key

From [stage 24](../tests/stage-24.md) and [stage 25](../tests/stage-25.md), on build `0x6AED2E85`.
`tools/brake_gaps.py LOG CSV` tabulates each tap's start value, landing value, duration and whether it bounced.

- **`key_brake` moves the chart early:** on the horse, the first change into the slowing state comes about 15 ms after
  an Alt press, and about 95 ms after an S tap, which only zeroes the stick. This fits the `OR T10 key_brake` branches.
- **Bounce:** on Alt only, the chart can leave the slowing state and return, such as `C2B2`, `C5AF`, `C2B2`, `C5AF`. Why
  isn't known. Counting any two changes ended the gap at about 140 ms, and 7 of 22 taps from 5.2 to 12 kept their gait.
  With two consecutive changes needed, 18 of 18 dropped one gait, 5 of them after a bounce.
- **Slope:** not what separates them. All 7 failures were downhill, but so were 11 of the 15 taps that worked, including
  the two steepest.
- **Hold, before the step down:** W and Alt held brought the horse to a stop. The camel slowed but didn't fully stop.
  Holding now steps non-horse mounts down to a stop ([slowdown.md](../slowdown.md)).
- **Drift:** Alt with A or D, and Alt followed by A or D, still drift. All 7 steering presses logged had W released, so
  a press with W and a steering key already held was never logged.

## Slowing while steering

On build `0x6AF5D198` with v1.3.1, the brake key, or S with A or D, while turning at a canter or above doesn't slow
the mount unless the brake key is held; 1.4.0 fixed this (see "Turning taps" below). Below a canter, a tap slows it.
Logs are in `scratch/probe-runs/gamepad-drift-1/`
(KBM and gamepad, with a mod-off stretch) and `scratch/probe-runs/slow-steer-2/` (KBM, gamepad, and gamepad with
`CameraDetached=0`).

- **Cause:** the horse stays in `C5AF`, the canter slowing state, only while its slowing condition (`OR T10 key_brake`)
  holds, and moves on to `B7CC` after 250 to 900 ms. A tap releases `key_brake` before then, and with stick input `T10`
  is false, so the chart returns to `C2B2`. The base game does the same: mod off, one press went `C5AF`, `C2B2`, and the
  next went on to `B7CC`. Trot and below move on at once, so a tap works there.
- **Why the mod's straight taps work:** the gap zeroes the stick, which keeps `T10` true until the next state. With
  `CameraDetached=0` every press starts a gap, and all 11 dropped a gait, 7 of them from `C2B2` through `C5AF`.
- **Turning taps:** since 1.4.0 (task `brake-while-turning`), a gap answers `T10` as holding and `TE` as not through
  slot 77 and leaves the stick as is, so a turning tap drops one gait while steering carries on (`stage-42`,
  `stage-45`); see [slowdown.md](../slowdown.md). Steering still blocks a gap when slot 77 isn't found, because the gap
  then zeroes the stick. With v1.3.1, `UpdateBrakeGap` didn't start a gap while `steering` and ended one on
  `reason=steering`, so a brake key press left only `key_brake`: in `slow-steer-2`, 6 of 11 canter presses with
  `steering=1` returned to `C2B2` within 30 to 95 ms, and the other 5 reached `B7CC`, which the tester confirmed were
  held presses.
- **S with A or D:** the S tap needs W held, so with W released no gap starts. The mod writes y = 0 with x = ±1, which
  the chart treats as a canter turn. In `gamepad-drift-1`, 4.6 s of S with D and 8.1 s of S with A kept the canter.

Constraints from the tester for any fix:

- **Drift:** not a concern. The mount drifts when the chart's keys and state for it are met.
- **Backward walk:** must not change.
- **Consistency:** braking should behave the same on every eligible mount, which is worth some extra brittleness to
  game updates, such as more lookups.
- **Special handling:** handling beyond the horse's, like the removed position write, can come back, but only for
  mounts whose charts don't read `key_brake`. If the mod can answer `T10` and `TC9` itself (below), the chart makes its
  own transitions and the position write shouldn't be needed.

**The fix answers `T10` as true for the mount** from the press until the next slowing state, with the stick left as
is, so steering continues, and `TE` as not holding (below). `key_brake` and drift are untouched.

- **Other mounts:** every ride chart enters its slowing node through `T10`. From the action charts, each gait node of the
  bear, camel, warthog, wolf, reindeer, elephant, iguana and both birds branches on `T10` to a node without a clip. That
  node goes to the slowing node on `OR T122(0, 50) T122(0, 51)`, and otherwise to the stop node. The slowing node's only
  ground exits are its speed check (`TC9`) and skills, so unlike the horse's `C5AF` it doesn't need the condition held.
  Since this is the same `T10` the zeroed stick gives, the landing gaits would match today's straight taps: the bear on
  5.2 and the camel on 3.5.
- **Source:** `tools/chart_node_branches.py CDMW_SRC scratch/out/charts CHART NODE` lists the branches into and out of
  a node, such as `m0002_ride_camel_upper 28`.

- **Where `T10` is evaluated:** found statically. Types below `0x83`, `T10` among them, go through slot 77 of
  `ClientCharacterControlActorComponent`'s vtable, which reads a per-type cache, so replacing that slot can answer `T10`
  for the mount only. See [offsets.md](offsets.md) (Chart condition evaluation).
- **Gait conditions:** gait nodes change on `T167(k, 0)` and `T1E3(mode, value)`, not on speed. The bear's gait nodes
  are `#41`, `#15` and `#30`, with `#16` for the skill gait. Going up needs `key_run` with `T167(3, 0)` or `T167(4, 0)`
  and `T1E3(4, v)`, and going down is on `OR T1E3(5, v) NOT T167(...)`. Its `v` values are 5.0, 9.0 and 13.0. The
  horse's chart uses `T167` 125 times: its canter needs `T167(4, 0)` and its gallop `T167(5, 0)`. `T167` may be the
  gear Shift selects, and `T1E3` may check the mount's stats, which would explain gaits unlocking with level.
  [Stage 43](../tests/stage-43.md) found `T167` answering 0 for every read all session, so it isn't the
  gear. It's probably "gait `k` is unlocked". Mounts other than the horse have no levels, and every gait is available
  from the start (tester), so on them it would always hold.
- **One gait down on `T66`:** in the charts extracted for `0x6AF5D198` (`scratch/out/charts-6AF5D198/`), each gait
  node of every non-horse land ride chart has a branch to the gait below on `TBA(0.5,4)`, then `AND T66 NOT key_run`:
  the bear's `#16`, `#30`, `#15`, `#41`, the camel's `#27` and `#23`, `#13`, `#35`, `#24`, and the wolf's `#24`, `#20`,
  `#3`, `#15`. The bull, elephant, reindeer, warthog, cucu bird and Carma bird charts have 5 or 6 `T66` branches each.
  The horse's chart has one, from its fast gallop `#220` to `#128`. `T66` is type `0x66`, read through slot 77, and
  the game answered 1 (not holding) on every mount in stages 38 and 43. What it means in game isn't known.
- **The `T66` ladder in every eligible chart:** each of the nine non-horse land ride charts has four gaits in a line,
  where Shift (`key_run`) goes up a step and `T66` without Shift goes down exactly the step Shift came up. The lowest
  has no `T66` branch. Gaits from lowest to highest, by node:

  | Chart | Gaits | Other nodes stepping down on `T66` |
  | --- | --- | --- |
  | Bear | `#41`, `#15`, `#30`, `#16` | `#0` to `#30` |
  | Camel | `#24`, `#35`, `#13`, `#23` | `#27` to `#13` |
  | Reindeer | `#39`, `#15`, `#30`, `#41` | `#3` to `#30` |
  | Elephant | `#6`, `#9`, `#23`, `#1` | `#3` to `#23` |
  | Warthog (battle boar) | `#42`, `#29`, `#44`, `#4` | `#15` and `#23` to `#44` |
  | Wolf | `#15`, `#3`, `#20`, `#8` | `#6` and `#24` to `#20` |
  | Iguana | `#5`, `#8`, `#27`, `#37` | `#31` to `#27` |
  | Carma bird | `#25`, `#37`, `#13`, `#39` | `#10` to `#13` |
  | Cucu bird | `#14`, `#9`, `#12`, `#8` | `#19` to `#12` |

  Every one of these branches is `TBA(0.5,4)`, then `AND T66 NOT key_run`, except one each on the wolf and warthog:
  `AND TBA(2,3) NOT T1E4(0, 0)`, then `AND NOT key_run T66`. `TBA(0.5,4)` is probably at least 0.5 s in the state,
  so a step down would wait for that after a Shift tap. The wagon isn't included.
- **Chart numbering on `0x6AF5D198`:** `AND`, `OR` and `NOT` are `0x28B`, `0x28C` and `0x28D`, so the interpreter's
  tokens in [offsets.md](offsets.md) are confirmed. Types from about `0x1E3` up moved by 2 or 3, such as `T1E3` to
  `T1E5` and `T27E` to `T281`. `T10`, `TE`, `T66`, `T167`, `TC9` and `key_brake` kept their numbers.
  `tools/chart_node_branches.py` and `tools/chart_branches.py` pick the numbering from the chart.
- **Chart numbering on `0x6B07E5E9`:** Verified on `0x6B07E5E9`. `AND`, `OR` and `NOT` moved again, to `0x28E`, `0x28F`
  and `0x290`. `T10`, `TE`, `T66`, `0xEA` (`key_*`) and `key_brake` kept their numbers in the horse and wolf charts. The
  charts use CDMW's new `2.06` layout, which CDMW reads from commit `c504d77`; older checkouts misread it as non-empty
  ShareValue sections. `tools/check_charts.py` knows `0x28E`; `chart_node_branches.py` and `chart_branches.py` still only
  know `0x287` and `0x28B`.
- **One-gait drops on other mounts:** `T10` gets them into their slowing node, but they still leave it at the node's
  fixed `TC9` speed. Answering that `TC9` as true at the next gait's speed, while the chart is in the slowing node, could
  make them leave one gait down. `TC9` is type `0xC9`, so it goes through the handler table `InstallBrakeKey` patches.
  This wasn't tried: the `T66` step down does it instead ([slowdown.md](../slowdown.md)).

- **Encoding ([stage 38](../tests/stage-38.md)):** slot 77 returns 0 when `T10` holds and 1 otherwise, and
  `T10` is "no move input". Any stick input counts, including x alone and a sent y of 0.03. The horse, bear, camel and
  wolf all read `T10` through slot 77, and every change to 0 came on the tick the chart entered a slowing state. So
  answering 0 for the mount should be enough to send each chart into its slowing node.

- **Answering slot 77 alone ([stage 39](../tests/stage-39.md)):** fails. The chart enters the slowing state
  on the answer and leaves it on the next frame, every 30 to 50 ms, on the horse from a canter and on the wolf, with or
  without steering. The check that keeps the chart in the slowing state, the in-branch's second condition program
  (`OR T10 key_brake` on the horse), doesn't see the answer. Gallop taps pass through `D85C` and work.
  [Stage 40](../tests/stage-40.md) found that only the interpreter's entry check reads `T10` through slot 77,
  once per entry. [Stage 41](../tests/stage-41.md) wrote 0 to the cache byte too, and it still flipped.
- **What sends the chart back:** the horse's slowing states branch back up to a gait on `TE` (type `0x0E`) and not
  `key_brake`, such as node 0 to node 82: `AND T167(4, 0) TC9(4.2,4)`, then `AND OR TE AND TE key_run NOT key_brake`.
  `TE` reads 0 for the mount while there's stick input, so it looks like "move input", the opposite of `T10`. A zeroed
  stick fails it and a held brake key fails `NOT key_brake`, which is why both stay down, and why a brake key tap without
  the mod bounces. [Stage 42](../tests/stage-42.md) answered `TE` as not holding during a gap as well, and
  passed on the horse and bear: no flipping, one gait per tap on the horse, and turning at the usual 52 to 74°/s.

A mount keeps turning in its slowing node with x held (stage 42). The answer is limited to the mount actor, as
`OnKeyCondition` does, so whether on-foot charts use `T10` doesn't matter. It replaces the gap's zeroed stick on every
mount, horse included, whenever slot 77 is found.

## Reading and overriding keys

Build 1.0.0.3479, unless a bullet names another.

- **Key check:** `exe+0x3B0150` is the handler for condition type `0xEA`. The handler table comes from `exe+0x3A9FC0`, at index
  `type - 0x83`. The handler reads the key hash from the condition at `+8` and the actor from the context at `+8`. `key_brake`'s
  hash is `0x377B9DB8`. Its live code starts with a jump to a non-module address (`0x13FFD003C` in the logged runs), so something
  hooks it at runtime. It returns 0 when the key is pressed: the game's answer was 1 except during brief presses, and answering 0
  on S started drifts in game. Replacing the table slot worked alongside the existing hook. That code was removed with the brake and is saved in
  `scratch/probe-runs/stage5-brake/s-brake.patch`.
- **Condition table on later builds:** the getter is Denuvo-mutated on every build, so `HorseSteer` finds it through
  a call in the condition dispatcher, `g_conditionTableCall`, which also checks the type is below the condition type
  count. The count is `0x286` up to `0x6AED2E85` and `0x28A` on `0x6AF5D198`, so the signature has a wildcard for it.
  On build
  `0x6AEB7D59` the getter is `0x3AA5C0` and the key handler `0x3B0750`. Replacing the slot works there alongside the
  other mod's hook, and on the wagon the actor at the context's `+8` is the wagon ([stage 15](../tests/stage-15.md)).
- **Installed mods:** Combat Evolved swaps riding charts such as `ride_upper` and `ride_weapon_upper`, and its strings show
  no movement or steering code. It logs a game key check at `exe+0x3B0150` on build 1.0.0.3479, which may help with reading
  game input.
- **Key query (Horse Tap Brake, checked against build 1.0.0.3479's exe):** the signature has one match, `exe+0x30C5640`, which takes a
  key hash and returns true if the key is held. It's slot 8 of the input object's vtable. That object comes from the first entry
  of the list at `[[WorldSystem+0xD8]+0x20]` (count at `+0x28`), then `+0x10`.
- **Key hash (Horse Tap Brake, checked against build 1.0.0.3479's exe):** Jenkins lookup3 with seed `0xC5EDE` on the lowercase name,
  which reproduces the `key_brake` hash above. The mod overrides the result of the slot 8 call to hold a key, so the
  `0xEA` handler table doesn't need patching. This only covers actions read through that query, so it can't replace the stick
  changes.
- **Key query in `HorseSteer`:** `BrakeKeyHeld` calls slot 8 from `OnStickRead`, and it answers for Alt, mouse X2 and
  gamepad B in game. On build `0x6AED2E85` the query is `exe+0x30CBB40`. It only reads a key map, and the path to its
  object is the one in the key check handler, `exe+0x3B0A30`, so the `keyCheck` signature covers the path and the slot.
