"""Run HorseSteer's address lookups on game exes and compare each result with that build's known address.

Exits with 1 if any lookup fails. An exe whose PE timestamp isn't in KNOWN only prints what the lookups found.

Usage: check_lookups.py EXE...
"""
import collections, re, struct, sys

# Section names change between builds, so sections are told apart by their flags.
EXEC, WRITE = 0x20000000, 0x80000000

# Class name in RTTI for each vtable.
VTABLES = {
    "vtActorManager": ".?AVClientActorManager@pa@@",
    "vtVehicleComponent": ".?AVClientVehicleActorComponent@pa@@",
    "vtRodeoUi": ".?AVUIGamePlayControlRootMiniGameRodeo@uiCommonScript@pa@@",
}

# Each signature's bytes, and the target's distance from the first byte. The bytes before each site include the field
# offsets the mod relies on, so a layout change fails the lookup.
SIGNATURES = {
    "hookSite": (
        "4C 8D B7 60 01 00 00 C5 F0 14 C0 C4 C1 7B 11 06 C4 C1 7A 11 5E 08 C5 FB 11 A7 54 01 00 00 89 8F 5C 01 00 00 "
        "49 8B 40 20 48 8B 88 30 12 00 00 48 8B 99 90 00 00 00",
        36,
    ),
    "stickSite": ("FF 50 48 48 8B 08 48 89 4F 48 48 8D 77 50 48 8B 47 48 48 89 06 C5 FA 10 0E C5 FA 10 46 04", 10),
    # Starts 14 bytes into the function, because another mod hooks its first bytes at runtime.
    "keyCheck": (
        "48 8B 48 68 48 8B 81 B0 01 00 00 80 B8 79 01 00 00 00 75 ?? 48 8B 05 ?? ?? ?? ?? 48 8B 88 D8 00 00 00 "
        "48 8B 41 20 83 78 28 00 77 04 33 C9 EB 08 48 8B 40 20 48 8B 48 10 48 8B 01 FF 50 40",
        -14,
    ),
    # A call to the chart condition handler table's getter.
    "conditionTableCall": (
        "E8 ?? ?? ?? ?? 81 FB ?? ?? 00 00 73 ?? 8D 8B 7D FF FF FF 48 8D 1C CD 00 00 00 00 48 83 3C 03 00",
        0,
    ),
}

# mov rax,[rip+x]; mov rcx,[rax+0x30]
WORLD_REFERENCE = re.compile(rb"\x48\x8B\x05(....)\x48\x8B\x48\x30", re.DOTALL)
# The most-referenced global must have at least this many times the references of the next.
WORLD_MIN_RATIO = 3
# The actor's chain to its physics object: [[[actor+0x68]+0x40]+0x140]+disp32. Only 8-aligned 64-bit loads count,
# because an unaligned pointer 0x11 past the physics object is loaded as often.
PHYSICS_CHAIN = re.compile(rb"\x8B.\x68.\x8B.\x40.\x8B.\x40\x01\x00\x00(.)\x8B.(....)", re.DOTALL)

KNOWN = {
    0x6AED2E85: {  # Steam build of 2026-11-06, passed stage 18
        "vtActorManager": 0x5946C40,
        "vtVehicleComponent": 0x5946C98,
        "vtRodeoUi": 0x5A747E8,
        "rodeoUiSlot0": 0x11F1180,
        "worldSystemSlot": 0x71C0100,
        "hookSite": 0xB6D946,
        "stickSite": 0xB6DA46,
        "keyCheck": 0x3B0A30,
        "conditionTableCall": 0x3AAA7C,
        "physicsSlot": 0x2C0,
    },
    0x6AF5D198: {  # Steam build of 2026-11-13, not yet tested in game
        "vtActorManager": 0x59C2760,
        "vtVehicleComponent": 0x59C2910,
        "vtRodeoUi": 0x5AF2ED8,
        "rodeoUiSlot0": 0x122AB80,
        "worldSystemSlot": 0x72451C0,
        "hookSite": 0xBA4B46,
        "stickSite": 0xBA4C46,
        "keyCheck": 0x3B4090,
        "conditionTableCall": 0x3AE0CC,
        "physicsSlot": 0x2D0,
    },
}


class Image:
    """A PE file with its sections, which are readable by RVA."""

    def __init__(self, path):
        self.data = open(path, "rb").read()
        d = self.data
        pe = struct.unpack_from("<I", d, 0x3C)[0]
        self.timestamp = struct.unpack_from("<I", d, pe + 8)[0]
        self.base = struct.unpack_from("<Q", d, pe + 48)[0]
        count = struct.unpack_from("<H", d, pe + 6)[0]
        optional_size = struct.unpack_from("<H", d, pe + 20)[0]
        self.sections = []
        for i in range(count):
            header = pe + 24 + optional_size + i * 40
            virtual_size, rva, raw_size, raw_offset = struct.unpack_from("<IIII", d, header + 8)
            flags = struct.unpack_from("<I", d, header + 36)[0]
            self.sections.append((rva, min(virtual_size, raw_size), raw_offset, flags))

    def offset(self, rva):
        for start, size, raw, _ in self.sections:
            if start <= rva < start + size:
                return rva - start + raw
        raise ValueError(f"RVA {rva:#x} is not in the file")

    def regions(self, want_flags):
        """Yield the start RVA and file range of each section that want_flags accepts."""
        for start, size, raw, flags in self.sections:
            if want_flags(flags):
                yield start, raw, raw + size

    def find_all(self, pattern, want_flags):
        """Return the RVA of every match of a compiled pattern, including overlapping ones."""
        hits = []
        for start, first, last in self.regions(want_flags):
            match = pattern.search(self.data, first, last)
            while match:
                hits.append(match.start() - first + start)
                match = pattern.search(self.data, match.start() + 1, last)
        return hits


def is_exec(flags):
    return bool(flags & EXEC)


def is_readonly_data(flags):
    return not flags & (EXEC | WRITE)


def find_vtables(image, class_name):
    """Return the vtable RVAs of class_name's primary base, from its RTTI, or an empty list."""
    names = image.find_all(re.compile(re.escape(class_name.encode() + b"\0")), lambda flags: not flags & EXEC)
    if not names:
        return []
    descriptor = names[0] - 0x10
    # CompleteObjectLocator: signature 1, offset 0 for the primary base, descriptor at +12 and its own RVA at +20.
    vtables = []
    pattern = re.compile(re.escape(struct.pack("<I", descriptor)))
    for field in image.find_all(pattern, is_readonly_data):
        locator = field - 12
        signature, offset = struct.unpack_from("<II", image.data, image.offset(locator))
        own_rva = struct.unpack_from("<I", image.data, image.offset(locator + 20))[0]
        if signature != 1 or offset != 0 or own_rva != locator:
            continue
        pointer = re.compile(re.escape(struct.pack("<Q", image.base + locator)))
        vtables += [p + 8 for p in image.find_all(pointer, is_readonly_data)]
    return vtables


def world_references(image):
    """Return (global RVA, reference count) pairs, most-referenced first."""
    counts = collections.Counter()
    for start, first, last in image.regions(is_exec):
        for match in WORLD_REFERENCE.finditer(image.data, first, last):
            next_instruction = match.start() - first + start + 7
            counts[next_instruction + struct.unpack("<i", match.group(1))[0]] += 1
    return counts.most_common(2)


def physics_loads(image):
    """Return (physics object offset, load count) pairs, most-loaded first."""
    counts = collections.Counter()
    for _, first, last in image.regions(is_exec):
        for match in PHYSICS_CHAIN.finditer(image.data, first, last):
            displacement = struct.unpack("<I", match.group(2))[0]
            if match.group(1)[0] & 0xF8 == 0x48 and displacement % 8 == 0 and displacement < 0x10000:
                counts[displacement] += 1
    return counts.most_common(2)


def signature_pattern(text):
    parts = [b"." if t == "??" else re.escape(bytes.fromhex(t)) for t in text.split()]
    return re.compile(b"".join(parts), re.DOTALL)


def check(image, known):
    """Print each lookup's result and return whether all of them found the known address."""
    ok = True

    def report(name, found, detail=""):
        nonlocal ok
        if not known:
            verdict = "found" if found is not None else "FAIL"
        else:
            verdict = "ok" if found == known[name] else "FAIL"
        ok &= verdict != "FAIL"
        address = f"{found:#x}" if found is not None else "none"
        print(f"  {verdict:5} {name:20} {address:12} {detail}")

    for name, class_name in VTABLES.items():
        hits = find_vtables(image, class_name)
        report(name, hits[0] if len(hits) == 1 else None, f"{len(hits)} vtable(s)")
        if name == "vtRodeoUi" and len(hits) == 1:
            slot0 = struct.unpack_from("<Q", image.data, image.offset(hits[0]))[0] - image.base
            report("rodeoUiSlot0", slot0, "first entry of vtRodeoUi")

    top = world_references(image)
    runner_up = top[1][1] if len(top) > 1 else 0
    clear = bool(top) and top[0][1] >= WORLD_MIN_RATIO * runner_up
    detail = f"{top[0][1] if top else 0} references, next {runner_up}"
    report("worldSystemSlot", top[0][0] if clear else None, detail)

    top = physics_loads(image)
    runner_up = top[1][1] if len(top) > 1 else 0
    clear = bool(top) and top[0][1] >= WORLD_MIN_RATIO * runner_up
    report("physicsSlot", top[0][0] if clear else None, f"{top[0][1] if top else 0} loads, next {runner_up}")

    for name, (text, distance) in SIGNATURES.items():
        hits = image.find_all(signature_pattern(text), is_exec)
        listed = " ".join(f"{h + distance:#x}" for h in hits[:8])
        report(name, hits[0] + distance if len(hits) == 1 else None, f"{len(hits)} match(es): {listed}")
    return ok


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    ok = True
    for path in sys.argv[1:]:
        image = Image(path)
        known = KNOWN.get(image.timestamp)
        print(f"{path}: PE timestamp {image.timestamp:#x}, {'known build' if known else 'unknown build'}")
        ok &= check(image, known)
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
