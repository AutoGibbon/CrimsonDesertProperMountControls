"""Print every vehicle type and every Riding_ character from the game's static tables.

Character rows are grouped by the name part after Riding_, such as Horse. The grouping follows the naming convention, so
a rideable character without that prefix isn't listed.

Usage: mount_census.py GAME_DIR CDMW_SRC
"""
import struct, sys
from collections import defaultdict
from pathlib import Path
sys.path.insert(0, sys.argv[2])
from cdmw.core.archive_format import discover_pamt_files, parse_archive_pamt
from cdmw.core.archive_extraction import read_archive_entry_data

TABLES = {
    f"gamedata/binarystaticinfo__/bin/{table}.staticinfo{part}"
    for table in ("vehicleinfo", "characterinfo") for part in ("header", "body")
}


def read_tables(root):
    """Return the contents of TABLES from the archives under root, keyed by path."""
    found = {}
    for pamt in discover_pamt_files(root):
        for entry in parse_archive_pamt(pamt):
            if entry.path in TABLES:
                r = read_archive_entry_data(entry)
                found[entry.path] = r[0] if isinstance(r, tuple) else r
        if len(found) == len(TABLES):
            return found
    sys.exit(f"missing tables: {sorted(TABLES - set(found))}")


def read_rows(header, body, key_format):
    """Return (key, name) for each row, with header entries holding a key_format key and a u32 offset, in body order."""
    entry_format = "<" + key_format + "I"
    count = struct.unpack_from("<H", header, 0)[0]
    if len(header) != 2 + count * struct.calcsize(entry_format):
        sys.exit(f"unexpected header size {len(header)} for {count} rows")
    rows = []
    for key, offset in sorted(
        struct.iter_unpack(entry_format, header[2:]), key=lambda entry: entry[1]
    ):
        length = struct.unpack_from("<I", body, offset)[0]
        name_end = offset + 4 + length
        row_key = struct.unpack_from("<" + key_format, body, name_end)[0]
        # Each row starts with its name and then its key, so a mismatch means the layout has changed.
        if row_key != key:
            sys.exit(f"row at {offset}: key {row_key} doesn't match header key {key}")
        rows.append((key, body[offset + 4:name_end].decode("utf-8", "replace")))
    return rows


def main():
    tables = read_tables(Path(sys.argv[1]))
    def table(name, part):
        return tables[f"gamedata/binarystaticinfo__/bin/{name}.staticinfo{part}"]

    vehicles = read_rows(table("vehicleinfo", "header"), table("vehicleinfo", "body"), "H")
    print(f"# Vehicle types: {len(vehicles)}")
    for key, name in sorted(vehicles):
        print(f"{key}\t{name}")

    characters = read_rows(table("characterinfo", "header"), table("characterinfo", "body"), "I")
    groups = defaultdict(list)
    for key, name in characters:
        if name.startswith("Riding_"):
            groups[name.split("_")[1]].append((name, key))
    print(f"\n# Riding_ characters: {sum(map(len, groups.values()))} of {len(characters)} rows")
    for group, members in sorted(groups.items(), key=lambda item: item[0].lower()):
        print(f"\n## {group}: {len(members)}")
        for name, key in sorted(members):
            print(f"0x{key:X}\t{name}")


main()
