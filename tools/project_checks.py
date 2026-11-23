"""Check this project's environment for the gibbon plugin's session start check, which calls check() at startup."""

import datetime
from pathlib import Path

# Extracts under scratch/out that tools/ regenerates; older than the current exe build, they may not match the game.
EXTRACTS = ["scratch/out/index.tsv", "scratch/out/charts", "scratch/out/charts-6AF5D198", "scratch/out/loc_eng.json"]


def pe_timestamp(path):
    """Return the PE header's TimeDateStamp of the exe at path."""
    with open(path, "rb") as f:
        head = f.read(0x400)
    pe = int.from_bytes(head[0x3C:0x40], "little")
    return int.from_bytes(head[pe + 8 : pe + 12], "little")


def check(root, env):
    """Check the game exe build, the shared tooling and the age of extracts against env["facts"]; return problems.

    root is the project directory as a Path; env is docs/state/environment.json as a dict.
    """
    problems = []
    facts = env.get("facts", {})
    shared = root.parent / "_shared"

    exe = Path(facts.get("game_dir", "")) / "bin64" / "CrimsonDesert.exe"
    if not exe.is_file():
        problems.append(f"game exe not found at {exe}")
    else:
        live = f"0x{pe_timestamp(exe):08X}"
        if live != facts.get("exe_timestamp"):
            problems.append(
                f"game exe is {live}, environment.json records {facts.get('exe_timestamp')}: "
                "the game has updated, follow the game-update skill"
            )

    if not (shared / "venv" / "Scripts" / "python.exe").is_file():
        problems.append(f"venv missing at {shared / 'venv'}: see {shared / 'README.md'}")
    if not (shared / "cdmw-src").is_dir():
        problems.append(f"CDMW checkout missing at {shared / 'cdmw-src'}: see {shared / 'README.md'}")

    # Stale extracts are printed as notes, not problems, because they matter only to work that reads them.
    seen = datetime.date.fromisoformat(facts["exe_seen"]) if facts.get("exe_seen") else None
    stale = [
        p
        for p in EXTRACTS
        if seen and (root / p).exists() and datetime.date.fromtimestamp((root / p).stat().st_mtime) < seen
    ]
    if stale:
        print(
            f"  note: extracts older than the exe seen {seen}; regenerate with tools/ before relying on them: "
            + ", ".join(stale)
        )
    return problems
