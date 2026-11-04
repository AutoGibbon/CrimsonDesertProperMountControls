"""Disassemble a PE's .text by .pdata function, annotating references to strings and imports, writing to a file.

Usage: asi_dis.py PE OUT_FILE
"""
import re, struct, sys
from capstone import Cs, CS_ARCH_X86, CS_MODE_64

d = open(sys.argv[1], "rb").read()
pe = struct.unpack_from("<I", d, 0x3C)[0]
nsec, = struct.unpack_from("<H", d, pe + 6)
opt, = struct.unpack_from("<H", d, pe + 20)
base, = struct.unpack_from("<Q", d, pe + 48)
secs = {}
for i in range(nsec):
    o = pe + 24 + opt + i * 40
    n = d[o:o + 8].rstrip(b"\0").decode()
    vs, va, rs, rp = struct.unpack_from("<IIII", d, o + 8)
    secs[n] = (va, vs, rp, rs)
def r2o(r):
    for va, vs, rp, rs in secs.values():
        if va <= r < va + max(vs, rs): return r - va + rp
syms = {}
imp, = struct.unpack_from("<I", d, pe + 24 + 112 + 8)
o = r2o(imp)
while True:
    ilt, _, _, name, iat = struct.unpack_from("<IIIII", d, o); o += 20
    if not name: break
    p, a = r2o(ilt or iat), iat
    while True:
        v, = struct.unpack_from("<Q", d, p); p += 8
        if not v: break
        if not v >> 63:
            q = r2o(v & 0x7fffffff) + 2; syms[a] = "IMP:" + d[q:d.index(b"\0", q)].decode()
        a += 8
def describe(rva):
    if rva in syms: return syms[rva]
    of = r2o(rva)
    if of is None: return None
    m = re.match(rb"[\x20-\x7e\r\n\t]{3,}\0", d[of:of + 300])
    if m: return repr(m.group(0)[:-1].decode())
    m = re.match(rb"(?:[\x20-\x7e]\0){3,}\0\0", d[of:of + 400])
    if m: return "L" + repr(m.group(0)[:-2].decode("utf-16le"))
    return None
md = Cs(CS_ARCH_X86, CS_MODE_64); md.skipdata = True
va, vs, rp, rs = secs[".pdata"]
funcs = []
for i in range(0, vs, 12):
    b, e, u = struct.unpack_from("<III", d, rp + i)
    if b: funcs.append((b, e))
funcs = sorted(set(funcs))
pat = re.compile(r"\[rip ([+-]) (0x[0-9a-f]+)\]")
with open(sys.argv[2], "w", encoding="utf-8") as out:
    for b, e in funcs:
        out.write(f"\n=== func {b:X}-{e:X}\n")
        for i in md.disasm(d[r2o(b):r2o(b) + e - b], base + b):
            note = ""
            m = pat.search(i.op_str)
            if m:
                t = i.address + i.size + int(m.group(2), 16) * (1 if m.group(1) == "+" else -1) - base
                ds = describe(t)
                note = f"  ; {t:X}" + (f" {ds}" if ds else "")
            elif i.mnemonic in ("call", "jmp") or i.mnemonic.startswith("j"):
                pass
            out.write(f"{i.address - base:08X}  {i.mnemonic} {i.op_str}{note}\n")
