#!/usr/bin/env bash
set -euo pipefail

repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
wx_source="$repo_root/Thirdparty/wxWidgets"
case "${1-}" in
    '') build_type=Release; default_build="$repo_root/build-wx-linux" ;;
    --debug) build_type=Debug; default_build="$repo_root/build-wx-linux-debug" ;;
    *) echo "usage: $0 [--debug]" >&2; exit 2 ;;
esac
wx_build="${KAINOTE_WX_BUILD_DIR:-$default_build}"
wx_prefix="${KAINOTE_WX_PREFIX:-$wx_build/prefix}"

if [[ ! -f "$wx_source/src/zlib/zlib.h" ]]; then
    echo 'wxWidgets submodules are missing; run git submodule update --init --recursive' >&2
    exit 1
fi

cmake -S "$wx_source" -B "$wx_build" -G Ninja \
    -DCMAKE_BUILD_TYPE="$build_type" \
    -DCMAKE_INSTALL_PREFIX="$wx_prefix" \
    -DwxBUILD_TOOLKIT=gtk3 \
    -DwxBUILD_SHARED=ON \
    -DwxBUILD_INSTALL=ON \
    -DwxBUILD_SAMPLES=OFF \
    -DwxBUILD_TESTS=OFF \
    -DwxBUILD_PRECOMP=OFF \
    -DwxUSE_MEDIACTRL=OFF \
    -DwxUSE_WEBVIEW=OFF
cmake --build "$wx_build" --parallel "${KAINOTE_WX_JOBS:-4}"
cmake --install "$wx_build"
"$wx_prefix/bin/wx-config" --version
