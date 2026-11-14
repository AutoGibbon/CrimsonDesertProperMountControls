"""Print the branches into and out of one node of an action chart, with their conditions decoded.

Usage: chart_node_branches.py CDMW_SRC CHART_DIR CHART_NAME NODE

CHART_NAME is the file name without .paac, such as m0002_ride_bear_upper, and NODE is the node index.
"""
import struct, sys, re
from collections import Counter, defaultdict
from pathlib import Path

sys.path.insert(0, sys.argv[1])
from cdmw.core.action_chart_binary import decode_action_chart

p = next(Path(sys.argv[2]).rglob(sys.argv[3] + ".paac"))
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
node = int(sys.argv[4])
print("==", p.name, "node", node, clip(c.nodes[node]))
for n, t, th, bi, p1, p2 in edges:
    if t is c.nodes[node] and n.index != node:
        print(f"  in  from {n.index} {clip(n)} | {p1} | {p2}")
for n, t, th, bi, p1, p2 in edges:
    if n.index == node:
        print(f"  out to {t.index if t else hex(th)} {clip(t) if t else ''} | {p1} | {p2}")
