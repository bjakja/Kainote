#!/usr/bin/env python3
"""Assemble a runnable Kainote package from a build tree.

Used by .github/workflows/build.yml for both platforms.  The runtime data that
is not built (the Automation 4 library, dictionaries, the CSRI renderer) is
fetched from pinned public repositories listed in runtime-assets.json; the
binaries, the CSRI renderer we do build, and the translations come from the
build tree and the source tree.

Fetched files are cached under Thirdparty/.cache/runtime so repeat runs are
cheap; the cache key is the pinned commit, so a bump refetches.

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
from pathlib import Path

HERE = Path(__file__).resolve().parent
MANIFEST = HERE / "runtime-assets.json"

# name in the build output -> name in the package
CSRI_RENDERERS = {"xy-Vsfilter.dll": "xy-VSFilter_kainote.dll"}
# Kainote's own DependencyControl modules: source dir -> package dir
LOCAL_MODULES = {
    "Thirdparty/DependencyControl/bad-mutex": "Automation/automation/Include/BM",
    "Thirdparty/DependencyControl/precise-timer": "Automation/automation/Include/PT",
    "Thirdparty/DependencyControl/threaded-libcurl": "Automation/automation/Include/DM",
}
VC_RUNTIME = ["msvcp140.dll", "vcruntime140.dll", "vcruntime140_1.dll"]
D3DX = "D3DX9_43.dll"


def log(msg: str) -> None:
    print(msg, flush=True)


def fetch(url: str, dest: Path) -> bool:
    """Download url to dest unless it is already there. False on failure."""
    if dest.exists() and dest.stat().st_size:
        return True
    dest.parent.mkdir(parents=True, exist_ok=True)
    tmp = dest.with_suffix(dest.suffix + ".part")
    try:
        with urllib.request.urlopen(url, timeout=60) as r, open(tmp, "wb") as f:
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


def fetch_runtime_assets(cache: Path, stage: Path) -> tuple[int, list[str]]:
    data = json.loads(MANIFEST.read_text(encoding="utf-8"))
    done = 0
    missed: list[str] = []
    for entry in data["automation"]:
        repo, ref, src, dest = entry["repo"], entry["ref"], entry["repo_path"], entry["dest"]
        url = f"https://raw.githubusercontent.com/{repo}/{ref}/{src}"
        cached = cache / ref[:12] / src
        target = stage / dest
        if target.exists():
            done += 1
            continue
        if not fetch(url, cached):
            missed.append(dest)
            continue
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(cached, target)
        done += 1
    return done, missed


def apply_kainote_changes(repo_root: Path, stage: Path, prefer_upstream: bool = False) -> tuple[list[str], list[str]]:
    """Install the files Kainote's package carries over the upstream library.

    `.github/patches/kainote-runtime-files.tar.gz` holds the automation library
    and theme files exactly as the released package ships them, with a
    `FILES.sha256` manifest.  The upstream repositories do not publish several of
    them (the karaoke template helpers, the json library, effector, karahelper),
    and the rest carry fixes upstream does not have -- LuaJIT compatibility in
    aegisub.lfs, an FFI leak fix in aegisub.re, DependencyControl's version
    substitution.  `.github/patches/*.patch` is the readable record of how each
    file differs from its upstream revision.

    Extraction is verified against the manifest, so a corrupt or partial bundle
    fails loudly instead of producing a package with half a library in it.
    """
    bundle = repo_root / ".github" / "patches" / "kainote-runtime-files.tar.gz"
    applied: list[str] = []
    failed: list[str] = []
    if prefer_upstream or not bundle.is_file():
        return applied, failed
    import tarfile

    with tarfile.open(bundle, "r:gz") as tar:
        members = {m.name: m for m in tar.getmembers() if m.isfile()}
        manifest_raw = tar.extractfile(members["FILES.sha256"]).read().decode()
        expected = dict(reversed(line.split("  ", 1)) for line in manifest_raw.strip().splitlines())
        for name, want in expected.items():
            if name not in members:
                failed.append(f"{name} (missing from bundle)")
                continue
            data = tar.extractfile(members[name]).read()
            got = hashlib.sha256(data).hexdigest()
            if got != want:
                failed.append(f"{name} (checksum)")
                continue
            path = stage / name
            if path.exists() and hashlib.sha256(path.read_bytes()).hexdigest() == got:
                continue
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
            applied.append(name)
    return applied, failed


def copy_local_modules(repo_root: Path, stage: Path) -> int:
    """BadMutex / PreciseTimer / DownloadManager scripts live in this tree."""
    copied = 0
    for src_rel, dest_rel in LOCAL_MODULES.items():
        src = repo_root / src_rel
        if not src.is_dir():
            continue
        for path in src.rglob("*"):
            if path.is_file() and path.suffix in (".lua", ".moon"):
                rel = path.relative_to(src)
                out = stage / dest_rel / rel
                out.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(path, out)
                copied += 1
    return copied


def copy_dictionaries(repo_root: Path, stage: Path) -> int:
    """en_US from Aegisub's dictionary repo, pl/th_TH from LibreOffice's."""
    plan = json.loads(MANIFEST.read_text(encoding="utf-8"))["dictionaries"]
    cache = repo_root / "Thirdparty" / ".cache" / "runtime"
    out_dir = stage / "Dictionary"
    out_dir.mkdir(parents=True, exist_ok=True)
    copied = 0
    for name, src in plan.items():
        cached = cache / "dict" / src["file"]
        if not fetch(src["url"], cached):
            continue
        shutil.copyfile(cached, out_dir / name)
        copied += 1
    return copied


def compile_locales(repo_root: Path, stage: Path) -> int:
    """Locale/*.po -> Locale/*.mo, the layout wxLocale loads."""
    po_dir = repo_root / "Locale"
    out_dir = stage / "Locale"
    msgfmt = shutil.which("msgfmt")
    if not po_dir.is_dir():
        return 0
    out_dir.mkdir(parents=True, exist_ok=True)
    count = 0
    for po in sorted(po_dir.glob("*.po")):
        mo = out_dir / (po.stem + ".mo")
        if msgfmt:
            r = subprocess.run([msgfmt, "-o", str(mo), str(po)], capture_output=True, text=True)
            if r.returncode == 0:
                count += 1
            else:
                log(f"    ! msgfmt failed for {po.name}: {r.stderr.strip()[:160]}")
        elif po.with_suffix(".mo").exists():
            shutil.copyfile(po.with_suffix(".mo"), mo)
            count += 1
    return count


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
    roots = [build_dir, build_dir.parent, build_dir.parent.parent]
    for root in roots:
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
        modules = {"BadMutex.dll": "BM", "PreciseTimer.dll": "PT", "DownloadManager.dll": "DM"}
        renderer = "xy-Vsfilter.dll"
    else:
        wanted = ["kainote"]
        modules = {}
        renderer = None
    for name in wanted:
        src = find_output(build_dir, name)
        if src:
            shutil.copyfile(src, stage / name)
            taken.append(name)
        else:
            missing.append(name)
    for dll, sub in modules.items():
        src = find_output(build_dir, dll)
        dest = f"Automation/automation/Include/{sub}/{dll}"
        if src:
            out = stage / dest
            out.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(src, out)
            taken.append(dest)
        else:
            missing.append(dest)
    if renderer:
        src = find_output(build_dir, renderer)
        dest = f"Csri/{CSRI_RENDERERS[renderer]}"
        if src:
            out = stage / dest
            out.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(src, out)
            taken.append(dest)
        else:
            missing.append(dest)
    return taken, missing


def copy_external_binaries(repo_root: Path, stage: Path) -> tuple[list[str], list[str]]:
    """The VSFiltermod renderer (prebuilt, pinned) and the VC/D3DX runtimes."""
    taken: list[str] = []
    missing: list[str] = []
    plan = json.loads(MANIFEST.read_text(encoding="utf-8"))["external_binaries"]
    cache = repo_root / "Thirdparty" / ".cache" / "runtime"
    for entry in plan:
        cached = cache / "ext" / Path(entry["url"]).name
        if not fetch(entry["url"], cached):
            missing.append(entry["dest"])
            continue
        if cached.suffix == ".zip":
            import zipfile
            with zipfile.ZipFile(cached) as z:
                member = next((n for n in z.namelist() if n.endswith(entry["member"])), None)
                if not member:
                    missing.append(entry["dest"])
                    continue
                (stage / entry["dest"]).parent.mkdir(parents=True, exist_ok=True)
                with z.open(member) as src, open(stage / entry["dest"], "wb") as out:
                    shutil.copyfileobj(src, out)
        else:
            (stage / entry["dest"]).parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(cached, stage / entry["dest"])
        taken.append(entry["dest"])
    return taken, missing


# Microsoft's own DirectX End-User Runtime redistributable; its terms allow
# shipping the runtime as part of an application, which is how D3DX9_43.dll
# reaches users.  Fetched from download.microsoft.com, never copied from a
# machine that happens to have it.
DX_REDIST_URL = ("https://download.microsoft.com/download/8/4/A/"
                 "84A35BF1-DAFE-4AE8-82AF-AD2AE20B6B14/directx_Jun2010_redist.exe")
DX_CAB = "Jun2010_D3DX9_43_x64.cab"
# The Visual Studio redistributable folder is what Microsoft licenses for
# app-local deployment, so it is preferred over the machine's System32 copies.
VS_CRT_DIRS = [
    "Microsoft Visual Studio/2022/*/VC/Redist/MSVC/*/x64/Microsoft.VC143.CRT",
    "Microsoft Visual Studio/2022/*/VC/Redist/MSVC/*/x64/Microsoft.VC142.CRT",
]
VC_RUNTIME_FALLBACK_DIRS = ["System32", "SysWOW64"]
D3DX_FILE = "D3DX9_43.dll"


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
            subprocess.run([str(redist), *args], capture_output=True, text=True, check=False)
            if cab.exists():
                break
    if not cab.exists():
        for found in work.glob("*.cab"):
            if "d3dx9_43" in found.name.lower():
                cab = found
                break
    if not cab.exists():
        log(f"    ! {DX_CAB} not found in the DirectX redistributable; keeping what is there")
        return None
    out = work / "unpacked"
    out.mkdir(parents=True, exist_ok=True)
    target = out / D3DX_FILE
    if not target.exists():
        subprocess.run(["expand", f"-F:{D3DX_FILE}", str(cab), str(out)],
                       capture_output=True, text=True, check=False)
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
    origins: dict[tuple[str, str], int] = {}
    for entry in data["automation"]:
        origin = (entry["repo"], entry["ref"][:12])
        origins[origin] = origins.get(origin, 0) + 1
    lines = ["Kainote runtime data and where it comes from.", ""]
    lines.append("Automation 4 library:")
    for (repo, ref), count in sorted(origins.items()):
        lines.append(f"  {repo} at {ref} ({count} files)")
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
    if (stage / "Automation" / "automation").is_dir():
        lines.append("")
        lines.append("Automation files the released package carries but the upstream repositories")
        lines.append("above do not publish, plus its changes to the ones they do:")
        lines.append("  Automation/** - see .github/patches/ in the source tree")
        lines.append("  Themes/*.txt - from the released package")
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
        if out.exists():
            out.unlink()
        import zipfile
        with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as z:
            for path in sorted(stage.rglob("*")):
                if path.is_file():
                    z.write(path, Path(stage.name) / path.relative_to(stage))
        return out
    out = dist / f"{stage.name}.tar.gz"
    if out.exists():
        out.unlink()
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
    ap.add_argument("--prefer-upstream", action="store_true",
                    help="do not install the files Kainote's package carries over the upstream library")
    args = ap.parse_args()

    repo_root = Path(args.repo_root).resolve()
    build_dir = Path(args.build_dir).resolve()
    stage = Path(args.stage).resolve()
    if stage.exists():
        shutil.rmtree(stage)
    stage.mkdir(parents=True)

    log(f"== packaging {args.platform}: build {build_dir} -> {stage}")
    taken, missing_bins = copy_binaries(args.platform, build_dir, stage)
    log(f"   binaries: {len(taken)} copied" + (f", missing: {', '.join(missing_bins)}" if missing_bins else ""))

    if args.platform == "windows":
        rt, miss_rt = copy_runtimes(repo_root, stage)
        log(f"   runtimes: {len(rt)} copied" + (f", missing: {', '.join(miss_rt)}" if miss_rt else ""))
        ext, miss_ext = copy_external_binaries(repo_root, stage)
        log(f"   external: {len(ext)} copied" + (f", missing: {', '.join(miss_ext)}" if miss_ext else ""))

    modules = copy_local_modules(repo_root, stage)
    log(f"   local DependencyControl modules: {modules} files")

    fetched, missed = fetch_runtime_assets(repo_root / "Thirdparty" / ".cache" / "runtime", stage)
    log(f"   automation library: {fetched} files" + (f", {len(missed)} failed" if missed else ""))
    for m in missed[:10]:
        log(f"      ! {m}")

    restored, unrestored = apply_kainote_changes(repo_root, stage, prefer_upstream=args.prefer_upstream)
    log(f"   Kainote's files restored: {len(restored)}"
        + (f", {len(unrestored)} problems: {', '.join(unrestored[:4])}" if unrestored else ""))

    dicts = copy_dictionaries(repo_root, stage)
    log(f"   dictionaries: {dicts} files")

    locales = compile_locales(repo_root, stage)
    log(f"   locales: {locales} catalogs")

    for placeholder in ("Automation/autosave/txt.txt", "Automation/log/txt.txt"):
        p = stage / placeholder
        p.parent.mkdir(parents=True, exist_ok=True)
        p.touch()
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
