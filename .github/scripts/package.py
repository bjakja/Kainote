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
import shutil
import subprocess
import sys
import tarfile
import urllib.request
from pathlib import Path

HERE = Path(__file__).resolve().parent
MANIFEST = HERE / "runtime-assets.json"

# name in the build output -> name in the package
CSRI_RENDERERS = {"VSFilter.dll": "xy-VSFilter_kainote.dll"}
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


def copy_binaries(platform: str, build_dir: Path, stage: Path) -> tuple[list[str], list[str]]:
    taken: list[str] = []
    missing: list[str] = []
    if platform == "windows":
        wanted = ["KaiNote.exe", "KaiNote.pdb", "Icons.dll", "ffms2.dll", "BadMutex.dll",
                  "PreciseTimer.dll", "DownloadManager.dll", "karahelper.dll",
                  "KaiNote_AVX.exe", "KaiNote_AVX.pdb"]
    else:
        wanted = ["kainote"]
    for name in wanted:
        src = build_dir / name
        if src.exists():
            shutil.copyfile(src, stage / name)
            taken.append(name)
        else:
            missing.append(name)
    # CSRI renderers we build ourselves, under the name the release layout uses
    for src_name, dest_name in CSRI_RENDERERS.items():
        src = build_dir / src_name
        if src.exists():
            (stage / "Csri").mkdir(parents=True, exist_ok=True)
            shutil.copyfile(src, stage / "Csri" / dest_name)
            taken.append(f"Csri/{dest_name}")
        else:
            missing.append(f"Csri/{dest_name}")
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


def copy_runtimes(stage: Path) -> tuple[list[str], list[str]]:
    taken: list[str] = []
    missing: list[str] = []
    roots = [Path(os.environ.get("WINDIR", "C:/Windows")) / d for d in ("System32", "SysWOW64")]
    for name in VC_RUNTIME + [D3DX]:
        if (stage / name).exists():
            taken.append(name)
            continue
        for root in roots:
            cand = root / name
            if cand.exists():
                shutil.copyfile(cand, stage / name)
                taken.append(name)
                break
        else:
            missing.append(name)
    return taken, missing


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
        rt, miss_rt = copy_runtimes(stage)
        log(f"   runtimes: {len(rt)} copied" + (f", missing: {', '.join(miss_rt)}" if miss_rt else ""))
        ext, miss_ext = copy_external_binaries(repo_root, stage)
        log(f"   external: {len(ext)} copied" + (f", missing: {', '.join(miss_ext)}" if miss_ext else ""))

    modules = copy_local_modules(repo_root, stage)
    log(f"   local DependencyControl modules: {modules} files")

    fetched, missed = fetch_runtime_assets(repo_root / "Thirdparty" / ".cache" / "runtime", stage)
    log(f"   automation library: {fetched} files" + (f", {len(missed)} failed" if missed else ""))
    for m in missed[:10]:
        log(f"      ! {m}")

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

    write_manifest_file(stage)
    files = sum(1 for p in stage.rglob("*") if p.is_file())
    log(f"== staged {files} files in {stage}")

    if not args.no_archive:
        out = archive(args.platform, stage, stage.parent)
        log(f"== archive {out} ({out.stat().st_size} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
