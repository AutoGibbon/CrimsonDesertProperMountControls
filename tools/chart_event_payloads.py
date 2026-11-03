"""Group a chart's frame events of one kind by payload, listing the clips that use each payload.

Usage: chart_event_payloads.py CDMW_SRC WIDTH_BYTES KIND_NAME CHART.paac...
"""
import struct, sys
from collections import defaultdict
from pathlib import Path
sys.path.insert(0, sys.argv[1])
from cdmw.core.action_chart_binary import decode_action_chart, frame_event_at
kind = sys.argv[3]
for a in sys.argv[4:]:
    p = Path(a); d = p.read_bytes(); c = decode_action_chart(d, virtual_path=p.name)
    groups = defaultdict(list); offs = set()
    for an in c.animations:
        for r in an.events:
            ev = frame_event_at(c, d, r)
            if ev.kind_name == kind: offs.add(ev.offset); groups[ev.offset].append((c.clip_name(an).split('/')[-1], round(r.begin,2), round(r.end,2), r.condition))
    print(p.name, "events", len(offs)); ordered = sorted(offs)
    gaps = [y-x for x,y in zip(ordered, ordered[1:])]; print(" min gap", min(gaps) if gaps else None)
    w = int(sys.argv[2]); pay = defaultdict(list)
    for o in ordered: pay[d[o:o+w].hex(" ", 4)].append(o)
    for h, os_ in sorted(pay.items(), key=lambda kv: -len(kv[1]))[:30]:
        uses = [u for o in os_ for u in groups[o]]
        print(f"  {h} x{len(os_)} uses={len(uses)} e.g.", uses[:3])
