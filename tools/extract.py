"""Decode archive entries whose path matches a regex into an output directory, preserving paths.

Usage: extract.py GAME_DIR CDMW_SRC OUT_DIR PATH_REGEX
"""
import re, sys
from pathlib import Path
sys.path.insert(0, sys.argv[2])
from cdmw.core.archive_format import discover_pamt_files, parse_archive_pamt
from cdmw.core.archive_extraction import read_archive_entry_data
root, out, rx = Path(sys.argv[1]), Path(sys.argv[3]), re.compile(sys.argv[4], re.I)
n = fail = 0
for pamt in discover_pamt_files(root):
    for e in parse_archive_pamt(pamt):
        if not rx.search(e.path): continue
        try:
            r = read_archive_entry_data(e); data = r[0] if isinstance(r, tuple) else r
            p = out / pamt.parent.name / e.path; p.parent.mkdir(parents=True, exist_ok=True); p.write_bytes(data); n += 1
        except Exception as ex:
            fail += 1; print("FAIL", e.path, type(ex).__name__, str(ex)[:150])
print("extracted", n, "failed", fail)
