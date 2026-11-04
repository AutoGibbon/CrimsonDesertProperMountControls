# Crimson Desert horse control mod

Investigation into, and potentially a native ASI mod for, replicating RDR2's horse-relative controls in Crimson Desert.
See `docs/status.md` for where the work stands and what's next, `docs/discovery.md` for requirements,
`docs/findings/` for evidence by topic, `docs/horsesteer-tests/` for tests and their results, and `docs/horseprobe/` for
the finished probe's docs.

## Paths

- Game: `Z:\SteamLibrary\steamapps\common\Crimson Desert` (executable: `bin64\CrimsonDesert.exe`)
- Existing mod tooling: `E:\Tools\mods\crimson desert\DMM` (`DMM.exe`, with installed mods in `mods\`)
- Installed ASI mods and their `.ini` files: the game's `bin64\`

## Layout

- `docs/`: status, requirements, findings and tests
- `tools/`: scripts and utilities for analysis and builds. Scripts that import CDMW take the path to a
  [CDMW-Full](https://github.com/Ratty123/CDMW-Full) checkout and need Python 3.11+ with `lz4`, `cryptography`, `Pillow`
  and `numpy`. Run them with `scratch/venv/Scripts/python`, which also has `capstone`.
- `src/`: mod source. `HorseSteer` is the mod. `HorseProbe` is a finished diagnostic, kept as a template for watches and scans.
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
cp build/HorseSteer/Release/HorseSteer.asi "E:/Tools/mods/crimson desert/DMM/mods/"
```

The tester then turns `HorseSteer` off and on in DMM and applies, so that the new build is copied to `bin64`. Disabling it
deletes its logs, so copy them to `scratch/probe-runs/` first. If a save won't load, check for a stale
`bin64\dmm\manifest.tsv` before suspecting the mod (see `docs/findings/stability.md`).

## Working notes

- **Tests:** write each new test as a new file in `docs/horsesteer-tests/`, which also explains the logs.
- **Mods:** other mods stay enabled during tests unless the tester agrees otherwise.
- **Updates:** the game can update at any launch, and Steam's update setting doesn't prevent it.
- **Static analysis:** `tools/disasm.py` reads the on-disk exe, and addresses in `.sxdata` disassemble cleanly there, so
  Ghidra has not been needed.
- **Not installed:** Cheat Engine, Ghidra and x64dbg. Ghidra 12.2 needs Java 25 and this machine has JRE 17, so add a
  portable JDK under `scratch/` if it's needed. Installing Cheat Engine needs the user's admin prompt:
  `choco install cheatengine -y`.
