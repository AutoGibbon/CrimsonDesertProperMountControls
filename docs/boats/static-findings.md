# Boats: findings from game files

Nobody has sailed a ship for this project yet. Everything here comes from the installed files, with DLC `dlc_01`
(Charting the Unknown) installed, on exe PE timestamp `0x6AED2E85`. Record what happens in game in the last column of
[Predictions to check in game](#predictions-to-check-in-game).

Each claim is one of these:

- **Stated:** the game's own text says so.
- **Data:** read directly from a game file.
- **Inferred:** a conclusion drawn from data, which could be wrong.

## Controls the game states

The sailing tutorial is `ui.dlc_01` key `15508784249070158230`:

> Once preparations to set sail with your crew are complete, take the ship's wheel to steer the ship directly with
> `{Key:MoveAnalog_ForGameAdvice}`.
>
> Unfurl the sails with `{Key:Key_Dash}` and furl them with `{Key:Key_Brake}` to adjust your speed.
>
> While sailing, your vessel will be affected by sea breezes, which vary in direction and intensity depending on the
> time and location.
>
> When wind blows from behind, the square sails unfurl, allowing you to sail with greater ease aided by the wind. When
> sailing into a headwind, the square sails will not unfurl, and you will sail unaffected by the wind.

The tutorial doesn't say whether the stick is read relative to the camera or the ship.

Other stated controls and rules:

- **Cannons (`ui.dlc_01` `8257447816268349846`):** `Key_SubAttack` adjusts the camera to aim to port or starboard, and
  `Key_HardAttack_DownOnce` fires. The number of cannons fired depends on the Gunners assigned.
- **Seabed scan (`ui.dlc_01` `7246223149681869206`):** `Key_Skill_8_Start` starts it, and it continues "even if you let
  go of the ship's wheel".
- **Unmanned wheel (`knowledge`):** a crew role "takes the helm if wheel is left unmanned during combat".
- **Crew:** a ship can't set sail without the required roles. "A Rigger must be assigned to set sail", and the Rigger
  "furls and unfurls the sails as you change speed". One role "rows to control the ship's direction and speed".
- **Ship skills (`knowledge`):** Ship Movement Lv. 1 to 3 (`MoveLv1` to `MoveLv3`), Boost (`MoveDash`), Drift
  (`MoveDrift`), and a "Maneuverability" stat.

## Files and tables

All **Data**. The DLC adds no `.paac` files and doesn't replace any chart, `ui/inputmap.xml` or the boat vehicle set, so
ships use the charts that were already in the base archives.

| What | Where |
|---|---|
| Ship and boat charts | `upperaction/4_riding/r0014_boat_upper`, `loweraction/2_mon/r0014_boat_lower` |
| Rider's chart at the wheel | `upperaction/1_pc/1_phm/ride_boat_upper` |
| Packages | `CD_R0014_Boat`, `CD_R0014_Boat_Lower`. Separate: `CD_R0000_Canoe_001`, `CD_AirShip` |
| Hulls | `cd_r0014_*`, including big ship, galley, panok and turtle ship variants |
| Sails | `cd_r0048_00_ship`: 215 animations in the base archives, 25 `.pac` models in the DLC |
| Wheel objects (DLC) | `gimmick_ship_steeringwheel_0001` to `0003`, `gimmick_ship_steeringchair_0001` |
| Base `vehicleinfo` | `Ship` (with `steeringwheel`), `Canoe` (with `platform`), `Boat` |
| `vehicleinfo.dlc_01` | `Shark`, `Orca`, `SunFish`, `Seal`, `Turtle`, `MantaRay`, `Dolphin`, `ScooterWarmachine` |
| Vehicle set | `cd_r0014_00_boat_vehicle.xml`: `_simulationType="BoatVehicle"`, `_maxSteerAngle` 45, `_maxSpeed` 100 |

`cd_r0014_00_boat_vehicle.xml` is the only boat vehicle set. It has no wheel data, and the wagon's `cd_r0004_00_wagon.xml` has no
`_simulationType`. The exe contains `SailControl`, `Rudder`, `NoGoAngle`, `YawSpeed` and `YawMultiplier` beside the sail
pose code, plus `VehicleSteeringGoalAngle` and `TribeInfo::_isSteeringMovement`. The string `BoatVehicle` isn't in the
exe in plain text.

## Boat lower chart

From `r0014_boat_lower`, 29 nodes. Nearly every node plays the same `cd_canoe_00_00_swim_nor_std_idle_00` clip, so the
ship's motion doesn't come from animation (**Inferred**). The transitions are **Data**. The meanings are **Inferred**.

| From node | Needs | To node |
|---|---|---|
| 21 | `key_run_once`, skill 12063 | 1, then 4 or 22 |
| 4 or 22 | `key_run_once`, skill 12064 | 19, then 8 or 26 |
| 8 or 26 | `key_run_once`, skill 12065 | 9 |
| 9 | `key_evade`, skill 12064 | 19 |
| 8 or 26 | `key_evade`, skill 12063 | 1 |
| 4 or 22 | `key_evade` | 21 |
| 21 | `key_brake` held, skill 12063 | 12, back to 21 on release |
| 9 | `key_skill_9_start`, skill 12071 | 20 |
| 20 | `key_skill_9` released after 1 s, skill 12071 | 5 |
| 5 | after 1 s, `TDF` below -45, skill 12073 | 17 |
| 5 | after 1 s, `TDF` above 45, skill 12073 | 6 |
| 5 | after 2 s, or `key_evade` with skill 12065 | 9 |
| 6 or 17 | `TDF` changes sign, or `TC9` below 6, among others | 9 |
| any | `T25E(0,0)` | 21 |

- **Reading:** 21 is sails furled, 4 or 22, 8 or 26 and 9 are three sail levels, 12 is backing up, 5 is Boost, and 6 and
  17 are Drift. Skills 12063 to 12065 fit Ship Movement Lv. 1 to 3, and 12073 fits Drift.
- **Name table order:** `keyguide_accelerate`, `key_run_once`, `keyguide_wagon_backmove`, `key_brake`,
  `keyguide_deccelerate`, `key_evade`, `keyguide_chargedash`, `key_skill_9_start`, which supports those pairings.
- **Conflict with the tutorial:** the tutorial lowers speed with `Key_Brake`, while the chart steps down on `key_evade`
  and reads `key_brake` only from node 21. It also raises speed with `Key_Dash`, while the chart reads `key_run_once`.
  They may be the same physical keys.

## Steering angle

`TDF` is a signed angle condition. Comparison codes 2, 3, 4 and 5 are inferred as at most, at least, above and below,
from how they pair into ranges. That it's the move angle relative to the mount is **Inferred**.

- **Same use as the horse:** horse node 25 enters `gallop_drift_l_stt` on `TDF` between -125 and -45, with a skill check.
  Boat node 5 enters node 17 on `TDF` below -45, with a skill check.
- **Wheel animation (`ride_boat_upper`):** `turnl_01` and `turnr_01` go to `_02` when `TDF` is past ±45 and within ±100,
  and to `_03` past ±100. They return when `TDF` is back within ±30, or on `T10`.
- **Why that suggests the camera:** A or D alone is ±90 relative to the ship, so `_03` would need S plus a side key if
  the angle were ship-relative. Measured against the camera, any angle can occur.
- **Wagon contrast:** `ride_carriage_upper` and `ride_carriage_lower` use `TDF` only on one dismount branch, and
  `r0012_wagon_upper` doesn't use it.

## Current mod behaviour

`CD_R0014_Boat` and the DLC water mounts' packages aren't in `g_groundMounts`, and `HorseSteer.cpp` leaves water mounts
out, so the mod logs `mount package not recognised` and stays inactive. The tutorial's `MoveAnalog` is the input the mod
already rewrites on land mounts.

## Predictions to check in game

| # | Prediction | Basis | In game |
|---|---|---|---|
| 1 | At the wheel, the move stick or WASD steers the ship | Stated | |
| 2 | With input held, the ship's heading follows the camera | Inferred, the main question | |
| 3 | Swinging the camera with no input doesn't turn the ship | Inferred | |
| 4 | A or D alone turns the ship toward that side of the camera, not port or starboard | Inferred | |
| 5 | W doesn't set speed, and the sails do | Stated for sails, inferred for W | |
| 6 | `Key_Dash` raises speed and `Key_Brake` lowers it. Record the keys that work | Stated, chart differs | |
| 7 | Speed has three sail levels, gated by Ship Movement skills | Inferred | |
| 8 | With sails furled, holding the brake key backs up and release stops it | Inferred | |
| 9 | The `key_skill_9` input boosts, and steering past 45° after 1 s of boost drifts | Inferred | |
| 10 | The rider's wheel pose changes in three steps, the last past 100° | Data, meaning inferred | |
| 11 | Square sails unfurl with wind behind and stay furled into a headwind | Stated | |
| 12 | `Key_SubAttack` moves the camera for port or starboard aim. Note if steering still works | Stated, steering unknown | |
| 13 | The player can let go of the wheel while the ship keeps moving | Stated | |
| 14 | The mod stays inactive and logs the ship's package name | Inferred from source | |
| 15 | The canoe and small boats steer differently from the ship's wheel | Inferred | |
| 16 | The DLC water mounts are rideable and the mod stays inactive on them | Inferred | |

## Not traced

- The code that moves the ship, and what `BoatVehicle` selects in it.
- Whether large ships use `cd_r0014_00_boat_vehicle.xml` or another movement path.
- What `T10`, `T13`, `TC`, `TE`, `T25E` and `T137` test, and the speed at each sail level.

`tools/index_archives.py` labels each package by its folder name, so `dlc/dlc_01/0009` and the base `0009` both show as
`0009`. Use the path, not that column, to tell DLC files from base files.
