#!/bin/bash
# BLOCK MAYHEM release verification (PART 56.1, 56.2, 63.1).
#
#   tools/bm.sh verify            (Windows: .\build.ps1 verify)
#
# Checks everything that can be verified mechanically, in order:
#   toolchain -> asset reproducibility -> asset manifest -> ROM build -> ROM header -> host unit tests and
#   simulations -> emulator harness -> generated scenarios -> every emulator scenario on the real ROM ->
#   task inventory.
# Every step reports PASS or FAIL. A missing tool, test file or scenario is a FAIL, never a silent skip.
# Writes docs/verify_report.md and exits 0 only when every step passed.
set -u
cd "$(dirname "$0")/.."
if [ -f /etc/profile.d/devkit-env.sh ]; then
    source /etc/profile.d/devkit-env.sh
fi
export PATH="/ucrt64/bin:$PATH"
JOBS=${JOBS:-4}
OUT=build/verify
mkdir -p "$OUT" docs
REPORT=docs/verify_report.md
START=$(date +%s)

NAMES=()
STATUS=()
DETAIL=()
FAILS=0

record() {
    NAMES+=("$1")
    STATUS+=("$2")
    DETAIL+=("$3")
    if [ "$2" != "PASS" ]; then
        FAILS=$((FAILS + 1))
    fi
    echo "[$2] $1 - $3"
}

# ------------------------------------------------------------------ 1. toolchain
missing=""
[ -n "${DEVKITARM:-}" ] && [ -x "$DEVKITARM/bin/arm-none-eabi-g++" ] || missing="$missing devkitARM"
[ -d /opt/butano/butano ] || missing="$missing Butano(/opt/butano)"
command -v python3 >/dev/null 2>&1 || missing="$missing python3"
[ -x /ucrt64/bin/g++ ] || missing="$missing host-g++(ucrt64)"
[ -d /opt/mgba-dev ] || missing="$missing mGBA-dev(harness)"
if [ -z "$missing" ]; then
    record "Toolchain" PASS "devkitARM gcc $("$DEVKITARM/bin/arm-none-eabi-gcc" -dumpversion), Butano $(grep -o 'BN_VERSION_STRING "[^"]*"' /opt/butano/butano/include/bn_version.h 2>/dev/null | cut -d'"' -f2), Python $(python3 -c 'import sys; print(sys.version.split()[0])'), host g++ $(/ucrt64/bin/g++ -dumpversion)"
else
    record "Toolchain" FAIL "missing:$missing"
fi

# ------------------------------------------------------------------ 2. assets regenerate identically
snapshot() {
    find graphics audio include -type f \( -name '*.bmp' -o -name '*.json' -o -name '*.xm' -o -name '*.wav' -o -name '*.h' \) \
        -print0 | sort -z | xargs -0 md5sum
}
snapshot > "$OUT/assets_before.md5"
if python3 tools/gen_graphics.py > "$OUT/gen_graphics.log" 2>&1 && python3 tools/gen_audio.py > "$OUT/gen_audio.log" 2>&1; then
    snapshot > "$OUT/assets_after.md5"
    changed=$(python3 -c "import sys; a=set(open(sys.argv[1]).read().splitlines()); b=open(sys.argv[2]).read().splitlines(); print(sum(1 for l in b if l not in a))"         "$OUT/assets_before.md5" "$OUT/assets_after.md5")
    if [ "$changed" = "0" ] && [ -s "$OUT/assets_after.md5" ]; then
        record "Assets reproducible" PASS "$(wc -l < "$OUT/assets_after.md5") generated files rebuilt byte-identically from tools/gfx and tools/audio"
    else
        record "Assets reproducible" FAIL "regenerated assets differ from the committed ones: $changed files"
    fi
else
    record "Assets reproducible" FAIL "asset generators failed (see $OUT/gen_*.log)"
fi

# ------------------------------------------------------------------ 3. asset manifest
if python3 tools/check_assets.py --write-manifest ASSET_MANIFEST.md > "$OUT/assets.log" 2>&1; then
    record "Asset manifest" PASS "$(tail -1 "$OUT/assets.log")"
else
    record "Asset manifest" FAIL "$(grep -c FAIL "$OUT/assets.log") failing requirements (see $OUT/assets.log)"
fi

# ------------------------------------------------------------------ 4. ROM build + header
rm -f BlockMayhem.gba
if make -j8 > "$OUT/rom.log" 2>&1 && [ -f BlockMayhem.gba ]; then
    warnings=$(grep -c -i "warning:" "$OUT/rom.log")
    if [ "$warnings" = "0" ]; then
        record "ROM build" PASS "BlockMayhem.gba $(stat -c %s BlockMayhem.gba) bytes, 0 compiler warnings"
    else
        record "ROM build" FAIL "$warnings compiler warnings (see $OUT/rom.log)"
    fi
else
    record "ROM build" FAIL "make failed or produced no ROM (see $OUT/rom.log)"
fi
if [ -f BlockMayhem.gba ] && python3 tools/check_rom.py BlockMayhem.gba > "$OUT/rom_header.log" 2>&1; then
    record "ROM header" PASS "$(grep -E 'title|game_code|save_type' "$OUT/rom_header.log" | tr -s ' ' | tr '\n' ' ')"
else
    record "ROM header" FAIL "$(grep PROBLEM "$OUT/rom_header.log" 2>/dev/null | head -3 | tr '\n' ' ')"
fi

# ------------------------------------------------------------------ 5. host unit tests and simulations
mkdir -p tests/build
if /ucrt64/bin/g++ -std=c++23 -O2 -g -Wall -Wextra -Wshadow -Wpedantic -DBM_HOST=1 -Icore core/*.cpp tests/*.cpp \
        -static -o tests/build/run_tests.exe > "$OUT/tests_build.log" 2>&1; then
    ./tests/build/run_tests.exe > "$OUT/tests.log" 2>&1
    summary=$(grep -E '^[0-9]+ passed, [0-9]+ failed' "$OUT/tests.log" | tail -1)
    if [ -n "$summary" ] && echo "$summary" | grep -q ' 0 failed'; then
        record "Unit tests + simulations" PASS "$summary (core mechanics, specials, abilities, heat, 30 stages, 6 bosses, modes, save, balance, exploits)"
    else
        record "Unit tests + simulations" FAIL "${summary:-no summary} (see $OUT/tests.log)"
    fi
else
    record "Unit tests + simulations" FAIL "host test build failed (see $OUT/tests_build.log)"
fi

# ------------------------------------------------------------------ 6. emulator harness
if bash tools/harness/build_harness.sh > "$OUT/harness.log" 2>&1 && [ -x tools/harness/build/bm_harness.exe ]; then
    record "Emulator harness" PASS "headless mGBA core, scripted input, test-interface checks, screenshots, audio RMS"
else
    record "Emulator harness" FAIL "harness build failed (see $OUT/harness.log)"
fi

# ------------------------------------------------------------------ 7. generated scenarios
if python3 tools/gen_stage_scenarios.py > /dev/null 2>&1 && python3 tools/gen_audio_scenario.py > /dev/null 2>&1; then
    record "Generated scenarios" PASS "stages_w1..6.txt (30 stages) and audio.txt (every track and sound) regenerated from data"
else
    record "Generated scenarios" FAIL "scenario generators failed"
fi

# ------------------------------------------------------------------ 8. emulator scenarios on the real ROM
SCENARIOS=$(ls tests/emu/*.txt 2>/dev/null)
if [ -z "$SCENARIOS" ] || [ ! -x tools/harness/build/bm_harness.exe ] || [ ! -f BlockMayhem.gba ]; then
    record "Emulator scenarios" FAIL "no scenarios, harness or ROM"
else
    mkdir -p "$OUT/emu" "$OUT/shots"
    run_one() {
        local script=$1
        local name
        name=$(basename "$script" .txt)
        local extra=""
        if grep -q '@fresh-save' "$script"; then
            rm -f "$OUT/emu/$name.sav"
            extra="--save $OUT/emu/$name.sav"
        fi
        # shellcheck disable=SC2086
        ./tools/harness/build/bm_harness.exe --rom BlockMayhem.gba --script "$script" --out "$OUT/shots" --quiet $extra \
            > "$OUT/emu/$name.log" 2>&1
        echo $? > "$OUT/emu/$name.exit"
    }
    running=0
    for s in $SCENARIOS; do
        run_one "$s" &
        running=$((running + 1))
        if [ "$running" -ge "$JOBS" ]; then
            wait -n
            running=$((running - 1))
        fi
    done
    wait
    passed=0
    failed=""
    total=0
    for s in $SCENARIOS; do
        name=$(basename "$s" .txt)
        total=$((total + 1))
        if [ "$(cat "$OUT/emu/$name.exit" 2>/dev/null)" = "0" ] && grep -q 'result: PASS' "$OUT/emu/$name.log"; then
            passed=$((passed + 1))
        else
            failed="$failed $name"
        fi
    done
    if [ -z "$failed" ]; then
        record "Emulator scenarios" PASS "$passed/$total scenarios PASS on BlockMayhem.gba"
    else
        record "Emulator scenarios" FAIL "$passed/$total passed; failed:$failed (see $OUT/emu/)"
    fi
fi

# ------------------------------------------------------------------ 9. performance figures (from stress.txt)
if [ -f "$OUT/emu/stress.log" ]; then
    avg=$(grep 'cpu_avg' "$OUT/emu/stress.log" | head -1 | awk '{print $2}')
    worst=$(grep 'cpu_max' "$OUT/emu/stress.log" | awk '{print $2}' | sort -n | tail -1)
    over=$(grep 'cpu_overruns' "$OUT/emu/stress.log" | awk '{s+=$2} END {print s}')
    frames=$(grep 'cpu_frames' "$OUT/emu/stress.log" | awk '{s+=$2} END {print s}')
    sprites=$(grep 'sprites_max' "$OUT/emu/stress.log" | awk '{print $2}' | sort -n | tail -1)
    if [ -n "$avg" ] && [ -n "$frames" ] && [ "$frames" -gt 0 ] && [ $((over * 100)) -le "$frames" ] && [ "$sprites" -le 128 ]; then
        record "Performance" PASS "arcade CPU avg $((avg / 10))%, worst frame $((worst / 10))%, overrun frames $over/$frames, sprite peak $sprites/128"
    else
        record "Performance" FAIL "stress figures out of bounds: avg=$avg worst=$worst overruns=$over/$frames sprites=$sprites"
    fi
else
    record "Performance" FAIL "stress.txt did not run"
fi

# ------------------------------------------------------------------ 10. documentation consistency
docs_detail=""
if /ucrt64/bin/g++ -std=c++23 -O2 -Wall -Wextra -DBM_HOST=1 -Icore core/*.cpp tools/gen_docs.cpp -static \
        -o tests/build/gen_docs.exe > "$OUT/docs_build.log" 2>&1; then
    before=$(md5sum < STAGE_PASSPORTS.md 2>/dev/null)
    ./tests/build/gen_docs.exe > /dev/null 2>&1
    if [ "$(md5sum < STAGE_PASSPORTS.md)" != "$before" ]; then
        docs_detail="STAGE_PASSPORTS.md was out of date with the stage data (regenerated: commit it); "
    fi
else
    docs_detail="passport generator failed to build (see $OUT/docs_build.log); "
fi
if python3 tools/check_docs.py > "$OUT/docs.log" 2>&1 && [ -z "$docs_detail" ]; then
    record "Documentation" PASS "STAGE_PASSPORTS.md matches the stage data; $(tail -1 "$OUT/docs.log")"
else
    record "Documentation" FAIL "${docs_detail}$(tail -1 "$OUT/docs.log")"
fi

# ------------------------------------------------------------------ 11. task inventory
python3 tools/task_report.py > "$OUT/tasks.log" 2>&1
open_tasks=$(grep -E '^\| [A-Z0-9-]+ \| (NOT STARTED|IN PROGRESS|BLOCKED) \|' TASKS.md | grep -v '| HUMAN |' | awk -F'|' '{print $2}' | tr -d ' ' | tr '\n' ' ')
human_tasks=$(grep -E '^\| [A-Z0-9-]+ \| [A-Z ]+ \| HUMAN \|' TASKS.md | awk -F'|' '{print $2}' | tr -d ' ' | tr '\n' ' ')
total_tasks=$(grep -cE '^\| [A-Z0-9-]+ \| (NOT STARTED|IN PROGRESS|IMPLEMENTED|TESTED|POLISHED|BLOCKED) \|' TASKS.md)
tested=$(grep -cE '^\| [A-Z0-9-]+ \| (TESTED|POLISHED) \|' TASKS.md)
if [ -z "$open_tasks" ]; then
    record "Task inventory" PASS "$total_tasks tasks, $tested tested; needs human judgement: ${human_tasks:-none}"
else
    record "Task inventory" FAIL "open tasks: $open_tasks; needs human judgement: ${human_tasks:-none}"
fi

# ------------------------------------------------------------------ report
END=$(date +%s)
{
    echo "# BLOCK MAYHEM — verify_release report"
    echo
    echo "Generated by \`tools/verify_release.sh\` on $(date '+%Y-%m-%d %H:%M') in $((END - START)) s"
    echo "(git $(git rev-parse --short HEAD 2>/dev/null || echo '?')$(git diff --quiet 2>/dev/null || echo ', uncommitted changes'))."
    echo
    echo "| Step | Result | Details |"
    echo "|---|---|---|"
    for i in "${!NAMES[@]}"; do
        echo "| ${NAMES[$i]} | ${STATUS[$i]} | ${DETAIL[$i]} |"
    done
    echo
    echo "Emulator scenarios:"
    echo
    echo "| Scenario | Result | Frames |"
    echo "|---|---|---:|"
    for s in $SCENARIOS; do
        name=$(basename "$s" .txt)
        res=$(grep -o 'result: [A-Z]*' "$OUT/emu/$name.log" 2>/dev/null | tail -1 | cut -d' ' -f2)
        fr=$(grep -o 'frames run: [0-9]*' "$OUT/emu/$name.log" 2>/dev/null | tail -1 | cut -d' ' -f3)
        echo "| $name | ${res:-FAIL} | ${fr:-0} |"
    done
    echo
    if [ "$FAILS" = "0" ]; then
        echo "**Overall: PASS** — every mechanically verifiable gate passed. Human-judgement items are listed in RELEASE_VERIFICATION.md."
    else
        echo "**Overall: FAIL** — $FAILS step(s) failed."
    fi
} > "$REPORT"

echo
if [ "$FAILS" = "0" ]; then
    echo "verify_release: PASS ($((END - START)) s) - report: $REPORT"
    exit 0
fi
echo "verify_release: FAIL ($FAILS step(s)) - report: $REPORT"
exit 1
