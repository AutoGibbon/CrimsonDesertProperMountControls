"""Tally each byte and dword position of Rotate frame events across all charts, showing which header fields vary.

Usage: rotate_census.py CDMW_SRC CHART_DIR
"""
import struct, sys
from collections import Counter, defaultdict
from pathlib import Path
sys.path.insert(0, sys.argv[1])
from cdmw.core.action_chart_binary import decode_action_chart, frame_event_at, FRAME_EVENT_SIZES
print("Rotate size", FRAME_EVENT_SIZES.get(58), "kind ids", 58)
w = FRAME_EVENT_SIZES.get(58) or 64; hdr = Counter(); ex = defaultdict(set); hsh = Counter(); tot = 0
for p in Path(sys.argv[2]).rglob("*.paac"):
    if p.name.startswith("commonactioninfo"): continue
    d = p.read_bytes()
    try: c = decode_action_chart(d, virtual_path=p.name)
    except Exception: continue
    seen = set()
    for a in c.animations:
        for r in a.events:
            ev = frame_event_at(c, d, r)
            if ev.kind_name == "Rotate" and ev.offset not in seen:
                seen.add(ev.offset); b = d[ev.offset:ev.offset + w]; tot += 1
                k = b[2:12].hex(); hdr[k] += 1; ex[k].add(p.stem); hsh[b[12:16].hex()] += 1
print("total", tot)
for k, n in hdr.most_common(25): print(k, n, sorted(ex[k])[:6])
print("hash field", hsh.most_common(12))
