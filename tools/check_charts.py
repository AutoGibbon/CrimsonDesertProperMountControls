"""Check that the game's ride charts still use the chart conditions HorseSteer answers, after a game update.

Usage: check_charts.py GAME_DIR CDMW_SRC OUT_DIR

Extracts the ride charts of every mount the mod supports into OUT_DIR, then checks each one:

- every chart enters a slowing state on T10 (type 0x10), which a slowdown answers as holding;
- the horse's slowing states go back up to a gait on TE (type 0x0E) without key_brake, which a slowdown answers as not
  holding;
- every chart with GroundMount::stepDown set has a gait ladder where each Shift (key_run) step up has a matching step
  down on AND T66 NOT key_run (type 0x66).

Prints PASS or FAIL per chart and per check, and exits with 1 if any check fails. A chart CDMW can't decode is
printed as UNCHECKED and doesn't fail the run; the wagon's has been one since a game update after 2026-11-03.
"""
import re
import struct
import sys
from pathlib import Path

sys.path.insert(0, sys.argv[2])
from cdmw.core.action_chart_binary import decode_action_chart
from cdmw.core.archive_extraction import read_archive_entry_data
from cdmw.core.archive_format import discover_pamt_files, parse_archive_pamt

HORSE = "r0001_horserd2_movetest_03_upper"
WAGON = "r0012_wagon_upper"
# Charts of the mounts with GroundMount::stepDown set in HorseSteer.cpp.
STEP_DOWN = [
    "m0002_ride_bear_upper",
    "m0002_ride_camel_upper",
    "m0002_ride_reindeer_upper",
    "m0002_ride_elephant_upper",
    "m0002_ride_warthog_upper",
    "m0002_ride_wolf_upper",
    "r0034_iguana_upper",
    "r0035_ride_carmabirdsaurus_00001_upper",
    "m0003_ride_cucubird_upper",
]
CHARTS = [HORSE, WAGON] + STEP_DOWN


def extract(game_dir: Path, out: Path) -> dict[str, Path]:
    """Write each chart in CHARTS from the game's archives to out, and return its path by name."""
    wanted = {name + ".paac" for name in CHARTS}
    found = {}
    for pamt in discover_pamt_files(game_dir):
        for entry in parse_archive_pamt(pamt):
            name = entry.path.rsplit("/", 1)[-1].lower()
            if name not in wanted:
                continue
            result = read_archive_entry_data(entry)
            data = result[0] if isinstance(result, tuple) else result
            path = out / pamt.parent.name / entry.path
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
            found[name[:-5]] = path
    return found


def branches(path: Path):
    """Return the chart's logic token base and its branches as (source, target, first program, second program)."""
    d = path.read_bytes()
    c = decode_action_chart(d, virtual_path=path.name)
    names = c.tables[0].values
    span = {s.name: s for s in c.spans}
    cs = span["conditionInstanceArray bytecode"]
    dw = struct.unpack_from(f"<{cs.size // 4}I", d, cs.offset)
    ix = span["conditionInstanceArray indices"]
    indices = struct.unpack_from(f"<{ix.count}H", d, ix.offset)
    starts = sorted(set(indices))
    # Game update 0x6AF5D198 moved AND, OR and NOT from 0x287 up to 0x28B up, where 0x287 became an ordinary type.
    logic = 0x28B if any(dw[s] in (0x28B, 0x28C, 0x28D) for s in starts) else 0x287

    def show(i):
        n = next((s for s in starts if s > i), len(dw))
        t = dw[i:n]
        op = t[0]
        if op == 0xEA:
            return names[t[1]] if t[1] < len(names) else "key?"
        if op in (logic, logic + 1, logic + 2):
            return ("AND", "OR", "NOT")[op - logic]
        return f"T{op:X}"

    def prog(first, count):
        return " ".join(show(indices[j]) for j in range(first, min(first + count, len(indices))))

    br, rec = span["branchInstanceArray"], span["branchInstanceArray records (32 bytes)"]
    by_hash = {struct.unpack_from("<I", d, n.offset + 0x18)[0]: n.index for n in c.nodes}
    edges = []
    for n in c.nodes:
        first, count = struct.unpack_from("<H", d, n.offset + 0xE4)[0], struct.unpack_from("<H", d, n.offset + 0xE8)[0]
        for r in range(first, min(first + count, rec.count)):
            b = br.offset + struct.unpack_from("<H", d, rec.offset + r * 32 + 0x0C)[0] * 56
            target = by_hash.get(struct.unpack_from("<I", d, b + 0x14)[0])
            s1, s2 = struct.unpack_from("<HH", d, b + 0x24)
            edges.append((n.index, target, prog(s1, d[b + 0x2A]), prog(s2, d[b + 0x2B])))
    return logic, edges


def has(program: str, token: str) -> bool:
    return re.search(rf"(^| ){token}( |$)", program) is not None


def check(name: str, edges) -> list[tuple[str, bool, str]]:
    """Return each check's name, whether it passed and a detail line."""
    results = []
    into_slowing = [(s, t) for s, t, a, b in edges if a == "" and has(b, "T10") and t is not None and t != s]
    slowing = {t for _, t in into_slowing}
    results.append(("T10 into slowing", bool(into_slowing),
                    f"{len(into_slowing)} branches from {len({s for s, _ in into_slowing})} nodes"))
    if name == HORSE:
        back_up = [(s, t) for s, t, a, b in edges
                   if s in slowing and t != s and has(b, "TE") and "NOT key_brake" in b]
        results.append(("TE back up from slowing", bool(back_up), f"{len(back_up)} branches, such as {back_up[:3]}"))
    if name in STEP_DOWN:
        gaits = {s for s, _ in into_slowing}
        up = {(s, t) for s, t, a, b in edges
              if s in gaits and t in gaits and t != s and has(b, "key_run") and "NOT key_run" not in b}
        down = {(s, t) for s, t, a, b in edges if t is not None and t != s and has(b, "T66") and "NOT key_run" in b}
        missing = sorted((t, s) for s, t in up if (t, s) not in down)
        ladder_ok = len(up) >= 3 and not missing
        results.append(("T66 step down per Shift step up", ladder_ok,
                        f"{len(up)} steps up {sorted(up)}, missing steps down {missing or 'none'}"))
    return results


def main() -> int:
    game_dir, out = Path(sys.argv[1]), Path(sys.argv[3])
    found = extract(game_dir, out)
    failed = False
    for name in CHARTS:
        path = found.get(name)
        if not path:
            print(f"FAIL {name}: not found in the archives")
            failed = True
            continue
        try:
            logic, edges = branches(path)
        except Exception as e:
            print(f"UNCHECKED {name}: CDMW can't decode it ({type(e).__name__}: {e})")
            continue
        results = check(name, edges)
        ok = all(passed for _, passed, _ in results)
        failed |= not ok
        print(f"{'PASS' if ok else 'FAIL'} {name} (logic tokens from 0x{logic:X})")
        for label, passed, detail in results:
            print(f"  {'pass' if passed else 'FAIL'} {label}: {detail}")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
