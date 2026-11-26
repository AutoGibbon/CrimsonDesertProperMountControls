# Status

Current focus, half-done work and gotchas for the next session. Rewritten at the end of each session. Task state is in
[state/tasks.json](state/tasks.json), tests in [state/tests.json](state/tests.json), environment in
[state/environment.json](state/environment.json), and history in git.

## Focus

- **Boats:** the tester reaches a ship (`boats-reach-ship`). [boats/plan.md](boats/plan.md) says how, and what the
  sailing test after it checks.
- **Land mounts:** 1.4.1 is on Nexus and works unchanged on game update `0x6B07E5E9` (2026-11-26,
  [stage-50](tests/stage-50.md)). The work waits on player reports and the next game update.

## Half done

Nothing.

## Gotchas

- **Game update:** when the session start check reports a changed exe, follow the `game-update` skill.
- **CDMW:** `../_shared/cdmw-src` is at `c504d77`, the first commit that reads `0x6B07E5E9`'s charts. weatherMod hasn't
  been checked on the new build or the new CDMW; the user does that from the weatherMod project.
- **Chart tools:** `tools/chart_node_branches.py` and `tools/chart_branches.py` don't know the `0x28E` logic tokens of
  `0x6B07E5E9` ([charts-and-braking.md](findings/charts-and-braking.md)), so they misread `AND`, `OR` and `NOT` on this
  build until fixed the way `tools/check_charts.py` was.
- **Boats findings build:** [boats/static-findings.md](boats/static-findings.md) predates `0x6B07E5E9`. Re-check the
  addresses and chart tokens its predictions rely on before `boats-sail-1`.
- **Extracts:** the ones in `scratch/out/` that the session start check lists predate the current build. Regenerate
  them with `tools/` before relying on them.
