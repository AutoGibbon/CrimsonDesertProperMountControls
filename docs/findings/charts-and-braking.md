# Action charts, keys and braking

Build 1.0.0.3479. `tools/chart_branches.py CDMW_SRC CHART_DIR` prints the horse chart's `key_brake` branches, the branches into
the slide and drift states, and its skill conditions, with conditions decoded.

## Verified from game data

- **Rotate events:** the horse chart has 76 unique `Rotate` frame events. Of 2,911 across all charts, 2,905 share the
  same header bytes, and no value there selects between camera, input and character frames.

## Hypotheses

- `Rotate` events set turn rates and angle limits while native code supplies the target direction. This is inferred from
  values such as 6.98 rad/s and 180.

## Brake and drift

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
  untraced.

## Slowing a mount with S

Constraints from the tester, for any S-based slowdown:

- **Don't use `key_brake`.** Zeroing the forward stick is the same input as releasing W, so it can't reach the slide and
  drift branches that need `key_brake`, on any mount's chart.
- **Shift and dash are the same input,** so `key_run` and `key_dash` go together. The `T10` branches out of gallop nodes
  `#24` and `#110` need both released, but with Shift held in stage 21 a stick gap still left state `E38E`, and
  `mount_value` returned to 14 within about 1 s.
- **A gait only rises with Shift, and holds once Shift is released.** A mount skill gives faster travel for a time past a
  gallop, and needs W and Shift held.
- **Mount speeds can't be a static table.** Mounts differ in stats, several horse breeds have different top speed and
  acceleration, and trust level changes those further. Maximum stats also take riding time to unlock, so logging each
  mount in live play isn't practical. Anything scaled to a mount, such as a position write, must be derived from the
  current mount as it's ridden.

Results from [stage 21](../horsesteer-tests/stage-21.md), on the horse:

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
- **Ending the gap at the next lower gait's speed ([stage 22](../horsesteer-tests/stage-22.md)):** W returning partway
  through the shared slowing node does resume a higher gait. On the bear, 12 of 12 taps from 13.3, 9.2 and 5.2 landed
  one gait lower. On the camel, 3.5 to 2.6 worked, but 9.0 released at about 6.1 m/s landed on 3.5, for a cause not
  traced. With no lower gait to target, a tap at the lowest gait still runs to the time cap.
- **Position write at `AssistPerSecond=0.4` on those mounts:** shortens the gaps by about a quarter, peaking at 2.0 m/s,
  and the tester saw no problem with the feet.

Horse slowing chain, with each node's exit speed and the low hex digits of its state: `#38` `E83C` (12), `#119` `D85C`
(9.5), `#0` `C5AF` (5.2), `#36` `B7CC` (2.8), `#186` `A73F` (near 0). A stop after releasing W from each gait, median time
to 0.5 m/s: walk 1.75 s, trot 2.84 s, canter 4.27 s, gallop 4.38 s, top gait 4.54 s. The last 2.8 m/s takes about 2.4 s
of a gallop stop.

Every other ride chart has one slowing node for all gaits, from static charts and 1 to 5 logged stops per mount. The
elephant has no logged stops. The charts have 40 to 50 nodes, against the horse's 281.

| Chart | Slowing node | Exits below, m/s |
| --- | --- | --- |
| Bear | `#39` | 4.5 |
| Camel | `#28` | 5.0 |
| Warthog | `#37` | 5.0 |
| Wolf | `#17` | 5.0 |
| Reindeer | `#9` | 3.0 |
| Elephant | `#20` | 3.0 |
| Carma bird | `#36` | 4.0 |
| Cucu bird | `#39` | 3.5 |
| Iguana | `#7` | 2.5 |

The landing gait doesn't follow from the exit speed: the bear's 4.5 lands on 5.2, and the camel's 5.0 lands on 3.5.

## Reading and overriding keys

- **Key check:** `exe+0x3B0150` is the handler for condition type `0xEA`. The handler table comes from `exe+0x3A9FC0`, at index
  `type - 0x83`. The handler reads the key hash from the condition at `+8` and the actor from the context at `+8`. `key_brake`'s
  hash is `0x377B9DB8`. Its live code starts with a jump to a non-module address (`0x13FFD003C` in the logged runs), so something
  hooks it at runtime. It returns 0 when the key is pressed: the game's answer was 1 except during brief presses, and answering 0
  on S started drifts in game. Replacing the table slot worked alongside the existing hook. That code was removed with the brake and is saved in
  `scratch/probe-runs/stage5-brake/s-brake.patch`.
- **Condition table on later builds:** the getter is Denuvo-mutated on every build, so `HorseSteer` finds it through
  a call in the condition dispatcher, `g_conditionTableCall`, which also checks the type is below `0x286`. On build
  `0x6AEB7D59` the getter is `0x3AA5C0` and the key handler `0x3B0750`. Replacing the slot works there alongside the
  other mod's hook, and on the wagon the actor at the context's `+8` is the wagon ([stage 15](../horsesteer-tests/stage-15.md)).
- **Installed mods:** Combat Evolved swaps riding charts such as `ride_upper` and `ride_weapon_upper`, and its strings show
  no movement or steering code. It logs a game key check at `exe+0x3B0150` on this build, which may help with reading
  game input.
- **Key query (Horse Tap Brake, checked against this build's exe):** the signature has one match, `exe+0x30C5640`, which takes a
  key hash and returns true if the key is held. It's slot 8 of the input object's vtable. That object comes from the first entry
  of the list at `[[WorldSystem+0xD8]+0x20]` (count at `+0x28`), then `+0x10`.
- **Key hash (Horse Tap Brake, checked against this build's exe):** Jenkins lookup3 with seed `0xC5EDE` on the lowercase name,
  which reproduces the `key_brake` hash above. The mod overrides the result of the slot 8 call to hold a key, so the
  `0xEA` handler table doesn't need patching. This only covers actions read through that query, so it can't replace the stick
  changes. Nothing here has been tested in game with `HorseSteer`.
