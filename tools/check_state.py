"""Validate docs/state/*.json against the project state layout and print the work to pick up.

Usage: python tools/check_state.py [REPO_ROOT]

Exits 1 on errors: a missing or invalid file, a bad id or status, or a reference to a task, test, commit or path
that doesn't exist. Warnings (gaps in done_when, evidence, results or blocked_by) are printed but don't fail.
Reads only.
"""

import json
import re
import subprocess
import sys
from pathlib import Path

TASK_STATUSES = ["todo", "in_progress", "blocked", "done", "dropped"]
TEST_STATUSES = ["planned", "passed", "failed", "partial", "inconclusive", "not_run", "aborted"]
TASK_FIELDS = {
    "id",
    "workstream",
    "title",
    "status",
    "next",
    "done_when",
    "depends_on",
    "evidence",
    "blocked_by",
    "detail",
    "updated",
}
TEST_FIELDS = {"id", "file", "checks", "status", "date", "build", "tasks", "result"}
ID_RE = re.compile(r"^[a-z0-9]+(-[a-z0-9]+)*$")
COMMIT_RE = re.compile(r"^[0-9a-f]{7,40}$")
# Evidence with a slash and no spaces is a repo path, checked for existence; other text is accepted as written.
PATH_RE = re.compile(r"^[^\s]*/[^\s]*$")


def load(path, errors):
    if not path.is_file():
        errors.append(f"{path}: missing")
        return None
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as e:
        errors.append(f"{path}: invalid JSON: {e}")
        return None


def path_exists(root, ref):
    """Return whether a repo path, with an optional #anchor, exists."""
    return (root / ref.split("#", 1)[0]).exists()


def commit_exists(root, ref):
    result = subprocess.run(
        ["git", "-C", str(root), "cat-file", "-e", f"{ref}^{{commit}}"], check=False, capture_output=True
    )
    return result.returncode == 0


def check_header(name, data, statuses, list_key, errors):
    if data.get("schema") != 1:
        errors.append(f"{name}: schema must be 1")
    if "statuses" in data and data["statuses"] != statuses:
        errors.append(f"{name}: statuses must be {statuses}")
    if not isinstance(data.get(list_key), list):
        errors.append(f"{name}: '{list_key}' must be a list")
        return []
    return data[list_key]


def check_ids(name, entries, errors):
    seen = set()
    for e in entries:
        i = e.get("id")
        if not isinstance(i, str) or not ID_RE.match(i):
            errors.append(f"{name}: bad id {i!r} (kebab-case required)")
        elif i in seen:
            errors.append(f"{name}: duplicate id {i}")
        seen.add(i)
    return seen


def main():
    root = Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
    state = root / "docs" / "state"
    errors = []
    warnings = []

    tasks_data = load(state / "tasks.json", errors)
    tests_data = load(state / "tests.json", errors)
    env_data = load(state / "environment.json", errors)

    tasks = check_header("tasks.json", tasks_data, TASK_STATUSES, "items", errors) if tasks_data else []
    tests = check_header("tests.json", tests_data, TEST_STATUSES, "tests", errors) if tests_data else []
    if env_data and env_data.get("schema") != 1:
        errors.append("environment.json: schema must be 1")

    task_ids = check_ids("tasks.json", tasks, errors)
    test_ids = check_ids("tests.json", tests, errors)

    for t in tasks:
        tid = t.get("id")
        status = t.get("status")
        for key in ("title", "status"):
            if not t.get(key):
                errors.append(f"tasks.json {tid}: '{key}' missing")
        for key in sorted(set(t) - TASK_FIELDS):
            warnings.append(f"tasks.json {tid}: field '{key}' isn't in the layout; move its content to the detail doc")
        if status not in TASK_STATUSES:
            errors.append(f"tasks.json {tid}: status {status!r} not in {TASK_STATUSES}")
        if status == "in_progress" and not t.get("done_when"):
            warnings.append(f"tasks.json {tid}: in_progress with no done_when")
        if status == "done" and not t.get("evidence"):
            warnings.append(f"tasks.json {tid}: done with no evidence")
        if t.get("next") and status in ("done", "dropped"):
            warnings.append(f"tasks.json {tid}: next is set on a {status} task")
        if status == "blocked" and not t.get("blocked_by"):
            warnings.append(f"tasks.json {tid}: blocked with no blocked_by")
        for dep in t.get("depends_on", []):
            if dep not in task_ids:
                errors.append(f"tasks.json {tid}: depends_on unknown task {dep}")
        for ev in t.get("evidence", []):
            if ev in test_ids:
                continue
            if COMMIT_RE.match(ev):
                if not commit_exists(root, ev):
                    errors.append(f"tasks.json {tid}: evidence commit {ev} not found")
            elif PATH_RE.match(ev) and not path_exists(root, ev):
                errors.append(f"tasks.json {tid}: evidence path {ev} not found")
        if t.get("detail") and not path_exists(root, t["detail"]):
            errors.append(f"tasks.json {tid}: detail {t['detail']} not found")

    for t in tests:
        tid = t.get("id")
        for key in ("checks", "status"):
            if not t.get(key):
                errors.append(f"tests.json {tid}: '{key}' missing")
        for key in sorted(set(t) - TEST_FIELDS):
            warnings.append(f"tests.json {tid}: field '{key}' isn't in the layout; move its content to the test file")
        if t.get("status") not in TEST_STATUSES:
            errors.append(f"tests.json {tid}: status {t.get('status')!r} not in {TEST_STATUSES}")
        if t.get("file") and not path_exists(root, t["file"]):
            errors.append(f"tests.json {tid}: file {t['file']} not found")
        if t.get("status") not in ("planned", "not_run") and not t.get("result"):
            warnings.append(f"tests.json {tid}: status {t.get('status')} with no result")
        for task in t.get("tasks", []):
            if task not in task_ids:
                errors.append(f"tests.json {tid}: unknown task {task}")

    for label, pick in (
        ("Next", lambda t: t.get("next")),
        ("In progress", lambda t: t.get("status") == "in_progress" and not t.get("next")),
        ("Blocked", lambda t: t.get("status") == "blocked" and not t.get("next")),
    ):
        chosen = [t for t in tasks if pick(t)]
        if chosen:
            print(f"{label}:")
            for t in chosen:
                extra = f" (blocked by: {t.get('blocked_by', '?')})" if t.get("status") == "blocked" else ""
                print(f"  - [{t.get('status')}] {t.get('id')}: {t.get('title')}{extra}")
    pending = [t for t in tests if t.get("status") == "planned"]
    if pending:
        print("Planned tests:")
        for t in pending:
            print(f"  - {t.get('id')}: {t.get('checks')}")

    if warnings:
        print(f"\nWarnings ({len(warnings)}):")
        for w in warnings:
            print(f"  - {w}")
    if errors:
        print(f"\nState check FAILED ({len(errors)}):")
        for e in errors:
            print(f"  - {e}")
        return 1
    print("\nState check passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
