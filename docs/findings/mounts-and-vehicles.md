# Mounts and vehicles

Build 1.0.0.3479. How the horse's backward walk works, and how to check it on other mounts, is in
[backward-walk.md](backward-walk.md).

## Verified from game data

- **Horse chart:** `characteractionpackagedescription.xml` maps the `CD_R0001_Horse` `RideOnHorse` slot to
  `UpperAction/4_Riding/R0001_HorseRD2_MoveTest_03_Upper`. No other package has a `RideOnHorse` slot. Other mounts such
  as `m0002_ride_wolf_upper` have their own charts.

## From probe and test runs

- **Mount link:** the rider's `ClientVehicleActorComponent` holds the mount key and seat, which `RideOn::Enter` reads the same way.
- **Mount key:** this save's horse key is `0xB0100002`. Key `0xB0100003`, from captures on build 2976, was not found.
- **Mount scan:** the horse's action package name is `CD_R0001_Horse`. Its appearance path is at a different chain from the protagonists'.
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
- **Alpine ibex:** `Riding_AlpineIbex_1` (`0x7308`) and `Riding_AlpineIbex_1000` (`0xF518B`) exist, but no ibex package is in `kGroundMountPackages`. `CD_M0002_Goat` is a possible match, but
  has no `RideOn*` slot, and no goat or ibex ride chart exists, so the ibex may not have its own riding setup. Its rows don't hold a package name as readable text. Mounting one
  in game and reading the package name would settle it.
- **Other vehicle types without a `Riding_` row:** `Lion`, `Tiger`, `Bull` and `GolemHorse`. `RideOnTiger` is commented out in the package file and `CD_M0002_Bull` has no `RideOn*` slot, so these may be unused.
- **Donkeys:** 40 character names contain "Donkey", none start with `Riding_`, and `CD_M0002_Donkey` has no `RideOn*` slot, so they may not be rideable. This is untested in game.
- **Warthog:** its ride chart `M0002_Ride_Warthog_Upper` is the `RideOnBoar` slot of `CD_M0002_BattleBoar`, so the `Riding_Warthog_*` rows should use that listed package.
- **Carriages:** the 74 `Riding_Wagon_*` rows include cow-drawn wagons (`CowFourWheel`) as well as horse-drawn ones, so test both.

## Leads

- **Other movement classes (Horse Tap Brake):** the exe has `VehicleMovementMethod`, `DriverMovementMethod`, `RideOnMovementMethod`, `RideOffMovementMethod` and `MiniGameMovementMethod`.
  These are names only, not traced.
