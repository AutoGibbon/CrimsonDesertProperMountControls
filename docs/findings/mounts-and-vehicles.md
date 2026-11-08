# Mounts and vehicles

Build 1.0.0.3479. How the horse's backward walk works, and how to check it on other mounts, is in
[backward-walk.md](backward-walk.md).

## Verified from game data

- **Horse chart:** `characteractionpackagedescription.xml` maps the `CD_R0001_Horse` `RideOnHorse` slot to
  `UpperAction/4_Riding/R0001_HorseRD2_MoveTest_03_Upper`. No other package has a `RideOnHorse` slot. Other mounts such
  as `m0002_ride_wolf_upper` have their own charts.

- **Package list:** `g_groundMounts` matches every package with a legged ground `RideOn*` slot.
- **Ride charts:** each has one blend file, and its stop node leaves to `walk_s_end` as below. Checked on build 26423880.

| Mount | Package | Ride chart | Blend | Stop node, exit |
| --- | --- | --- | --- | --- |
| Camel | `CD_M0002_Camel` | `m0002_ride_camel_upper` | `monster/camel_move_2cycle` | #16 → #33 |
| Reindeer | `CD_M0002_Reindeer` | `m0002_ride_reindeer_upper` | `monster/reindeer_move_2cycle` | #28 → #21 |
| Elephant | `CD_M0002_Elephant` | `m0002_ride_elephant_upper` | `elephant/elephant_move_2cycle` | #41 → #8 |
| Boar, warthog | `CD_M0002_BattleBoar` | `m0002_ride_warthog_upper` | `monster/battleboar_move_2cycle` | #5 → #18 |
| Bear | `CD_M0002_Bear` | `m0002_ride_bear_upper` | `monster/bear_move_2cycle` | #7 → #19, also has skill branches |
| Wolf | `CD_M0002_Wolf`, `_Elite`, `_Elite_Big`, `_Boss` | `m0002_ride_wolf_upper` | `monster/wolf_move_2cycle` | #1 → #7 |
| Iguana | `CD_R0034_Iguana` | `r0034_iguana_upper` | `monster/iguana_move_2cycle` | #21 → #33 |
| Carmabirdsaurus | `CD_R0035_CarmaBirdsaurus` | `r0035_ride_carmabirdsaurus_00001_upper` | `monster/carmabirdsaurus_move_2cycle` | #9 → #0 |
| Cucubird | `CD_M0003_CucuBird` | `m0003_ride_cucubird_upper` | `monster/cucubird_move_2cycle` | #5 → #10 |

The horse's backward walk on these mounts is covered in [backward-walk.md](backward-walk.md).

## From probe and test runs

- **Mount link:** the rider's `ClientVehicleActorComponent` holds the mount key and seat, which `RideOn::Enter` reads the same way.
- **Mount key:** this save's horse key is `0xB0100002`. Key `0xB0100003`, from captures on build 2976, was not found.
- **Mount scan:** the horse's action package name is `CD_R0001_Horse`. Its appearance path is at a different chain from the protagonists'. On build 3479, inline text from the mount was at: appearance `[[[mount+0x68]+0x70]+0x188]+0x18`, model `[[[[mount+0x68]+0x40]+0x130]+0x280]+0x18`, description `[[[[mount+0x68]+0x40]+0x220]+0x18]+0x18` and ride chart `[[[[mount+0x68]+0x48]+0x140]+0x18]+0x18`. `ReadMountIdentity` logs these.
- **Package chain:** `[[[[mount+0x68]+0x40]+0x28]+0x18]+0x48` only holds the name on some mounts, and reads garbage on others. In stage 11b, the same object held the lowercase name at `+0x18` on the camel, reindeer, snow deer, boar, wolf, iguana, carmabirdsaurus and ibex. Stage 12 confirmed it on all 13 mounts ridden, also including the horse, bear, tiger, lion and cucubird.
- **Appearance chain:** `[[[[mount+0x68]+0x40]+0x40]+0x38]+0x18` holds the appearance path, which separates mounts that share a package. In stage 12 it read on every mount, including the camel, where the stage 11b scans didn't show it.
- **Other chains on build 26423880:** the `appearance`, `model` and `ride_chart` chains from the mount scan above read nothing useful. `description` read on every mount, but it gives `animal_horse.xml` for the camel and ibex as well as the horse, so it can't separate every mount.
- **Packages seen in game:** the snow deer uses `CD_M0002_Reindeer`, the wolf, tiger and lion use `CD_M0002_Wolf_Elite`, and the ibex uses `CD_R0001_Horse`. Stage 12's appearance paths confirm the tiger, lion, ibex, cucubird and the rest.
- **Unreached:** the save data has one elephant record, and adding it didn't make the elephant appear in the mount list.
- **Cucubird gliding:** in stage 12b, steering worked as modified while gliding from a cliff.
  The scan was removed from `HorseSteer`; its output is in `scratch/probe-runs/steer5/` and the last version with it is not committed.
- **Stowing under a carriage:** riding the underside of a carriage counts as mounted, with key `0xB01004A7`. `ReadPackageName` returned unreadable text there, so the mod
  stayed inactive, which is correct. That chain doesn't always lead to a name, which matters for carriage work.

## From the vehicle and character tables

`tools/mount_census.py` lists the rows. Grouping characters by the `Riding_` prefix is a naming convention, so it can miss a rideable character.

- **Table layout:** `vehicleinfo` and `characterinfo` rows in `gamedata/binarystaticinfo__/bin/` start with a length-prefixed name, then the key. Headers are a u16 count, then a key and
  u32 offset per row; the key is u16 for vehicles and u32 for characters. The rest of the row layout isn't mapped.
- **`_vehicleInfo`:** not located in `characterinfo` rows. A scan's strongest candidate, the u16 at +111 after the key, is a false positive, as it gives ordinary NPCs vehicle types.
- **Vehicle types:** the table has 34. The legged ones are `Horse`, `GolemHorse`, `Boar`, `Wolf`, `Bear`, `ReinDeer`, `AlpineIbex`, `CuCubird`, `Elephant`, `Iguana`, `CarmabirdSaurus`,
  `Camel`, `Lion`, `Tiger` and `Bull`. The carriage types are `1Horse2Wheels`, `1Horse4Wheels`, `2Horse4Wheels` and `PlayerWagon`.
- **Alpine ibex:** `Riding_AlpineIbex_1` (`0x7308`) and `Riding_AlpineIbex_1000` (`0xF518B`) exist. In game, the ibex uses `CD_R0001_Horse`, so it has no riding setup of its own.
- **Other vehicle types without a `Riding_` row:** `Lion`, `Tiger`, `Bull` and `GolemHorse`. `RideOnTiger` is commented out in the package file and `CD_M0002_Bull` has no `RideOn*` slot, so these may be unused. `m0002_ride_bull_upper` exists, but no package uses it.
- **Donkeys:** 40 character names contain "Donkey", none start with `Riding_`, and `CD_M0002_Donkey` has no `RideOn*` slot, and no donkey record can be written to the save data, so they can't be tested and may not be rideable. `horse/donkey_move` has one backward clip and only a forward speed input.
- **Warthog:** its ride chart `M0002_Ride_Warthog_Upper` is the `RideOnBoar` slot of `CD_M0002_BattleBoar`, so the `Riding_Warthog_*` rows should use that listed package.
- **Carriages:** the 74 `Riding_Wagon_*` rows include cow-drawn wagons (`CowFourWheel`) as well as horse-drawn ones, so test both. On the test save, the tester found many wagons in the world but no option to steal or mount them. This looks like game progress rather than the mod, though it wasn't checked with the mod off.

## Wagon

Build `0x6AEB7D59`. Tested on `cd_r0004_05_wagon` in [stage 14](../horsesteer-tests/stage-14.md) and
[stage 15](../horsesteer-tests/stage-15.md). Logs are in `scratch/probe-runs/wagon-1/` (mod inactive), `stage14/`,
`stage14-steer024/`, `stage14-steer010/` and `stage15/`.

- **Package and charts:** the wagon's package is `CD_R0012_Wagon_0001`, read as `cd_r0012_wagon_0001`, and its chart is
  `r0012_wagon_upper`. The wagon actor is the mount, and its chart reads `key_brake`. The animal pulling it is joined to
  the wagon by an articulated link, so its position and angle aren't fixed to the wagon's.
- **Forward:** replacing the camera vectors steers it with W and A or D, ignoring the camera, at 33 to 40°/s at 7 to 8 m/s.
- **Reverse, chart node 43:** `keyguide_wagon_backmove`. Entered from nodes 6, 42, 51 and 73 with `key_brake`, and left
  on `NOT key_brake`. The CSV shows it as state `E14A`, travelling backward at about 1 m/s.
- **Brake key with A or D:** the wagon stays in `E14A` and turns at about 55 to 60°/s while backing, and its travel drifts
  60 to 80° off straight back. The mod-off run, with taps of 0.3 s or less, turned at about 40 to 50°/s with up to 60°
  drift. This is the game's own behaviour, and the tester rated it poor.
- **Movement is a root transform, not wheel physics:** the chart moves and turns the mount actor's transform directly,
  and `mount_phys_yaw` is that root's heading. It points along the horse: after a tap of A the horse sits at an angle to
  the wagon body and the path runs straight back from the horse, along `mount_phys_yaw`. The wagon body holds that angle
  while reversing, and colliders can move it. With A held, the root turns at 60°/s and the two circle in place. Only the
  wagon's actor reads `key_brake`, and the wagon's prefab holds only its body and harness.
- **Wagon body angle not found (stage 17):** no rotation reachable within 3 pointers of the wagon, the horse, or their
  physics objects showed the body's angle to the horse. Every rotation that moved followed `mount_phys_yaw`. The wagon's
  Havok file (`cd_r0004_05_wagon_0001_vehicle.hkx`) has bodies for the axles, suspension and wheels joined by
  `hkRagdollConstraint`, `hkWheelConstraint` and `hkFixedConstraint`, and no horse body, so the body angle is probably
  one of those bodies. Searching for it was stopped, and the wagon's reverse steering is accepted as the game's.
- **Turn in `E14A` (stage 15 CSV):** the wagon doesn't stop. Within about 0.6 s of A or D, `mount_vel` falls from 1.0
  to 0.5 m/s and stays there, while `mount_phys_yaw` turns at a steady 58 to 60°/s from the first row. That's a circle
  of about 0.5 m radius. With D held for 6 s the wagon turned a full 360° at that rate. In one run with A held, the rate
  fell to about 8°/s after 2.3 s and 140°. In straight reverse, `travel_yaw` matches `mount_phys_yaw`.
- **Taps in `E14A` ([stage 16](../horsesteer-tests/stage-16.md)):** A held for 0.2 to 0.3 s turns the wagon 15 to 18°,
  and a shorter tap 9°. The rate ramps to 60°/s over about 0.3 s and back to 0 over about 0.3 s after release, speed
  dips to about 0.8 m/s, and the wagon then reverses straight along its new heading. Holding A gives 62°/s while speed
  falls to 0.5 m/s.
- **The animal (stage 16):** a `cd_r0001_horse` actor whose key is the wagon's plus 2. The driver NPC is the wagon's key
  plus 1, with a vehicle link to the wagon at seat 0. The horse's vehicle link is empty, and it holds the wagon's key at
  `+0xAC`. The wagon's `+0xB0` points to a list with the driver's key at `+0x0` and the horse's at `+0x4`. The horse's
  physics object (`actor+0x68->+0x40->+0x140->+0x2C0`) reads as the wagon's position and heading, so its visible angle
  to the wagon comes from somewhere else.
- **Rate:** node 43's `Rotate` event has 3.142 and 1.047 among its floats. 1.047 rad/s is 60°/s, which matches the
  measured rate, but the field's meaning isn't confirmed, nor whether it turns the wagon or the animal. The same event record is shared with node 75. The drive nodes 13
  and 23 have 6.283 and 0 in those places.
- **S with A or D through the backward walk:** before stage 15, the mod cleared S, so A or D with no forward input sent
  the wagon to state `C7EF`, a forward pivot at 48 to 59°/s. `SteerInput` at 0.56, 0.24 and 0.1 made no difference, even
  as stick X ramped from 0.03 to 0.1. The tester saw the animal leave reverse for a tight forward turn.
- **S as the brake key (stage 15):** S answers the wagon's `key_brake` as pressed, which reverses it as the brake key
  does, including with A or D. Passed. Other mounts keep the backward walk.

## Wagon slowdown

Build `0x6AED2E85`, stages [26](../horsesteer-tests/stage-26.md) to [28](../horsesteer-tests/stage-28.md). Nodes are from
`r0012_wagon_upper`. That condition `TC9` tests speed is inferred, and matches the logged speeds. State IDs change between
sessions.

| Node | `mount_value` | State in these runs | Role |
| --- | --- | --- | --- |
| 44 | 5.5 | `D93D` | Drive. Up to 11.9 m/s with Shift, and decays to a 5.5 m/s cruise without it |
| 75 | 2.8 | `C7EF` | Low drive, settling at 2.8 m/s |
| 23 | 0 | `0200` | Slowing at 6 m/s or above, moves to node 13 below 5 m/s |
| 13 | 0 | `0724` | Slowing below that, about 1.4 m/s² |
| 51 | missing | `AFAA` | Idle |
| 43 | 1.0 | `E14A` | Brake reverse |

- **`key_brake`:** only idle (51) leads to reverse, and it's tested before forward input. The drive and slowing nodes
  don't read it, so the brake key can start the slowdown on a moving wagon without reversing it.
- **Landing:** with forward input back, nodes 23 and 13 both go to node 75, so a tap from top speed settles at 2.8 m/s and
  skips the 5.5 m/s cruise.
- **Taps from 5.5 m/s:** these go straight to node 13, so the second slowing state only comes at idle, and they ran to
  the 3 s cap at about 1 m/s. On the wagon, a tap from `mount_value` above `kBrakeMountLowValue` also ends on the first
  slowing state below `kBrakeMountEndSpeed` (5 m/s).
- **Taps from `mount_value` 2.8:** a dip to about 0.26 m/s, and back to 2.8 m/s in about 4 s with W held.
- **Shift held:** node 44 leaves on released input only with `NOT key_run`, so a tap at top speed with Shift held does
  little.

## Leads

- **Other movement classes (Horse Tap Brake):** the exe has `VehicleMovementMethod`, `DriverMovementMethod`, `RideOnMovementMethod`, `RideOffMovementMethod` and `MiniGameMovementMethod`.
  These are names only, not traced.
