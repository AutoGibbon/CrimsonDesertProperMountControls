# Boats plan

The goal is ship-relative steering, if ships turn out to have the same camera-linked movement as ground mounts. Evidence
so far is in [static-findings.md](static-findings.md), and the tasks are the `boats` items in
[../state/tasks.json](../state/tasks.json).

## Reaching a ship

The game's text says Vellua must be liberated before a ship can be built there, that the Shipbuilding mission is under a
port's Missions menu on the world map, and that required crew roles, including a Rigger, must be assigned through the
Ship Manager before setting sail.

## Sailing test

Set `Logging=1` in `bin64\ProperHorseControls.ini`, then sail with the mod inactive. Fill in the "In game" column of
[the predictions table](static-findings.md#predictions-to-check-in-game), starting with 2 to 5, and read the ship's
package name from the `mount package not recognised` log line. Copy the logs to `scratch/probe-runs/` before they can be
deleted. Write the test as a new file in [../tests/](../tests/), and point `boats-sail-1` in
[../state/tests.json](../state/tests.json) at it.

## Decision

If steering is camera-linked, trace the movement code and see whether the mod's `MoveAnalog` rewrite works on a ship.
Speed comes from the sails, so the forward and S-tap logic may not apply. Record the decision here.

The open questions are predictions 2, 6, 15 and 16 in the predictions table: whether steering follows the camera,
which keys change speed, whether the canoe and small boats steer the same way, and whether the DLC water mounts need the
same work.
