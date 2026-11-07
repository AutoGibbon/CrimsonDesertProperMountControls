"""Build Proper Horse Controls and pack it for release.

Writes build/ProperHorseControls-<version>.zip containing ProperHorseControls/ProperHorseControls.asi and
ProperHorseControls/ProperHorseControls.ini, with Logging=0 in the packed ini whatever src/ has.
The version is kVersion in src/HorseSteer/HorseSteer.cpp.

Usage: python tools/pack.py
"""

import re
import subprocess
import sys
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "src" / "HorseSteer"
BUILD = ROOT / "build" / "HorseSteer"
NAME = "ProperHorseControls"


def run(*args: str) -> None:
    print(">", " ".join(args), flush=True)
    subprocess.run(args, cwd=ROOT, check=True)


def read_version() -> str:
    match = re.search(r'kVersion\s*=\s*"([^"]+)"', (SRC / "HorseSteer.cpp").read_text(encoding="utf-8"))
    if not match:
        sys.exit("kVersion not found in HorseSteer.cpp")
    return match.group(1)


def release_ini() -> str:
    """Return the source ini's text with Logging set to 0."""
    text = (SRC / f"{NAME}.ini").read_text(encoding="utf-8")
    text, count = re.subn(r"(?m)^Logging=\d+", "Logging=0", text)
    if count != 1:
        sys.exit(f"expected one Logging= line in {NAME}.ini, found {count}")
    return text


def main() -> None:
    run("cmake", "-S", str(SRC), "-B", str(BUILD), "-G", "Visual Studio 17 2022", "-A", "x64")
    run("cmake", "--build", str(BUILD), "--config", "Release")

    asi = BUILD / "Release" / f"{NAME}.asi"
    if not asi.is_file():
        sys.exit(f"build output missing: {asi}")

    version = read_version()
    out = ROOT / "build" / f"{NAME}-{version}.zip"
    out.unlink(missing_ok=True)
    with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as zf:
        zf.write(asi, f"{NAME}/{NAME}.asi")
        zf.writestr(f"{NAME}/{NAME}.ini", release_ini())

    print(f"\nCreated {out}")
    with zipfile.ZipFile(out) as zf:
        for info in zf.infolist():
            print(f"  {info.filename}  {info.file_size} bytes")


if __name__ == "__main__":
    main()
