"""Tabulate each brake gap in a HorseSteer log: mount, source, start value, the first mount_value above 0 after it ends,
whether its state changes returned to the start state, its end reason and its duration.

Needs a log from stage 24 or later, for the source field.

Usage: brake_gaps.py LOG CSV
"""
import collections, csv, re, sys

log = open(sys.argv[1], encoding="utf8", errors="replace").read().splitlines()
rows = list(csv.DictReader(open(sys.argv[2], encoding="utf8", errors="replace")))
for r in rows:
    r["t"] = int(r["tick_ms"])


def num(r, k):
    try:
        return float(r[k])
    except ValueError:
        return None


mount, gap, gaps = "?", None, []
for line in log:
    m = re.search(r"mount package: (\S+)", line)
    if m:
        mount = m.group(1).lower()
    m = re.search(r"brake gap start tick=(\d+) speed=([\d.]+) value=([\d.-]+) state=(\w+) shift=(\d) source=(.*)$", line)
    if m:
        gap = dict(mount=mount, start=int(m[1]), value=round(float(m[3]), 1), first=m[4], source=m[6], states=[])
        continue
    if gap is None:
        continue
    m = re.search(r"\] state (\w+) tick=", line)
    if m:
        gap["states"].append(m[1])
        continue
    m = re.search(r"brake gap end tick=(\d+) after=(\d+)ms .*reason=(.*)$", line)
    if m:
        gap.update(end=int(m[1]), after=int(m[2]), reason=m[3])
        gaps.append(gap)
        gap = None

starts = [g["start"] for g in gaps]
table = collections.defaultdict(list)
for g in gaps:
    # Stop at the next tap, so that a tap made soon after this one isn't read as where this one landed.
    limit = min([s for s in starts if s > g["start"]] + [g["end"] + 2500])
    values = [num(r, "mount_value") for r in rows if g["end"] <= r["t"] <= limit]
    values = [round(v, 1) for v in values if v and v > 0]
    land = values[0] if values else None
    table[(g["mount"], g["source"], g["value"], land, g["first"] in g["states"], g["reason"])].append(g["after"])

print("mount source from land bounced reason count after_ms")
for key, after in sorted(table.items(), key=lambda kv: (kv[0][0], kv[0][1], -kv[0][2], str(kv[0][3]))):
    print(*key, len(after), sorted(after))
