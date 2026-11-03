"""Write every archive entry path, with its package and size, to a TSV.

Usage: index_archives.py GAME_DIR CDMW_SRC OUT_TSV
"""
import sys, time
from pathlib import Path
sys.path.insert(0, sys.argv[2])
from cdmw.core.archive_format import discover_pamt_files, parse_archive_pamt
root = Path(sys.argv[1]); t = time.time(); n = 0
with open(sys.argv[3], "w", encoding="utf-8", newline="") as f:
    for pamt in discover_pamt_files(root):
        try:
            entries = parse_archive_pamt(pamt)
        except Exception as e:
            print("FAIL", pamt, type(e).__name__, str(e)[:200]); continue
        print(pamt.parent.name, len(entries), flush=True)
        for e in entries:
            f.write(f"{e.path}\t{pamt.parent.name}\t{e.orig_size}\n"); n += 1
print("total", n, "secs", round(time.time() - t))
