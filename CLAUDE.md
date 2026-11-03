# Crimson Desert horse control mod

Investigation into, and potentially a native ASI mod for, replicating RDR2's horse-relative controls in Crimson Desert.
See `docs/discovery.md` for requirements and findings, and `docs/horseprobe-analysis.md` for how to interpret probe test
results and what to do next.

## Paths

- Game: `Z:\SteamLibrary\steamapps\common\Crimson Desert` (executable: `bin64\CrimsonDesert.exe`)
- Existing mod tooling: `E:\Tools\mods\crimson desert\DMM` (`DMM.exe`, with installed mods in `mods\`)
- Installed ASI mods and their `.ini` files: the game's `bin64\`

## Layout

- `docs/`: investigation notes and findings
- `tools/`: scripts and utilities for analysis and builds. Scripts that import CDMW take the path to a
  [CDMW-Full](https://github.com/Ratty123/CDMW-Full) checkout and need Python 3.11+ with `lz4`, `cryptography`, `Pillow`
  and `numpy`.
- `src/`: mod source
- `scratch/`: git-ignored; put game files, extracts and dumps here so they are not committed
