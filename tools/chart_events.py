"""Count frame event kinds in a chart, and print all known kind names matching a regex.

Usage: chart_events.py CDMW_SRC KIND_REGEX CHART.paac...
"""
import re, sys
from collections import Counter
from pathlib import Path
sys.path.insert(0, sys.argv[1])
from cdmw.core.action_chart_binary import decode_action_chart, frame_event_at, FRAME_EVENT_KINDS
print("kinds:", [k for k in FRAME_EVENT_KINDS if re.search(sys.argv[2], k, re.I)])
for a in sys.argv[3:]:
    p = Path(a); d = p.read_bytes(); c = decode_action_chart(d, virtual_path=p.name); n = Counter()
    for an in c.animations:
        for r in an.events: n[frame_event_at(c, d, r).kind_name] += 1
    print(p.name, n.most_common(40))
