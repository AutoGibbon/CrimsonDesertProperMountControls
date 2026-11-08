# Crimson Desert horse control mod

Investigation into, and potentially a native ASI mod for, replicating RDR2's horse-relative controls in Crimson Desert.
See `docs/status.md` first for where each piece of work stands and what's next. It links `docs/horsesteer-status.md` for
land mounts and `docs/boats/status.md` for ships. `docs/discovery.md` has requirements,
`docs/findings/` has evidence by topic, `docs/boats/` has ship evidence, `docs/horsesteer-tests/` has tests and their
results, and `docs/horseprobe/` has the finished probe's docs.

## Paths

- Game: `Z:\SteamLibrary\steamapps\common\Crimson Desert` (executable: `bin64\CrimsonDesert.exe`)
- Existing mod tooling: `E:\Tools\mods\crimson desert\DMM` (`DMM.exe`, with installed mods in `mods\`)
- Installed ASI mods and their `.ini` files: the game's `bin64\`

## Layout

- `docs/`: status, requirements, findings and tests
- `tools/`: scripts and utilities for analysis and builds. Scripts that import CDMW take the path to a
  [CDMW-Full](https://github.com/Ratty123/CDMW-Full) checkout and need Python 3.11+ with `lz4`, `cryptography`, `Pillow`
  and `numpy`. Run them with `scratch/venv/Scripts/python`, which also has `capstone`.
- `mod_page/`: the Nexus page's BBCode description and changelog.
- `src/`: mod source. `HorseSteer` is the mod, published as Proper Horse Controls and built as `ProperHorseControls.asi`. `HorseProbe` is a finished diagnostic, kept as a template for watches and scans.
- `scratch/`: git-ignored and managed by the agent; put game files, extracts and dumps here so they are not committed. Don't commit it or `build/`.
  - `exe-baseline/`: the current exe. It can't be recreated after a game update, so don't delete it.
  - `probe-runs/`: saved test logs. Copy each test's logs here before they can be deleted or overwritten.
  - `cdmw-src/`: CDMW source at commit `585ac51`, with no `.git`. To recreate it, run
    `git clone https://github.com/Ratty123/CDMW-Full scratch/cdmw-src`. `cdmw-release/CDMW.exe` is downloaded but has never been run.
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
`bin64\dmm\manifest.tsv` before suspecting the mod (see `docs/findings/stability.md`).

For a release, run `python tools/pack.py`. It builds, then writes `build/ProperHorseControls-<version>.zip` with
`ProperHorseControls/` holding the asi, the ini, `LICENSE` and `THIRD_PARTY_NOTICES.txt`, with `Logging=0` in the zipped ini whatever the source has.

## Working notes

- **Tests:** write each new test as a new file in `docs/horsesteer-tests/`, which also explains the logs. `HorseSteer` only traces with
  `Logging=1` in `ProperHorseControls.ini`, and the zip from `tools/pack.py` ships with it off, so a test's setup must turn it on in `bin64`. `LogProblem`
  writes regardless, and launching with `Logging=0` deletes earlier logs, so copy them first.
- **Mods:** other mods stay enabled during tests unless the tester agrees otherwise.
- **Updates:** the game can update at any launch, and Steam's update setting doesn't prevent it.
- **Static analysis:** `tools/disasm.py` reads the on-disk exe, and addresses in `.sxdata` disassemble cleanly there, so
  Ghidra has not been needed.
- **Not installed:** Cheat Engine, Ghidra and x64dbg. Ghidra 12.2 needs Java 25 and this machine has JRE 17, so add a
  portable JDK under `scratch/` if it's needed. Installing Cheat Engine needs the user's admin prompt:
  `choco install cheatengine -y`.
