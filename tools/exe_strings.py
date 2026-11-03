"""Print ASCII strings from a PE file that match a regex.

Usage: exe_strings.py EXE REGEX
"""
import re, sys
data = open(sys.argv[1], "rb").read(); rx = re.compile(sys.argv[2], re.I); seen = set()
for m in re.finditer(rb"[\x20-\x7e]{5,200}", data):
    s = m.group().decode()
    if rx.search(s) and s not in seen:
        seen.add(s); print(hex(m.start()), s)
