# Status

Where each piece of work stands. Keep this page to the table and the game state, and put detail in each work's own doc.

| Work | State | Next | Detail |
|---|---|---|---|
| **Boats** (current focus) | Game files read. No in-game results and no code | Tester reaches a ship and plays it with the mod inactive | [boats/status.md](boats/status.md) |
| **Land mounts** (`HorseSteer`, published as Proper Horse Controls) | v1.3.0 is tagged, packed and on Nexus | Player reports and the next game update | [horsesteer-status.md](horsesteer-status.md) |

## Game state

- **Exe:** PE timestamp `0x6AED2E85` (2026-11-06), identical to the copy in `scratch/exe-baseline/`.
- **DLC:** `dlc_01` (Charting the Unknown) installed 2026-11-08 in `dlc/dlc_01/`, with the exe unchanged.
- **After a game update:** run `tools/check_lookups.py` on the new exe.
- **Stale extracts:** `scratch/out/index.tsv`, `scratch/out/charts/` and `scratch/out/loc_eng.json` predate the 2026-11-06
  update and the DLC. Regenerate them with `tools/` before relying on them.
