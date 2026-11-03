"""Print the ASCII strings surrounding given file offsets of a PE file.

Usage: exe_ctx.py EXE RADIUS HEX_OFFSET...
"""
import re, sys
d = open(sys.argv[1], "rb").read(); r = int(sys.argv[2])
for a in sys.argv[3:]:
    o = int(a, 16); print("==", a)
    print(" ; ".join(s.decode() for s in re.findall(rb"[\x20-\x7e]{4,120}", d[o-r:o+r])))
