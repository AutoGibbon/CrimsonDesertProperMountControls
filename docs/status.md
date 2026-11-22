# Status

Where each piece of work stands. Keep this page to the table and the game state, and put detail in each work's own doc.

| Work | State | Next | Detail |
| --- | --- | --- | --- |
| **Boats** (current focus) | Game files read. No in-game results and no code | Tester reaches a ship and plays it with the mod inactive | [boats/status.md](boats/status.md) |
| **Land mounts** (`HorseSteer`, published as Proper Horse Controls) | v1.3.0 is on Nexus. v1.3.1 is tagged and packed. v1.3.2 (game update `0x6AF5D198`) is tagged and packed. v1.4.0 (braking while turning, one-gait drops on every mount) passed stage 45 and is tagged and packed. v1.4.1 (follower riding, steering release ramp, hold brake) passed stage 49 and is tagged and packed | Upload v1.4.1 | [horsesteer-status.md](horsesteer-status.md) |

## Game state

- **Exe:** PE timestamp `0x6AFDC0CF` (2026-11-20), identical to the copy in `../_shared/exe-baseline/`. v1.4.1 works on
  it unchanged. The previous builds, `0x6AF5D198` (2026-11-13) and `0x6AED2E85` (2026-11-06), are kept there too.
- **DLC:** `dlc_01` (Charting the Unknown) installed 2026-11-08 in `dlc/dlc_01/`, with the exe unchanged.
- **Save for tests:** the tester loads save slot 1. It's edited to add mounts other than the horse when a test needs
  them.
- **After a game update:** follow the `game-update` skill (`.claude/skills/game-update/SKILL.md`), which runs
  `tools/check_lookups.py`, `tools/check_charts.py` and `tools/check_fields.py` and records the new build.
- **Stale extracts:** `scratch/out/index.tsv`, `scratch/out/charts/` and `scratch/out/loc_eng.json` predate the 2026-11-06
  update and the DLC. Regenerate them with `tools/` before relying on them.
