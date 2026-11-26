# Status

Current focus, half-done work and gotchas for the next session. Rewritten at the end of each session. Task state is in
[state/tasks.json](state/tasks.json), tests in [state/tests.json](state/tests.json), environment in
[state/environment.json](state/environment.json), and history in git.

## Focus

- **Boats:** the tester reaches a ship (`boats-reach-ship`). [boats/plan.md](boats/plan.md) says how, and what the
  sailing test after it checks.
- **Land mounts:** 1.4.1 is on Nexus. Game update `0x6B07E5E9` (2026-11-26) passed every static check;
  `update-6b07e5e9` waits on [stage-50](tests/stage-50.md), the in-game smoke test of the deployed 1.4.1 build.

## Half done

- **`update-6b07e5e9`:** blocked until DMM gets an update for `0x6B07E5E9`. When it works, the tester runs stage-50
  with no new build or DMM apply; `bin64\ProperHorseControls.ini` already has `Logging=1`. After a pass, add
  `; in-game test stage-50` to the `0x6B07E5E9` comment in `KNOWN` in `tools/check_lookups.py`.

## Gotchas

- **Game update:** when the session start check reports a changed exe, follow the `game-update` skill.
- **CDMW:** `../_shared/cdmw-src` is at `c504d77`, the first commit that reads `0x6B07E5E9`'s charts. weatherMod hasn't
  been checked on the new build or the new CDMW; the user does that from the weatherMod project.
- **Chart tools:** `tools/chart_node_branches.py` and `tools/chart_branches.py` don't know the `0x28E` logic tokens of
  `0x6B07E5E9` ([charts-and-braking.md](findings/charts-and-braking.md)), so they misread `AND`, `OR` and `NOT` on this
  build until fixed the way `tools/check_charts.py` was.
- **DMM broken:** on `0x6B07E5E9` DMM doesn't work. Before an in-game task, including `boats-reach-ship`, ask the user
  whether the game runs and with which mods.
- **Boats findings build:** [boats/static-findings.md](boats/static-findings.md) predates `0x6B07E5E9`. Re-check the
  addresses and chart tokens its predictions rely on before `boats-sail-1`.
- **Extracts:** the ones in `scratch/out/` that the session start check lists predate the current build. Regenerate
  them with `tools/` before relying on them.
