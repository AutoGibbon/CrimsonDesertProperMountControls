"""Compare the field offsets and rodeo vtable slot that HorseSteer's lookups don't cover between two game exes.

Usage: check_fields.py OLD_EXE NEW_EXE

For each pointer chain the mod reads, counts the game's code that loads the same chain, as `mov r64,[a+X]` followed by
`mov r64,[that+Y]`, in both exes. A chain whose count falls below half fails, and the same-base offsets whose counts
rose most are printed as candidates for where the field moved. Slot 47 of UIGamePlayControlRootMiniGameRodeo's vtable
fails if its instructions differ, ignoring addresses, and the slots whose instructions match the old slot 47 are printed.

Exits with 1 if any check fails. Float fields such as the physics object's +0x80 and +0x90 aren't covered.
"""
import collections, re, struct, sys

from capstone import CS_ARCH_X86, CS_MODE_64, Cs

from check_lookups import VTABLES, Image, find_vtables, is_exec

# Each chain the mod reads with ReadPtr, as (first offset, second offset), and where in HorseSteer.cpp it's used.
CHAINS = {
    (0x30, 0x58): "manager+0x58, ResolveRide",
    (0x30, 0x50): "manager+0x50 (Kliff), ResolveRide",
    (0x58, 0xD8): "rider, ResolveRide",
    (0x30, 0x08): "actor table, FindActor",
    (0x08, 0x98): "actor table buckets, FindActor",
    (0x08, 0xA0): "actor table nodes, FindActor",
    (0x68, 0x40): "control component",
    (0x68, 0x48): "vehicle component",
    (0x48, 0x18): "vehicle link, mount key and seat",
    (0x68, 0x1A0): "sync component, rotation and velocity",
    (0x40, 0x140): "physics owner",
    (0x40, 0x28): "package owner, mount name",
    (0x28, 0x18): "package, mount name",
    (0x40, 0x30): "chart info",
    (0x18, 0x28): "chart state",
    (0x18, 0x38): "chart values",
    (0x40, 0x38): "second name object",
    (0x18, 0x50): "camera block",
    (0xD8, 0x20): "input system list, BrakeKeyHeld",
    (0x20, 0x20): "input list first, BrakeKeyHeld",
    (0x20, 0x10): "input object, BrakeKeyHeld",
}
RODEO_FRAME_SLOT = 47
MIN_RATIO = 0.5
CANDIDATE_RANGE = 0x40

LOAD = re.compile(rb"[\x48\x49\x4C\x4D]\x8B[\x40-\xBF]", re.DOTALL)


def decode_load(data, i):
    """Return (destination, base, displacement, length) of a `mov r64,[base+disp]` at i, or None."""
    rex, modrm = data[i], data[i + 2]
    mod, reg, rm = modrm >> 6, (modrm >> 3) & 7, modrm & 7
    if rm == 4:  # SIB addressing
        return None
    reg |= (rex & 4) << 1
    rm |= (rex & 1) << 3
    if mod == 1:
        return reg, rm, struct.unpack_from("<b", data, i + 3)[0], 4
    return reg, rm, struct.unpack_from("<i", data, i + 3)[0], 7


def chain_counts(image):
    """Count chained loads in executable sections by (first displacement, second displacement)."""
    counts = collections.Counter()
    data = image.data
    for _, first, last in image.regions(is_exec):
        for match in LOAD.finditer(data, first, last):
            a = decode_load(data, match.start())
            if not a:
                continue
            j = match.start() + a[3]
            if data[j] in (0x48, 0x49, 0x4C, 0x4D) and data[j + 1] == 0x8B and 0x40 <= data[j + 2] <= 0xBF:
                b = decode_load(data, j)
                if b and b[1] == a[0]:
                    counts[(a[2], b[2])] += 1
    return counts


def slot_shape(image, vtable, slot, disassembler):
    """Return the mnemonics of the function in vtable's slot, up to its first ret or 32 instructions."""
    rva = struct.unpack_from("<Q", image.data, image.offset(vtable + 8 * slot))[0] - image.base
    start = image.offset(rva)
    shape = []
    for insn in disassembler.disasm(image.data[start:start + 256], rva):
        shape.append(insn.mnemonic)
        if insn.mnemonic == "ret" or len(shape) == 32:
            break
    return tuple(shape)


def check_chains(old, new):
    ok = True
    co, cn = chain_counts(old), chain_counts(new)
    print(f"chained loads in total: old {sum(co.values())}, new {sum(cn.values())}")
    for (a, b), use in CHAINS.items():
        o, n = co[(a, b)], cn[(a, b)]
        passed = n >= MIN_RATIO * o
        ok &= passed
        print(f"  {'pass' if passed else 'FAIL'} +{a:#x} -> +{b:#x}  old {o:5}  new {n:5}  {use}")
        if not passed:
            rises = sorted(
                ((cn[(a, d)] - co[(a, d)], d) for (x, d) in set(cn) if x == a and abs(d - b) <= CANDIDATE_RANGE),
                reverse=True,
            )
            for gain, d in rises[:3]:
                if gain > 0:
                    print(f"         candidate +{a:#x} -> +{d:#x}  old {co[(a, d)]}  new {cn[(a, d)]}")
    return ok


def check_rodeo_slot(old, new):
    disassembler = Cs(CS_ARCH_X86, CS_MODE_64)
    vo = find_vtables(old, VTABLES["vtRodeoUi"])
    vn = find_vtables(new, VTABLES["vtRodeoUi"])
    if len(vo) != 1 or len(vn) != 1:
        print(f"  FAIL rodeo slot {RODEO_FRAME_SLOT}: vtables found, old {len(vo)}, new {len(vn)}")
        return False
    want = slot_shape(old, vo[0], RODEO_FRAME_SLOT, disassembler)
    got = slot_shape(new, vn[0], RODEO_FRAME_SLOT, disassembler)
    passed = want == got
    print(f"  {'pass' if passed else 'FAIL'} rodeo slot {RODEO_FRAME_SLOT}: {' '.join(got)}")
    if not passed:
        print(f"         old: {' '.join(want)}")
        for slot in range(RODEO_FRAME_SLOT - 8, RODEO_FRAME_SLOT + 9):
            try:
                if slot_shape(new, vn[0], slot, disassembler) == want:
                    print(f"         candidate slot {slot}")
            except ValueError:
                break
    return passed


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    old, new = Image(sys.argv[1]), Image(sys.argv[2])
    print(f"old {old.timestamp:#x}, new {new.timestamp:#x}")
    ok = check_rodeo_slot(old, new)
    ok &= check_chains(old, new)
    print("PASS" if ok else "FAIL")
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
