# Crimson Desert horse control mod

Investigation into, and potentially a native ASI mod for, replicating RDR2's horse-relative controls in Crimson Desert.
`docs/status.md` has the current focus and gotchas. `docs/state/` holds task status (`tasks.json`), test status and
results (`tests.json`) and the game build, deployed builds and installed mods (`environment.json`).
`docs/discovery.md` has requirements, `docs/horsesteer-design.md` and `docs/slowdown.md` say how the mod works,
`docs/findings/` has evidence and open questions by topic, `docs/boats/` has the ship plan and evidence,
`docs/horsesteer-tests/` has each test's steps, results and the log formats, and `docs/horseprobe/` has the finished
probe's docs.

## Session routine

- **Start:** the `SessionStart` hook runs `tools/session_start.py` and its output is in context; if it isn't, run the
  script. Raise any problems it reports with the user, then read `docs/status.md`. Work on the `next` items in
  `docs/state/tasks.json` unless the user says otherwise.
- **Starting a task:** agree its `done_when` with the user when setting it `in_progress`. Check the open questions in
  the findings docs the task touches; one it depends on becomes a task or a `done_when` line now.
- **During:** update a JSON entry in `docs/state/` when its state changes, not at the end. When a result answers an
  open question, take it off its list in the same commit. Commit after each finished step, with a message saying what
  changed and why.
- **Done:** record in `evidence` what showed it: a test, a command and its output, or the user's confirmation. For
  code, scripts or data Claude can check without the user, a review by a session that didn't write it (`/code-review`,
  or a read-only reviewer subagent given the task's `done_when`, reporting correctness gaps only and skipping style)
  can help; skip it where it adds cost without catching anything.
- **End** (the `wrap-up` skill): set statuses and `next` flags, add or fill `tests.json` entries, update
  `environment.json`, rewrite `docs/status.md`, commit. Don't end a session with uncommitted work.
- **Rules:** never delete or reword a task or test entry, or change a run test's `checks`, without the user's agreement.
  Mark findings Verified (with the build) or Hypothesis (with a confidence: low, medium or high).

## Paths

- Game: `Z:\SteamLibrary\steamapps\common\Crimson Desert` (executable: `bin64\CrimsonDesert.exe`)
- Existing mod tooling: `E:\Tools\mods\crimson desert\DMM` (`DMM.exe`, with installed mods in `mods\`)
- Installed ASI mods and their `.ini` files: the game's `bin64\`
- Shared tooling: `../_shared` holds the venv, the CDMW checkout and one exe copy per game build, used by every
  Crimson Desert mod project here; its `README.md` says how to recreate each.

## Layout

- `docs/`: status, state JSON, requirements, design, findings and tests
- `tools/`: scripts and utilities for analysis and builds. Scripts that import CDMW take the path to a
  [CDMW-Full](https://github.com/Ratty123/CDMW-Full) checkout and need Python 3.11+ with `lz4`, `cryptography`, `Pillow`
  and `numpy`. Run them with `../_shared/venv/Scripts/python`, which also has `capstone`, and pass
  `../_shared/cdmw-src` as the CDMW checkout.
- `mod_page/`: the Nexus page's BBCode description and changelog.
- `src/`: mod source. `HorseSteer` is the mod, published as Proper Horse Controls and built as `ProperHorseControls.asi`. `HorseProbe` is a finished diagnostic, kept as a template for watches and scans.
- `scratch/`: git-ignored and managed by the agent; put game files, extracts and dumps here so they are not committed. Don't commit it or `build/`.
  - `probe-runs/`: saved test logs. Copy each test's logs here before they can be deleted or overwritten.
  - `cdmw-release/CDMW.exe`: downloaded but has never been run.
  - `out/`: archive index (`index.tsv`), all action charts (`charts/`), English text (`loc_eng.json`) and other extracts,
    which can be regenerated with `tools/`.
  - `caballo.md`: CrimsonDesertCoop's horse research (`docs/research/caballo-coop.md`), written for build 2976.

## Build and deploy

```bash
cmake -S src/HorseSteer -B build/HorseSteer -G "Visual Studio 17 2022" -A x64
cmake --build build/HorseSteer --config Release
cp build/HorseSteer/Release/ProperHorseControls.asi "E:/Tools/mods/crimson desert/DMM/mods/"
```

The tester then turns `ProperHorseControls` off and on in DMM and applies, so that the new build is copied to `bin64`.
DMM doesn't copy `ProperHorseControls.ini` from `DMM\mods`, so put settings changes in the game's
`bin64\ProperHorseControls.ini` directly. Disabling it deletes its logs, so copy them to `scratch/probe-runs/` first. If a save won't load, check for a stale
`bin64\dmm\manifest.tsv` before suspecting the mod (see `docs/findings/stability.md`). After a deploy, update the
artefacts in `docs/state/environment.json`; the session start check compares the deployed asi with `build/` by hash.

For a release, run `python tools/pack.py`. It builds, then writes `build/ProperHorseControls-<version>.zip` with
`ProperHorseControls/` holding the asi, the ini, `LICENSE` and `THIRD_PARTY_NOTICES.txt`, with `Logging=0` in the zipped ini whatever the source has. Once it's on Nexus, set `nexus_version` in
`docs/state/environment.json`.

## Working notes

- **Tests:** write each new test as a new file in `docs/horsesteer-tests/`, which also explains the logs, and add it to
  `docs/state/tests.json`. `HorseSteer` only traces with
  `Logging=1` in `ProperHorseControls.ini`, and the zip from `tools/pack.py` ships with it off, so a test's setup must turn it on in `bin64`. `LogProblem`
  writes regardless, and launching with `Logging=0` deletes earlier logs, so copy them first.
- **Mods:** other mods stay enabled during tests unless the tester agrees otherwise. `installed` in
  `docs/state/environment.json` lists them.
- **Updates:** the game can update at any launch, and Steam's update setting doesn't prevent it.
- **Static analysis:** `tools/disasm.py` reads the on-disk exe, and addresses in `.sxdata` disassemble cleanly there, so
  Ghidra has not been needed.
- **Not installed:** Cheat Engine, Ghidra and x64dbg. Ghidra 12.2 needs Java 25 and this machine has JRE 17, so add a
  portable JDK under `scratch/` if it's needed. Installing Cheat Engine needs the user's admin prompt:
  `choco install cheatengine -y`.
