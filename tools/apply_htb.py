"""Apply Horse Tap Brake 1.0.2's runtime lookups to a game exe on disk: constants, key hash, RTTI vtable, global,
signature. The ASI addresses are specific to that version.

Usage: apply_htb.py ASI EXE
"""
import collections, struct, sys

asi = open(sys.argv[1], "rb").read()
exe = open(sys.argv[2], "rb").read()

def sections(d):
    pe = struct.unpack_from("<I", d, 0x3C)[0]
    n, = struct.unpack_from("<H", d, pe + 6)
    opt, = struct.unpack_from("<H", d, pe + 20)
    out = []
    for i in range(n):
        o = pe + 24 + opt + i * 40
        name = d[o:o + 8].rstrip(b"\0").decode("latin1")
        vs, va, rs, rp = struct.unpack_from("<IIII", d, o + 8)
        ch, = struct.unpack_from("<I", d, o + 36)
        out.append((name, va, vs, rp, rs, ch))
    return pe, out
_, asec = sections(asi)
def ao(r):
    for s in asec:
        if s[1] <= r < s[1] + max(s[2], s[4]): return r - s[1] + s[3]

print("slot table", struct.unpack_from("<ii", asi, ao(0x1A3A0)))
print("hook table", [hex(x - 0x180000000) for x in struct.unpack_from("<QQ", asi, ao(0x1A4D0))])
for name, r in (("1AA34", 0x1AA34), ("1AA40", 0x1AA40), ("1AA44", 0x1AA44), ("1AA48", 0x1AA48)):
    print(name, struct.unpack_from("<f", asi, ao(r))[0], hex(struct.unpack_from("<I", asi, ao(r))[0]))
print("1AA38 double", struct.unpack_from("<d", asi, ao(0x1AA38))[0])
pat = asi[ao(0x1A3B0):ao(0x1A3B0) + 0x89]
mask = asi[ao(0x1A440):ao(0x1A440) + 0x89]
mask = mask.split(b"\0")[0]
pat = pat[:len(mask)]
print("sig len", len(mask))
print("sig", " ".join(f"{b:02X}" if m == 0x78 else "??" for b, m in zip(pat, mask)))

def rot(x, k): return ((x << k) | (x >> (32 - k))) & 0xFFFFFFFF
def lookup3(data, seed):
    """Bob Jenkins' hashlittle, the routine at ASI 0x19C0: init 0xDEADBEEF + length + seed."""
    M = 0xFFFFFFFF
    a = b = c = (0xDEADBEEF + len(data) + seed) & M
    p, n = 0, len(data)
    while n > 12:
        a = (a + struct.unpack_from("<I", data, p)[0]) & M
        b = (b + struct.unpack_from("<I", data, p + 4)[0]) & M
        c = (c + struct.unpack_from("<I", data, p + 8)[0]) & M
        a = (a - c) & M; a ^= rot(c, 4); c = (c + b) & M
        b = (b - a) & M; b ^= rot(a, 6); a = (a + c) & M
        c = (c - b) & M; c ^= rot(b, 8); b = (b + a) & M
        a = (a - c) & M; a ^= rot(c, 16); c = (c + b) & M
        b = (b - a) & M; b ^= rot(a, 19); a = (a + c) & M
        c = (c - b) & M; c ^= rot(b, 4); b = (b + a) & M
        p += 12; n -= 12
    if n == 0: return c
    t = data[p:] + b"\0" * (12 - n)
    x, y, z = struct.unpack("<III", t)
    a = (a + x) & M; b = (b + y) & M; c = (c + z) & M
    c ^= b; c = (c - rot(b, 14)) & M
    a ^= c; a = (a - rot(c, 11)) & M
    b ^= a; b = (b - rot(a, 25)) & M
    c ^= b; c = (c - rot(b, 16)) & M
    a ^= c; a = (a - rot(c, 4)) & M
    b ^= a; b = (b - rot(a, 14)) & M
    c ^= b; c = (c - rot(b, 24)) & M
    return c
for k in ("Key_Run", "Key_Run_Once_AccByInput", "Key_Brake"):
    print(k, "lower", hex(lookup3(k.lower().encode(), 0xC5EDE)), "raw", hex(lookup3(k.encode(), 0xC5EDE)))

pe, sec = sections(exe)
ts, = struct.unpack_from("<I", exe, pe + 8)
base, = struct.unpack_from("<Q", exe, pe + 48)
print("\nexe timestamp", hex(ts), "base", hex(base), "size", len(exe))
for s in sec: print(" ", s[0], hex(s[1]), hex(s[2]), "exec" if s[5] & 0x20000000 else "", "write" if s[5] & 0x80000000 else "")
def eo(r):
    for s in sec:
        if s[1] <= r < s[1] + min(s[2], s[4]): return r - s[1] + s[3]
def er(of):
    for s in sec:
        if s[3] <= of < s[3] + s[4]: return of - s[3] + s[1]

td = exe.find(b".?AVNormalMovementMethod@pa@@\0")
print("type descriptor string file offset", hex(td))
td_rva = er(td) - 0x10
cols = []
for s in sec:
    if s[5] & 0x80000000 or s[5] & 0x20000000: continue
    data = exe[s[3]:s[3] + s[4]]
    for i in range(0, len(data) - 24, 4):
        if struct.unpack_from("<I", data, i)[0] == 1 and struct.unpack_from("<I", data, i + 12)[0] == td_rva and struct.unpack_from("<I", data, i + 20)[0] == s[1] + i:
            if struct.unpack_from("<I", data, i + 4)[0] == 0:
                cols.append(s[1] + i)
print("primary COLs", [hex(c) for c in cols])
for col in cols:
    tgt = struct.pack("<Q", base + col)
    of = exe.find(tgt)
    while of != -1:
        vt = er(of) + 8
        slots = []
        for j in range(40):
            v, = struct.unpack_from("<Q", exe, of + 8 + j * 8)
            r = v - base
            if not (0x1000 <= r < 0x20000000): break
            ss = next((s for s in sec if s[1] <= r < s[1] + s[2]), None)
            if not ss or not ss[5] & 0x20000000: break
            slots.append((r, ss[0]))
        print("vtable", hex(vt), "slots", len(slots))
        for j, (r, n) in enumerate(slots): print(f"  [{j}] +0x{j * 8:X} exe+0x{r:X} {n}")
        of = exe.find(tgt, of + 8)

# game-state global, votes as in ASI 0x1390: mov rax,[rip+x]; mov rcx,[rax+0x30]
votes = collections.Counter()
for s in sec:
    if not s[5] & 0x20000000: continue
    data = exe[s[3]:s[3] + s[4]]
    i = data.find(b"\x48\x8B\x05")
    while i != -1 and i + 11 <= len(data):
        if data[i + 7:i + 11] == b"\x48\x8B\x48\x30":
            votes[s[1] + i + 7 + struct.unpack_from("<i", data, i + 3)[0]] += 1
        i = data.find(b"\x48\x8B\x05", i + 1)
print("global votes", [(hex(a), n) for a, n in votes.most_common(5)])

# key query signature in executable sections
hits = []
for s in sec:
    if not s[5] & 0x20000000: continue
    data = exe[s[3]:s[3] + s[4]]
    i = data.find(bytes([pat[0]]))
    while i != -1 and i + len(pat) <= len(data):
        if all(m != 0x78 or data[i + j] == pat[j] for j, m in enumerate(mask)):
            hits.append((s[0], hex(s[1] + i)))
        i = data.find(bytes([pat[0]]), i + 1)
print("key query signature hits", hits[:10])
