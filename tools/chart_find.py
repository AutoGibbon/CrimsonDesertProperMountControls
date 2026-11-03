"""List charts whose string tables contain a regex match, with the matching strings.

Usage: chart_find.py CDMW_SRC CHART_DIR REGEX
"""
import re, sys
from pathlib import Path
sys.path.insert(0, sys.argv[1])
from cdmw.core.action_chart_binary import decode_action_chart
rx = re.compile(sys.argv[3])
for p in sorted(Path(sys.argv[2]).rglob("*.paac")):
    try: c = decode_action_chart(p.read_bytes(), virtual_path=p.name)
    except Exception as e: print("ERR", p.name, e); continue
    hits = [v for t in c.tables for v in t.values if rx.search(v)]
    if hits: print(p.as_posix().split("bin__/")[-1], hits)
