#!/usr/bin/env python3
"""Compile Locale/*.po into the layout wxWidgets loads.

Output is <out>/<lang>/LC_MESSAGES/kainote.mo.  wx searches <prefix>/<lang>/
LC_MESSAGES and <prefix>/<lang> and nothing else, and enumerates the available
translations by walking directories, so a flat <out>/<lang>.mo is invisible to
it -- which is how the Windows packages came to ship catalogues that could not
load.

This is the one implementation; the CMake build, the packaging script and the
Visual Studio pre-build event all call it, because when they each had their own
they disagreed about both the layout and whether a failure was fatal.
"""
import argparse
import pathlib
import shutil
import subprocess
import sys

DOMAIN = "kainote"


def compile_catalogs(po_dir, out_dir, strict=False, msgfmt=None):
    """Returns the number of catalogues written."""
    po_dir, out_dir = pathlib.Path(po_dir), pathlib.Path(out_dir)
    msgfmt = msgfmt or shutil.which("msgfmt")
    if not msgfmt:
        message = "msgfmt (gettext) not found; no translation catalogues were built"
        if strict:
            sys.exit("error: " + message)
        print("  ! " + message)
        return 0
    if not po_dir.is_dir():
        return 0

    count = 0
    for po in sorted(po_dir.glob("*.po")):
        mo = out_dir / po.stem / "LC_MESSAGES" / (DOMAIN + ".mo")
        mo.parent.mkdir(parents=True, exist_ok=True)
        # -c rejects a catalogue whose format directives do not match the msgid,
        # which is how the Korean placeholders got through last time.
        result = subprocess.run([msgfmt, "-c", "-o", str(mo), str(po)],
                                capture_output=True, text=True)
        if result.returncode == 0:
            count += 1
            continue
        detail = result.stderr.strip().splitlines()
        detail = detail[0][:200] if detail else "exit %d" % result.returncode
        if strict:
            sys.exit("error: msgfmt failed for %s: %s" % (po.name, detail))
        # Never leave a truncated catalogue behind for wx to load.
        mo.unlink(missing_ok=True)
        print("  ! msgfmt failed for %s, skipping it: %s" % (po.name, detail))
    return count


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--po-dir", required=True)
    parser.add_argument("--out-dir", required=True)
    parser.add_argument("--msgfmt")
    parser.add_argument("--strict", action="store_true",
                        help="fail the build instead of skipping a bad catalogue")
    args = parser.parse_args()
    count = compile_catalogs(args.po_dir, args.out_dir, args.strict, args.msgfmt)
    print("  %d translation catalogues" % count)


if __name__ == "__main__":
    main()
