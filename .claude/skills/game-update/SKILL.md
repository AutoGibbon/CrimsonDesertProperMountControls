---
name: game-update
description: Check a new Crimson Desert build for changes that break Proper Horse Controls (HorseSteer), using the static checks in tools/, then record the build. Use when the user says the game has updated, or asks to check for drift, offsets or lookups after an update.
---

# Check a game update

Run every command from the repo root with `scratch/venv/Scripts/python`. `GAME` is
`Z:\SteamLibrary\steamapps\common\Crimson Desert`, and `BASE` is the newest exe in `scratch/exe-baseline/`
(`CrimsonDesert_<timestamp>.exe`).

## Checks

Run all three, even after one fails.

1. `tools/check_lookups.py GAME/bin64/CrimsonDesert.exe BASE`. The first line gives the new PE timestamp. If it's
   already in `KNOWN`, the exe hasn't changed, so skip steps 1 and 3, but still run step 2, because the charts ship
   separately from the exe.
2. `tools/check_charts.py GAME scratch/cdmw-src scratch/out/charts-check`. The wagon reports `UNCHECKED` every time.
3. `tools/check_fields.py BASE GAME/bin64/CrimsonDesert.exe`.

## If a check fails

Report what failed and what it breaks in game, then stop. Don't change the mod until the user agrees on a fix.

| Failure | In game | Where to look |
| --- | --- | --- |
| Required lookup (`vt*` except `vtRodeoUi`, `worldSystemSlot`, `physicsSlot`, `hookSite`, `stickSite`) | `MOD INACTIVE` | Signature bytes in both exes with `tools/disasm.py`, then the porting route in `docs/findings/offsets.md` |
| Optional lookup (`keyCheck`, `conditionTableCall`, `cachedCondition*`, `vtRodeoUi`) | That feature drops out; the mod stays active | As above. On `0x6AF5D198`, `conditionTableCall` failed because the condition type count changed |
| `check_charts.py` | Braking flips in and out of slowing, or does nothing | `docs/findings/charts-and-braking.md`; `STEP_DOWN` must match `GroundMount::stepDown` in `HorseSteer.cpp` |
| `check_fields.py` chain | A read returns a wrong pointer: no steering, a wrong mount, or no brake key | Disassemble use sites of the old and candidate offsets in both exes with `tools/disasm.py` |
| `check_fields.py` rodeo slot | The mod doesn't pause in the taming minigame | Candidate slots it prints; `docs/findings/input-and-steering.md` |

A failed chain is a lead, not proof. On `0x6AF5D198`, `+0x40 -> +0x28` fell from 43 to 13 and the mount name read
still worked, while the true shift, the physics object from `+0x2C0` to `+0x2D0`, also showed up as a fall to 1.

## If all checks pass

1. Copy the exe to `scratch/exe-baseline/CrimsonDesert_<timestamp>.exe`, with the timestamp in uppercase hex as in the
   existing names. Keep the older copies.
2. Add the build to `KNOWN` in `tools/check_lookups.py` with the addresses it printed, commented
   `# Steam build of <date>, not yet tested in game`, and run step 1 again to confirm every line reads `ok`.
3. Update the exe line under "Game state" in `docs/status.md`, and add the build to the paragraph on game updates in
   `docs/horsesteer-status.md`.
4. Propose an in-game smoke test: `Logging=1` in `bin64\ProperHorseControls.ini`, ride the horse, and check that
   `bin64\ProperHorseControls.log` reaches `MOD ACTIVE: all features enabled.` If the user agrees, write it as a new test
   in `docs/horsesteer-tests/`.
