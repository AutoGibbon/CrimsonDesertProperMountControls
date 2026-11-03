"""Print string tables, node counts and clip names for action charts.

Usage: chart_dump.py CDMW_SRC CHART.paac...
"""
import sys
from pathlib import Path
sys.path.insert(0, sys.argv[1])
from cdmw.core.action_chart_binary import decode_action_chart
for a in sys.argv[2:]:
    p = Path(a); d = p.read_bytes(); c = decode_action_chart(d, virtual_path=p.name)
    print("=====", p.name, d.__len__(), "nodes", len(c.nodes), "anims", len(c.animations), c.counts)
    for t in c.tables:
        vals = [t.value(i) for i in range(len(t.raw))] if hasattr(t, "raw") else []
        print(f"-- table {t.index} {t.role} n={len(vals)}")
        if t.index in (0,) or "name" in str(t.role).lower(): print("   ", vals[:400])
    print(" fields of node:", [f for f in dir(c.nodes[0]) if not f.startswith('_')] if c.nodes else None)
