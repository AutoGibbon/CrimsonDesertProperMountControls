# Boat work status

The goal is ship-relative steering, if ships turn out to have the same camera-linked movement as ground mounts. Evidence
so far is in [static-findings.md](static-findings.md).

## Where it stands

- **Done:** read the ship charts, vehicle data, input map and the DLC's sailing text. The findings end with 16 predictions.
- **Not done:** nobody has sailed a ship, so whether steering is camera-linked is unknown. No mod code exists for boats.
- **Mod today:** inactive on ships. `CD_R0014_Boat` isn't in `g_groundMounts`.

## Next

1. **Tester:** reach a ship. The game's text says Vellua must be liberated before a ship can be built there, that the
   Shipbuilding mission is under a port's Missions menu on the world map, and that required crew roles, including a
   Rigger, must be assigned through the Ship Manager before setting sail.
2. **Tester:** set `Logging=1` in `bin64\ProperHorseControls.ini`, then sail with the mod inactive. Fill in the "In game"
   column of the predictions table, starting with 2 to 5. Copy the logs to `scratch/probe-runs/` before they can be
   deleted.
3. **Compare:** correct `static-findings.md` wherever play contradicts it, and read the ship's package name from the
   `mount package not recognised` log line.
4. **Decide:** if steering is camera-linked, trace the movement code and see whether the mod's `MoveAnalog` rewrite works
   on a ship. Speed comes from the sails, so the forward and S-tap logic may not apply.

Write in-game tests as new files in [../horsesteer-tests/](../horsesteer-tests/) and add each to that README.

## Open questions

- Is the wheel steering camera-relative or ship-relative? Prediction 2 settles this.
- Which keys change speed? The tutorial names `Key_Dash` and `Key_Brake`, while `r0014_boat_lower` reads `key_run_once`
  and `key_evade`.
- Do large ships, the canoe and small boats all steer the same way?
- Do the water mounts in `vehicleinfo.dlc_01` need the same work?
