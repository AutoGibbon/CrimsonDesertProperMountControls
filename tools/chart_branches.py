"""Print the horse chart's key_brake branches, the branches into its slide and drift states, and its skill conditions.

Usage: chart_branches.py CDMW_SRC CHART_DIR
"""
import struct, sys, re
from collections import Counter, defaultdict
from pathlib import Path

sys.path.insert(0, sys.argv[1])
from cdmw.core.action_chart_binary import decode_action_chart

p = next(Path(sys.argv[2]).rglob("r0001_horserd2_movetest_03_upper.paac"))
d = p.read_bytes()
c = decode_action_chart(d, virtual_path=p.name)
names, clips = c.tables[0].values, c.tables[1].values


def span(n):
    return next(x for x in c.spans if x.name == n)


cs = span("conditionInstanceArray bytecode")
dw = struct.unpack_from(f"<{cs.size//4}I", d, cs.offset)
ix = span("conditionInstanceArray indices")
indices = struct.unpack_from(f"<{ix.count}H", d, ix.offset)
starts = sorted(set(indices))
br = span("branchInstanceArray")
rec = span("branchInstanceArray records (32 bytes)")


def tok(i):
    n = next((s for s in starts if s > i), len(dw))
    return dw[i:n]


def fl(u):
    return struct.unpack("<f", struct.pack("<I", u))[0]


def show(t):
    op = t[0]
    if op == 0xEA:
        return names[t[1]] if t[1] < len(names) else "key?"
    if op == 0x287:
        return "AND"
    if op == 0x288:
        return "OR"
    if op == 0x289:
        return "NOT"
    if op == 0xEE:
        return f"SKILL_USABLE({t[1]})"
    if op == 0xED:
        return f"SKILL_LEARNED({t[1]})"
    if len(t) > 1 and 0x30000000 < t[1] < 0xD0000000:
        return f"T{op:X}({fl(t[1]):g},{t[2]})"
    return f"T{op:X}" + (str(tuple(t[1:3])) if len(t) > 1 else "")


def prog(s, n):
    return " ".join(show(tok(indices[j])) for j in range(s, min(s + n, len(indices))))


def clip(n):
    if not n.animation_count:
        return "-"
    a = c.animations[n.first_animation]
    return clips[a.clip_index].rsplit("/", 1)[-1].replace("cd_rd_prh_", "").replace(".paa", "").replace("basic_00_00_", "").replace("00_00_nor_", "") if a.clip_index < len(clips) else "?"


by_hash = {struct.unpack_from("<I", d, n.offset + 0x18)[0]: n for n in c.nodes}
edges = []
for n in c.nodes:
    first, count = struct.unpack_from("<H", d, n.offset + 0xE4)[0], struct.unpack_from("<H", d, n.offset + 0xE8)[0]
    for r in range(first, first + count):
        if r >= rec.count:
            continue
        bi = struct.unpack_from("<H", d, rec.offset + r * 32 + 0x0C)[0]
        b = br.offset + bi * 56
        target = struct.unpack_from("<I", d, b + 0x14)[0]
        s1, s2 = struct.unpack_from("<HH", d, b + 0x24)
        edges.append((n, by_hash.get(target), target, bi, prog(s1, d[b + 0x2A]), prog(s2, d[b + 0x2B])))

print("== token frequency around key_brake: how it is used ==")
use = Counter()
for n, t, th, bi, p1, p2 in edges:
    for pr in (p1, p2):
        if "key_brake" in pr:
            use[pr] += 1
for k, v in use.most_common():
    print(v, k)

print("\n== branches whose brake use is positive (not preceded by NOT) ==")
rows = Counter()
for n, t, th, bi, p1, p2 in edges:
    for pr in (p1, p2):
        if "key_brake" in pr and not re.search(r"NOT key_brake", pr):
            rows[(clip(n), clip(t) if t else hex(th), p1, p2)] += 1
for k, v in sorted(rows.items(), key=lambda kv: kv[0][1]):
    print(v, " | ".join(k))

print("\n== every branch into the slip stop and drift start nodes ==")
tn = [n for n in c.nodes if re.search(r"slip_end|drift_._stt", clip(n))]
for t0 in tn:
    print("--", t0.index, clip(t0))
    for n, t, th, bi, p1, p2 in edges:
        if t is t0:
            print(f"   from {n.index} {clip(n)} | {p1} | {p2}")

print("\n== who enters the gallop nodes that lead straight to slip_end ==")
for src in (131, 233):
    for n, t, th, bi, p1, p2 in edges:
        if t is c.nodes[src]:
            print(f"into {src}: from {n.index} {clip(n)} | {p1} | {p2}")

print("\n== all skill conditions in the horse chart ==")
skills = Counter()
for n, t, th, bi, p1, p2 in edges:
    for m in re.findall(r"SKILL_\w+\(\d+\)", p1 + " " + p2):
        skills[(m, clip(t) if t else hex(th))] += 1
for k, v in sorted(skills.items()):
    print(v, k)
