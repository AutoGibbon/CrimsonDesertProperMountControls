"""Query docs/state/tasks.json and tests.json: list, show and trace the dependencies of tasks, and list tests.

Usage:
  python tools/tasks.py list [--open] [--status S] [--workstream W]
  python tools/tasks.py show ID...
  python tools/tasks.py deps ID
  python tools/tasks.py tests [--status S]

--root REPO_ROOT before the command reads another project; by default it's the folder above this script's.
Reads only; tools/check_state.py validates the files.
"""

import argparse
import json
import sys
from pathlib import Path

CLOSED = ("done", "dropped")


def load(root, name, key):
    path = root / "docs" / "state" / name
    try:
        return json.loads(path.read_text(encoding="utf-8"))[key]
    except (OSError, json.JSONDecodeError, KeyError, TypeError) as e:
        sys.exit(f"{path}: can't read '{key}': {e}")


def task_line(t):
    """Return one line for a task: status, id, workstream, next flag, dependencies and title."""
    flag = " *next" if t.get("next") else ""
    deps = f" <- {', '.join(t['depends_on'])}" if t.get("depends_on") else ""
    return f"[{t.get('status')}] {t.get('id')} ({t.get('workstream', '-')}){flag}{deps}: {t.get('title')}"


def cmd_list(tasks, args):
    for t in tasks:
        if args.open and t.get("status") in CLOSED:
            continue
        if args.status and t.get("status") != args.status:
            continue
        if args.workstream and t.get("workstream") != args.workstream:
            continue
        print(task_line(t))


def cmd_show(tasks, args):
    by_id = {t.get("id"): t for t in tasks}
    for i, tid in enumerate(args.ids):
        t = by_id.get(tid)
        if i:
            print()
        if t is None:
            print(f"{tid}: no such task")
            continue
        print(task_line(t))
        for key in ("done_when", "evidence", "blocked_by"):
            value = t.get(key)
            if not value:
                continue
            print(f"  {key}:")
            for item in value if isinstance(value, list) else [value]:
                print(f"    - {item}")
        for key in ("detail", "updated"):
            if t.get(key):
                print(f"  {key}: {t[key]}")


def print_tree(tid, edges, by_id, depth, path):
    """Print tid's chain through edges, indented by depth, marking a task already on the current path as a cycle."""
    for nxt in edges.get(tid, []):
        t = by_id.get(nxt)
        status = t.get("status") if t else "unknown"
        if nxt in path:
            print(f"{'  ' * depth}{nxt} [{status}] (cycle)")
            continue
        print(f"{'  ' * depth}{nxt} [{status}]")
        print_tree(nxt, edges, by_id, depth + 1, path | {nxt})


def cmd_deps(tasks, args):
    by_id = {t.get("id"): t for t in tasks}
    if args.id not in by_id:
        sys.exit(f"{args.id}: no such task")
    waits_on = {t.get("id"): t.get("depends_on", []) for t in tasks}
    waited_on_by = {}
    for t in tasks:
        for dep in t.get("depends_on", []):
            waited_on_by.setdefault(dep, []).append(t.get("id"))
    print(task_line(by_id[args.id]))
    for label, edges in (("waits on", waits_on), ("waited on by", waited_on_by)):
        print(f"{label}:")
        if edges.get(args.id):
            print_tree(args.id, edges, by_id, 1, {args.id})
        else:
            print("  (none)")


def cmd_tests(tests, args):
    for t in tests:
        if args.status and t.get("status") != args.status:
            continue
        when = " ".join(str(t[k]) for k in ("date", "build") if t.get(k))
        text = t.get("result") if t.get("status") not in ("planned", "not_run") else t.get("checks")
        print(f"[{t.get('status')}] {t.get('id')}{' ' + when if when else ''}: {text}")


def main():
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    parser = argparse.ArgumentParser(description="Query docs/state/tasks.json and tests.json.")
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parent.parent)
    sub = parser.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("list", help="one line per task")
    p.add_argument("--open", action="store_true", help="leave out done and dropped tasks")
    p.add_argument("--status")
    p.add_argument("--workstream")
    p = sub.add_parser("show", help="full entries")
    p.add_argument("ids", nargs="+", metavar="ID")
    p = sub.add_parser("deps", help="what a task waits on and what waits on it, recursively")
    p.add_argument("id", metavar="ID")
    p = sub.add_parser("tests", help="one line per test")
    p.add_argument("--status")
    args = parser.parse_args()

    root = args.root.resolve()
    if args.cmd == "tests":
        cmd_tests(load(root, "tests.json", "tests"), args)
    else:
        {"list": cmd_list, "show": cmd_show, "deps": cmd_deps}[args.cmd](load(root, "tasks.json", "items"), args)


if __name__ == "__main__":
    main()
