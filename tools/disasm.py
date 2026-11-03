"""Disassemble a range of a PE file around given RVAs, using capstone.

Usage: disasm.py EXE BYTES_BEFORE BYTES_AFTER HEX_RVA...
"""
import struct, sys
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
d = open(sys.argv[1], "rb").read()
pe = struct.unpack_from("<I", d, 60)[0]
n = struct.unpack_from("<H", d, pe + 6)[0]; opt = struct.unpack_from("<H", d, pe + 20)[0]; base = struct.unpack_from("<Q", d, pe + 48)[0]
secs = [struct.unpack_from("<IIII", d, pe + 24 + opt + 40 * i + 8) + (d[pe + 24 + opt + 40 * i:pe + 24 + opt + 40 * i + 8].rstrip(b"\0").decode(),) for i in range(n)]
before, after = int(sys.argv[2]), int(sys.argv[3])
md = Cs(CS_ARCH_X86, CS_MODE_64)
for a in sys.argv[4:]:
    rva = int(a, 16)
    vs, va, rs, rp, name = next(s for s in secs if s[1] <= rva < s[1] + max(s[0], s[2]))
    start = rva - before
    print(f"== {a} in {name}")
    for i in md.disasm(d[start - va + rp:start - va + rp + before + after], base + start):
        print(f"{'>' if i.address - base == rva else ' '} {i.address - base:08X}  {i.mnemonic} {i.op_str}")
