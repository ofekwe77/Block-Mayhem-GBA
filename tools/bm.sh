#!/bin/bash
# BLOCK MAYHEM command dispatcher. Runs inside MSYS2 from the project root
# (an ASCII-only path - see build.ps1). Usage: tools/bm.sh <command> [args]
set -e
cd "$(dirname "$0")/.."

if [ -f /etc/profile.d/devkit-env.sh ]; then
    source /etc/profile.d/devkit-env.sh
fi

JOBS=${JOBS:-8}
HOSTCXX=${HOSTCXX:-/ucrt64/bin/g++}
HOSTCC=${HOSTCC:-/ucrt64/bin/gcc}
export PATH="/ucrt64/bin:$PATH"

cmd_assets() {
    python3 tools/gen_graphics.py
    python3 tools/gen_audio.py
}

cmd_rom() {
    make -j"$JOBS"
    if [ ! -f BlockMayhem.gba ]; then
        echo "ERROR: BlockMayhem.gba was not produced" >&2
        exit 1
    fi
    ls -l BlockMayhem.gba
}

cmd_tests() {
    mkdir -p tests/build
    "$HOSTCXX" -std=c++23 -O2 -g -Wall -Wextra -Wshadow -Wpedantic -DBM_HOST=1 -Icore \
        core/*.cpp tests/*.cpp -static -o tests/build/run_tests.exe
    ./tests/build/run_tests.exe "$@"
}

cmd_docs() {
    mkdir -p tests/build
    "$HOSTCXX" -std=c++23 -O2 -Wall -Wextra -DBM_HOST=1 -Icore core/*.cpp tools/gen_docs.cpp -static         -o tests/build/gen_docs.exe
    ./tests/build/gen_docs.exe
}

cmd_harness() {
    bash tools/harness/build_harness.sh
}

cmd_emu() {
    ./tools/harness/build/bm_harness.exe "$@"
}

cmd_verify() {
    bash tools/verify_release.sh "$@"
}

cmd_clean() {
    make clean || true
    rm -rf tests/build
}

case "${1:-all}" in
    all)     cmd_assets; cmd_rom ;;
    assets)  cmd_assets ;;
    rom)     cmd_rom ;;
    tests)   shift; cmd_tests "$@" ;;
    harness) cmd_harness ;;
    docs)    cmd_docs ;;
    emu)     shift; cmd_emu "$@" ;;
    verify)  shift; cmd_verify "$@" ;;
    clean)   cmd_clean ;;
    *) echo "unknown command: $1" >&2; exit 2 ;;
esac
