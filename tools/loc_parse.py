"""Decode English paloc tables with CDMW and print entries matching a regex.

Usage: loc_parse.py CDMW_SRC GAME_DIR REGEX OUT_JSON
"""
import re, sys, json
from pathlib import Path
import lz4.block
sys.path.insert(0, sys.argv[1])
from cdmw.core.archive_format import discover_pamt_files, parse_archive_pamt
from cdmw.core.archive_extraction import read_archive_entry_data
from cdmw.core.paloc_format import parse_paloc
rx = re.compile(sys.argv[3], re.I); rows = []
for pamt in discover_pamt_files(Path(sys.argv[2])):
    if pamt.parent.name != "0020": continue
    for e in parse_archive_pamt(pamt):
        if "/eng/" not in e.path or not e.path.endswith(".paloc"): continue
        r = read_archive_entry_data(e); d = r[0] if isinstance(r, tuple) else r
        if len(d) != e.orig_size:
            d = lz4.block.decompress(d, uncompressed_size=e.orig_size)
        try: t = parse_paloc(d, name=e.path)
        except Exception as ex: print("ERR", e.path, len(d), e.orig_size, ex); continue
        for x in t.entries:
            rows.append((Path(e.path).stem, x.key, x.text))
print(len(rows), "entries")
json.dump(rows, open(sys.argv[4], "w", encoding="utf-8"), ensure_ascii=False)
for r in rows:
    if rx.search(str(r[1])) or rx.search(str(r[2])): print(r[0], "|", str(r[1])[:60], "|", str(r[2])[:600])
