"""Print the RTTI class name behind each vtable RVA in a PE file, to check published addresses against a build.

Usage: check_rtti.py EXE HEX_RVA...
"""
import struct, sys
d = open(sys.argv[1], "rb").read()
pe = struct.unpack_from("<I", d, 60)[0]
print("PE timestamp", hex(struct.unpack_from("<I", d, pe + 8)[0]))
n = struct.unpack_from("<H", d, pe + 6)[0]; opt = struct.unpack_from("<H", d, pe + 20)[0]; base = struct.unpack_from("<Q", d, pe + 48)[0]
secs = [struct.unpack_from("<IIII", d, pe + 24 + opt + 40 * i + 8) for i in range(n)]
def off(rva):
    for vs, va, rs, rp in secs:
        if va <= rva < va + max(vs, rs): return rva - va + rp
    raise ValueError(hex(rva))
for a in sys.argv[2:]:
    rva = int(a, 16)
    try:
        col = struct.unpack_from("<Q", d, off(rva - 8))[0] - base
        td = struct.unpack_from("<i", d, off(col + 12))[0]
        o = off(td + 16); print(a, d[o:d.index(b"\0", o)].decode())
    except Exception as e:
        print(a, "unresolved", e)
