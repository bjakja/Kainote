#!/usr/bin/env python3
"""Assert the generated associations agree with the table and with Registry.cpp.

This is the one link the compiler cannot see: the static_asserts in
FileTypes.h tie the table to the icon ids, but nothing ties either to the
string the installer writes into DefaultIcon.
"""

import pathlib
import re
import subprocess
import sys
import tempfile

here = pathlib.Path(__file__).resolve().parent
repo = here.parents[1]

with tempfile.TemporaryDirectory() as tmp:
    out = pathlib.Path(tmp) / "associations.iss"
    subprocess.run([sys.executable, str(here / "gen_associations.py"),
                    "--repo", str(repo), "--out", str(out)], check=True)
    text = out.read_text(encoding="utf-8")

icons = [int(m) for m in re.findall(
    r'DefaultIcon"; ValueType: string; ValueData: """\{app\}\\Kainote\.exe"",-(\d+)"', text)]

expected = list(range(109, 127))
if icons != expected:
    sys.exit(f"DefaultIcon ids {icons} != {expected}")

# Registry.cpp must build the same string: quoted path, comma, negative id.
registry = (repo / "Kainote" / "Registry.cpp").read_text(encoding="utf-8")
if 'L"\\"" + pathfull + L"\\"," + std::to_wstring(-iconResourceId)' not in registry:
    sys.exit("Registry.cpp no longer writes \"<exe>\",-<id>; the installer would disagree")

progids = set(re.findall(r'Software\\Classes\\(Kainote\.[a-z0-9]+)"', text))
if len(progids) != 18:
    sys.exit(f"expected 18 ProgIDs, found {len(progids)}")

print(f"associations.iss: {len(icons)} entries, icon ids {icons[0]}..{icons[-1]}, "
      f"{len(progids)} ProgIDs, DefaultIcon format matches Registry.cpp")
