#!/usr/bin/env python3
"""Assemble a runnable Kainote package from a build tree.

Used by .github/workflows/build.yml for both platforms.  The Automation 4
library and the themes are tracked in this repository (see Automation/README.md)
and copied from the source tree; the dictionaries and the VSFiltermod renderer
are fetched from pinned upstream locations listed in runtime-assets.json; the
executables, the CSRI renderer and the DependencyControl modules come from the
build tree; the translations are compiled from Locale/*.po.

Usage:
  package.py --platform windows|linux --build-dir <dir> --stage <dir> [--repo-root <dir>]
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tarfile
import urllib.request
import zipfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
MANIFEST = HERE / "runtime-assets.json"

# what the build tree calls the CSRI renderer -> what the package calls it
CSRI_RENDERERS = {"xy-Vsfilter.dll": "xy-VSFilter_kainote.dll"}
# Kainote's DependencyControl modules: DLL name -> package subdirectory.  The
# subdirectory has to match the name requireffi is given ("PT.PreciseTimer.
# PreciseTimer" -> PT/PreciseTimer/PreciseTimer.dll), or the module is not found.
LOCAL_MODULES = {"BadMutex.dll": "BM/BadMutex",
                 "PreciseTimer.dll": "PT/PreciseTimer",
                 "DownloadManager.dll": "DM/DownloadManager"}
# What ffms2.dll imports; avdevice, avfilter and postproc are not used.
FFMPEG_LIBS = ["avcodec", "avformat", "avutil", "swresample", "swscale"]
VC_RUNTIME = ["msvcp140.dll", "vcruntime140.dll", "vcruntime140_1.dll"]

# Microsoft's own DirectX End-User Runtime redistributable; its terms allow
# shipping the runtime as part of an application, which is how D3DX9_43.dll
# reaches users.  Fetched from download.microsoft.com, never copied from a
# machine that happens to have it.
DX_REDIST_URL = ("https://download.microsoft.com/download/8/4/A/"
                 "84A35BF1-DAFE-4AE8-82AF-AD2AE20B6B14/directx_Jun2010_redist.exe")
DX_CAB = "Jun2010_D3DX9_43_x64.cab"
D3DX_FILE = "D3DX9_43.dll"
# The Visual Studio redistributable folder is what Microsoft licenses for
# app-local deployment, so it is preferred over the machine's System32 copies.
VS_CRT_DIRS = [
    "Microsoft Visual Studio/2022/*/VC/Redist/MSVC/*/x64/Microsoft.VC143.CRT",
    "Microsoft Visual Studio/2022/*/VC/Redist/MSVC/*/x64/Microsoft.VC142.CRT",
]
VC_RUNTIME_FALLBACK_DIRS = ["System32", "SysWOW64"]


def log(msg: str) -> None:
    print(msg, flush=True)


def fetch(url: str, dest: Path) -> bool:
    """Download url to dest unless it is already there. False on failure."""
    if dest.exists() and dest.stat().st_size:
        return True
    dest.parent.mkdir(parents=True, exist_ok=True)
    tmp = dest.with_suffix(dest.suffix + ".part")
    try:
        with urllib.request.urlopen(url, timeout=300) as r, open(tmp, "wb") as f:
            shutil.copyfileobj(r, f)
    except Exception as exc:  # network, 404, ...
        log(f"    ! fetch failed: {url} ({exc})")
        tmp.unlink(missing_ok=True)
        return False
    tmp.replace(dest)
    return True


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 16), b""):
            h.update(chunk)
    return h.hexdigest()


def copy_automation(repo_root: Path, stage: Path) -> int:
    """The tracked automation library and themes (see Automation/README.md)."""
    copied = 0
    for folder in ("Automation", "Themes"):
        root = repo_root / folder
        if not root.is_dir():
            continue
        for path in sorted(root.rglob("*")):
            if path.is_file():
                dest = stage / path.relative_to(repo_root)
                dest.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(path, dest)
                copied += 1
    return copied


def find_output(build_dir: Path, name: str) -> Path | None:
    """Locate a build output, wherever its project puts it.

    The projects disagree about OutDir: Kainote writes into the platform folder,
    the CSRI renderer into a csri subfolder, and the DependencyControl modules
    into Automation/automation/Include/<module>/<name>. Searching keeps the
    packaging independent of that.
    """
    direct = build_dir / name
    if direct.exists():
        return direct
    skip = {".git", "dist", "node_modules", ".cache"}
    for root in (build_dir, build_dir.parent, build_dir.parent.parent):
        if not root.is_dir():
            continue
        for path in sorted(root.rglob(name)):
            if path.is_file() and not (skip & set(path.parts)):
                return path
    return None


def copy_binaries(platform: str, build_dir: Path, stage: Path) -> tuple[list[str], list[str]]:
    taken: list[str] = []
    missing: list[str] = []
    if platform == "windows":
        wanted = ["KaiNote.exe", "KaiNote.pdb", "Icons.dll", "ffms2.dll",
                  "KaiNote_AVX.exe", "KaiNote_AVX.pdb"]
    else:
        wanted = ["kainote"]
    for name in wanted:
        src = find_output(build_dir, name)
        if src:
            shutil.copyfile(src, stage / name)
            taken.append(name)
        else:
            missing.append(name)

    for dll, sub in (LOCAL_MODULES.items() if platform == "windows" else ()):
        dest = f"Automation/automation/Include/{sub}/{dll}"
        src = find_output(build_dir, dll)
        if src:
            out = stage / dest
            out.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(src, out)
            taken.append(dest)
        else:
            missing.append(dest)

    if platform == "windows":
        for built_name, packaged_name in CSRI_RENDERERS.items():
            dest = f"Csri/{packaged_name}"
            src = find_output(build_dir, built_name)
            if src:
                out = stage / dest
                out.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(src, out)
                taken.append(dest)
            else:
                missing.append(dest)
    return taken, missing


def copy_ffmpeg_runtime(repo_root: Path, build_dir: Path, stage: Path) -> tuple[list[str], list[str]]:
    """The FFmpeg DLLs ffms2.dll imports, from the hydrated developer package."""
    taken: list[str] = []
    missing: list[str] = []
    binaries = repo_root / "Thirdparty" / "ffmpeg" / "bin"
    for lib in FFMPEG_LIBS:
        # The name carries the ABI version (avcodec-61.dll), so match the prefix.
        found = sorted(binaries.glob(f"{lib}-*.dll")) if binaries.is_dir() else []
        src = found[0] if found else find_output(build_dir, f"{lib}.dll")
        if src is None:
            missing.append(f"{lib}-*.dll")
            continue
        shutil.copyfile(src, stage / src.name)
        taken.append(src.name)
    return taken, missing


def copy_external_binaries(repo_root: Path, stage: Path) -> tuple[list[str], list[str]]:
    """The VSFiltermod renderer (prebuilt, pinned)."""
    taken: list[str] = []
    missing: list[str] = []
    plan = json.loads(MANIFEST.read_text(encoding="utf-8"))["external_binaries"]
    cache = repo_root / "Thirdparty" / ".cache" / "runtime"
    for entry in plan:
        cached = cache / "ext" / Path(entry["url"]).name
        if not fetch(entry["url"], cached):
            missing.append(entry["dest"])
            continue
        out = stage / entry["dest"]
        out.parent.mkdir(parents=True, exist_ok=True)
        if cached.suffix == ".zip":
            with zipfile.ZipFile(cached) as z:
                member = next((n for n in z.namelist() if n.endswith(entry["member"])), None)
                if not member:
                    missing.append(entry["dest"])
                    continue
                with z.open(member) as src, open(out, "wb") as dest:
                    shutil.copyfileobj(src, dest)
        else:
            shutil.copyfile(cached, out)
        taken.append(entry["dest"])
    return taken, missing


def copy_dictionaries(repo_root: Path, stage: Path) -> int:
    """en_US from Aegisub's dictionary repository, pl/th_TH from LibreOffice's."""
    plan = json.loads(MANIFEST.read_text(encoding="utf-8"))["dictionaries"]
    cache = repo_root / "Thirdparty" / ".cache" / "runtime" / "dict"
    out_dir = stage / "Dictionary"
    out_dir.mkdir(parents=True, exist_ok=True)
    copied = 0
    for name, src in plan.items():
        cached = cache / src["file"]
        if not fetch(src["url"], cached):
            continue
        shutil.copyfile(cached, out_dir / name)
        copied += 1
    return copied


def compile_locales(repo_root: Path, stage: Path) -> int:
    """Locale/*.po -> Locale/<lang>/LC_MESSAGES/<lang>.mo.

    wxWidgets only searches <prefix>/<lang>/LC_MESSAGES and <prefix>/<lang>, and
    wxFileTranslationsLoader::GetAvailableTranslations() enumerates with
    wxDIR_DIRS, so a flat Locale/<lang>.mo is never even a candidate.  It used to
    work before wx 3.0 dropped the bare-prefix search; writing it flat here meant
    the package shipped catalogs that could not load.
    """
    po_dir = repo_root / "Locale"
    out_dir = stage / "Locale"
    msgfmt = shutil.which("msgfmt")
    if not po_dir.is_dir():
        return 0
    out_dir.mkdir(parents=True, exist_ok=True)
    count = 0
    for po in sorted(po_dir.glob("*.po")):
        mo = out_dir / po.stem / "LC_MESSAGES" / (po.stem + ".mo")
        mo.parent.mkdir(parents=True, exist_ok=True)
        if msgfmt:
            r = subprocess.run([msgfmt, "-o", str(mo), str(po)], capture_output=True, text=True)
            if r.returncode == 0:
                count += 1
            else:
                log(f"    ! msgfmt failed for {po.name}: {r.stderr.strip()[:160]}")
        elif po.with_suffix(".mo").exists():
            shutil.copyfile(po.with_suffix(".mo"), mo)
            count += 1
    if count == 0 and msgfmt is None:
        log("    ! msgfmt is not on PATH and no compiled catalogs exist: "
            "the package will have no translations")
    return count


def _find_in_dirs(name: str, dirs: list[Path]) -> Path | None:
    for root in dirs:
        for candidate in sorted(root.glob(name)) + sorted(root.rglob(name)):
            if candidate.is_file():
                return candidate
    return None


def _extract_d3dx9(redist: Path) -> Path | None:
    """Unpack D3DX9_43.dll from the official DirectX redistributable."""
    work = redist.parent / "directx"
    cab = work / DX_CAB
    if not cab.exists():
        work.mkdir(parents=True, exist_ok=True)
        for args in (["/Q:A", f"/T:{work}", "/C"], ["/Q", f"/T:{work}", "/C"]):
            try:
                subprocess.run([str(redist), *args], capture_output=True, text=True, check=False)
            except OSError as exc:  # not runnable here (e.g. packaging on Linux)
                log(f"    ! could not run the DirectX redistributable: {exc}")
                break
            if cab.exists():
                break
    if not cab.exists():
        for found in work.glob("*.cab"):
            if "d3dx9_43" in found.name.lower():
                cab = found
                break
    if not cab.exists():
        log(f"    ! {DX_CAB} not found in the DirectX redistributable")
        return None
    out = work / "unpacked"
    out.mkdir(parents=True, exist_ok=True)
    target = out / D3DX_FILE
    if not target.exists():
        try:
            subprocess.run(["expand", f"-F:{D3DX_FILE}", str(cab), str(out)],
                           capture_output=True, text=True, check=False)
        except OSError as exc:
            log(f"    ! could not run expand: {exc}")
    return target if target.exists() else None


def copy_runtimes(repo_root: Path, stage: Path) -> tuple[list[str], list[str]]:
    taken: list[str] = []
    missing: list[str] = []
    program_files = Path(os.environ.get("ProgramFiles", "C:/Program Files"))
    crt_dirs = [Path(p) for pattern in VS_CRT_DIRS for p in program_files.glob(pattern)]
    system_dirs = [Path(os.environ.get("WINDIR", "C:/Windows")) / d for d in VC_RUNTIME_FALLBACK_DIRS]
    for name in VC_RUNTIME:
        if (stage / name).exists():
            taken.append(name)
            continue
        src = _find_in_dirs(name, crt_dirs) or _find_in_dirs(name, system_dirs)
        if src:
            shutil.copyfile(src, stage / name)
            taken.append(name)
        else:
            missing.append(name)

    if (stage / D3DX_FILE).exists():
        taken.append(D3DX_FILE)
        return taken, missing
    cache = repo_root / "Thirdparty" / ".cache" / "runtime" / "directx"
    redist = cache / Path(DX_REDIST_URL).name
    if fetch(DX_REDIST_URL, redist):
        extracted = _extract_d3dx9(redist)
        if extracted:
            shutil.copyfile(extracted, stage / D3DX_FILE)
            taken.append(D3DX_FILE)
        else:
            missing.append(D3DX_FILE)
    else:
        missing.append(D3DX_FILE)
    return taken, missing


def write_notices(repo_root: Path, stage: Path) -> Path:
    """List where every non-built file in the package came from."""
    data = json.loads(MANIFEST.read_text(encoding="utf-8"))
    lines = ["Kainote runtime data and where it comes from.", ""]
    lines.append("Automation 4 library and themes: tracked in this repository,")
    lines.append("  assembled from the upstream revisions named in Automation/README.md.")
    lines.append("")
    lines.append("Dictionaries:")
    for name, src in sorted(data["dictionaries"].items()):
        lines.append(f"  {name} - {src['repo']} at {src['ref'][:12]}")
    if (stage / "Csri").is_dir():
        lines.append("")
        lines.append("CSRI renderers:")
        lines.append("  Csri/xy-VSFilter_kainote.dll - built from this repository's VSFilter")
        for entry in data["external_binaries"]:
            lines.append(f"  {entry['dest']} - {entry['source']}")
    shipped_ffmpeg = sorted(p.name for lib in FFMPEG_LIBS for p in stage.glob(f"{lib}-*.dll"))
    if shipped_ffmpeg:
        lines.append("")
        lines.append("FFmpeg (GPL-3.0-or-later), the prebuilt shared developer package named in")
        lines.append("  Thirdparty/dependencies.json; ffms2.dll links against it:")
        lines.append("  " + ", ".join(shipped_ffmpeg))
    if any((stage / n).exists() for n in VC_RUNTIME + [D3DX_FILE]):
        lines.append("")
        lines.append("Microsoft redistributables (permitted for app-local redistribution with")
        lines.append("an application; taken from Microsoft's own redistributables):")
        lines.append("  msvcp140.dll, vcruntime140.dll, vcruntime140_1.dll - Visual Studio redistributable")
        lines.append("  D3DX9_43.dll - DirectX End-User Runtime (directx_Jun2010_redist.exe)")
    lines.append("")
    path = stage / "third-party-notices.txt"
    path.write_text("\n".join(lines), encoding="utf-8")
    return path


def write_manifest_file(stage: Path) -> None:
    lines = []
    for path in sorted(stage.rglob("*")):
        if path.is_file() and path.name != "artifact-manifest.txt":
            lines.append(f"{path.relative_to(stage).as_posix()}\t{sha256(path)}")
    (stage / "artifact-manifest.txt").write_text("\n".join(lines) + "\n", encoding="utf-8")


def archive(platform: str, stage: Path, dist: Path) -> Path:
    dist.mkdir(parents=True, exist_ok=True)
    if platform == "windows":
        out = dist / f"{stage.name}.zip"
        out.unlink(missing_ok=True)
        with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as z:
            for path in sorted(stage.rglob("*")):
                if path.is_file():
                    z.write(path, Path(stage.name) / path.relative_to(stage))
        return out
    out = dist / f"{stage.name}.tar.gz"
    out.unlink(missing_ok=True)
    with tarfile.open(out, "w:gz", compresslevel=9) as tar:
        for path in sorted(stage.rglob("*")):
            tar.add(path, arcname=Path(stage.name) / path.relative_to(stage))
    return out


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--platform", choices=["windows", "linux"], required=True)
    ap.add_argument("--build-dir", required=True)
    ap.add_argument("--stage", required=True)
    ap.add_argument("--repo-root", default=".")
    ap.add_argument("--no-archive", action="store_true")
    args = ap.parse_args()

    repo_root = Path(args.repo_root).resolve()
    build_dir = Path(args.build_dir).resolve()
    stage = Path(args.stage).resolve()
    if stage.exists():
        shutil.rmtree(stage)
    stage.mkdir(parents=True)

    log(f"== packaging {args.platform}: build {build_dir} -> {stage}")
    library = copy_automation(repo_root, stage)
    log(f"   automation library and themes: {library} files")

    taken, missing = copy_binaries(args.platform, build_dir, stage)
    log(f"   binaries: {len(taken)} copied" + (f", missing: {', '.join(missing)}" if missing else ""))

    if args.platform == "windows":
        ff, miss_ff = copy_ffmpeg_runtime(repo_root, build_dir, stage)
        log(f"   ffmpeg: {len(ff)} copied")
        if miss_ff:
            raise SystemExit(f"ffmpeg runtime missing from the package: {', '.join(miss_ff)}")
        rt, miss_rt = copy_runtimes(repo_root, stage)
        log(f"   runtimes: {len(rt)} copied" + (f", missing: {', '.join(miss_rt)}" if miss_rt else ""))
        ext, miss_ext = copy_external_binaries(repo_root, stage)
        log(f"   external: {len(ext)} copied" + (f", missing: {', '.join(miss_ext)}" if miss_ext else ""))

    dicts = copy_dictionaries(repo_root, stage)
    log(f"   dictionaries: {dicts} files")

    locales = compile_locales(repo_root, stage)
    log(f"   locales: {locales} catalogs")

    for name in ("README.md", "LICENSE"):
        src = repo_root / name
        if src.exists():
            shutil.copyfile(src, stage / ("LICENSE.txt" if name == "LICENSE" else name))

    write_notices(repo_root, stage)
    write_manifest_file(stage)
    files = sum(1 for p in stage.rglob("*") if p.is_file())
    log(f"== staged {files} files in {stage}")

    if not args.no_archive:
        out = archive(args.platform, stage, stage.parent)
        log(f"== archive {out} ({out.stat().st_size} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
