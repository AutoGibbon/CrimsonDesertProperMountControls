"""Print what a new session needs before it starts work: git state, environment checks and the next tasks.

Usage: python tools/session_start.py [--hook]

Exits 1 if a check fails, or always 0 with --hook, because Claude Code adds a SessionStart hook's output to the
session's context only when the hook exits 0. Reads only.
"""

import datetime
import hashlib
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ENV = ROOT / "docs" / "state" / "environment.json"
TEMPLATES = Path.home() / ".claude" / "skills" / "project-scaffold" / "templates"
# Scripts each project copies unchanged from TEMPLATES.
COPIED_SCRIPTS = ("check_state.py", "tasks.py")


def template_notes():
    """Return a note for each of COPIED_SCRIPTS that is missing from tools/ or differs from its template."""

    def norm(path):
        return path.read_bytes().replace(b"\r\n", b"\n")

    notes = []
    for name in COPIED_SCRIPTS:
        ours, template = ROOT / "tools" / name, TEMPLATES / name
        if not template.is_file():
            continue
        if not ours.is_file():
            notes.append(f"tools/{name} is missing; copy it from the project-scaffold template")
        elif norm(ours) != norm(template):
            notes.append(
                f"tools/{name} differs from the project-scaffold template (see the project-state-migrate audit)"
            )
    return notes


def run(*args):
    return subprocess.run(args, check=False, cwd=ROOT, capture_output=True, text=True).stdout.rstrip()


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def tree_hashes(path):
    """Return {relative path: sha256} for every file under path, or for path itself if it is a file."""
    if path.is_file():
        return {"": sha256(path)}
    return {str(p.relative_to(path)).replace("\\", "/"): sha256(p) for p in path.rglob("*") if p.is_file()}


def artefact_checks(env):
    """Compare each artefact's built and deployed file or folder by hash; return the problems found."""
    problems = []
    for a in env.get("artefacts", []):
        built, deployed = a.get("built"), a.get("deployed")
        if not (built and deployed):
            continue
        b = ROOT / built
        d = Path(deployed)
        if not b.exists():
            problems.append(f"{a['name']}: build output {built} missing")
        elif not d.exists():
            problems.append(f"{a['name']}: not deployed at {deployed}")
        elif tree_hashes(b) != tree_hashes(d):
            problems.append(f"{a['name']}: deployed copy differs from {built}")
    return problems


SHARED = ROOT.parent / "_shared"
# Extracts under scratch/out that tools/ regenerates; older than the current exe build, they may not match the game.
EXTRACTS = ["scratch/out/index.tsv", "scratch/out/charts", "scratch/out/charts-6AF5D198", "scratch/out/loc_eng.json"]


def pe_timestamp(path):
    """Return the PE header's TimeDateStamp of the exe at path."""
    with open(path, "rb") as f:
        head = f.read(0x400)
    pe = int.from_bytes(head[0x3C:0x40], "little")
    return int.from_bytes(head[pe + 8 : pe + 12], "little")


def project_checks(env):
    """Check the game exe build, the shared tooling and the age of extracts against env['facts']; return problems."""
    problems = []
    facts = env.get("facts", {})

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

    if not (SHARED / "venv" / "Scripts" / "python.exe").is_file():
        problems.append(f"venv missing at {SHARED / 'venv'}: see {SHARED / 'README.md'}")
    if not (SHARED / "cdmw-src").is_dir():
        problems.append(f"CDMW checkout missing at {SHARED / 'cdmw-src'}: see {SHARED / 'README.md'}")

    # Stale extracts are printed as notes, not problems, because they matter only to work that reads them.
    seen = datetime.date.fromisoformat(facts["exe_seen"]) if facts.get("exe_seen") else None
    stale = [
        p
        for p in EXTRACTS
        if seen and (ROOT / p).exists() and datetime.date.fromtimestamp((ROOT / p).stat().st_mtime) < seen
    ]
    if stale:
        print(
            f"  note: extracts older than the exe seen {seen}; regenerate with tools/ before relying on them: "
            + ", ".join(stale)
        )
    return problems


def main():
    print("== git")
    print(run("git", "status", "--short") or "(clean)")
    print(run("git", "log", "--oneline", "-5"))

    problems = []
    print("\n== environment")
    if ENV.is_file():
        env = json.loads(ENV.read_text(encoding="utf-8"))
        problems += project_checks(env)
        problems += artefact_checks(env)
    for p in problems:
        print(f"  ! {p}")
    notes = template_notes()
    for note in notes:
        print(f"  note: {note}")
    if not problems and not notes:
        print("  ok")

    print("\n== state", flush=True)
    state = subprocess.run([sys.executable, str(ROOT / "tools" / "check_state.py"), str(ROOT)], check=False, cwd=ROOT)

    failed = bool(problems or state.returncode)
    if "--hook" in sys.argv:
        if failed:
            print("\nSession start check found problems; tell the user before starting work.")
        return 0
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
