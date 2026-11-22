# Status

Current focus, half-done work and gotchas for the next session. Rewritten at the end of each session. Task state is in
[state/tasks.json](state/tasks.json), tests in [state/tests.json](state/tests.json), environment in
[state/environment.json](state/environment.json), and history in git.

## Focus

- **Boats:** the tester reaches a ship (`boats-reach-ship`). [boats/plan.md](boats/plan.md) says how, and what the
  sailing test after it checks.
- **Land mounts:** 1.4.1 is on Nexus. The work waits on player reports and the next game update.

## Half done

Nothing.

## Gotchas

- **Game update:** when the session start check reports a changed exe, follow the `game-update` skill.
- **Extracts:** the ones in `scratch/out/` that the session start check lists predate the current build. Regenerate
  them with `tools/` before relying on them.
