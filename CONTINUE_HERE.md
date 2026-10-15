# CONTINUE HERE — BLOCK MAYHEM development journal

Read this first when resuming. Then `TASKS.md` (status of every requirement) and `PROGRESS_REPORT.md`.

## 1. Current project state (2026-10-15, iteration 2)

A real, bootable GBA ROM (`BlockMayhem.gba`, ~1.1 MB) built with devkitARM + Butano 21.9.0, warning-free.
Every mode in the spec menu is playable: ADVENTURE (6 worlds x 5 stages, 6 bosses), CLASSIC, CHAOS ARCADE
(ability loadout screen), CHAOS RUN (roguelite), DUAL BOARD, BOSS RUSH (unlocks after the World 3 boss),
plus RECORDS, ACHIEVEMENTS, OPTIONS (sound test, credits). First launch plays the intro cutscene; worlds
are introduced on first visit; beating the Core of Mayhem plays the ending, credits and THE END.

Verification: 79/79 host tests (`.\build.ps1 tests`), 8/8 emulator scenarios in the real mGBA core:
smoke, adventure, worlds, chaos_run, boss_rush, menus, save (`--save`), story (`--save`).

## 2. Implemented systems (see TASKS.md for evidence)

- Core (`core/`, portable, deterministic, host-tested): engine, chain resolver + all special cells,
  session (scoring, heat/Overheat, 8 abilities, garbage, hazards, bosses, dual board), bots,
  30 stages, 6 bosses, Chaos Run (`bm_run.*`), achievements (`bm_achievements.*`), save (`bm_save.*`).
- Presentation (`src/`): board renderer with 4 layouts (single, boss, dual, dual boss) and a
  non-overlapping HUD (`hud_pos` table in bm_game_scene.cpp), six animated world backdrops,
  six multi-part boss rigs, effects, achievement banner, all menus and story scenes.
- Assets (`tools/gen_graphics.py`, `tools/gen_audio.py`, deterministic): fonts (small, big, tiny 3x5),
  tiles, sprites (goal icons, upgrade badges), backgrounds, bosses, 14 songs + 8 hot variants, 44 SFX.

## 3. Partially implemented / next priorities

1. MODE-03 Chaos Arcade controlled environmental events (timed hazards that escalate).
2. Balance: SIM-03 runner with a human-paced bot (input delay, imperfect placement) -> par times,
   boss HP; SIM-04 exploit tests; BAL-02..04 documentation (BALANCE.md has the core values + run caps).
3. HW-02 stress scenarios (dense board, chains, boss, particles, dual) + HW-03 sprite budget audit.
4. Visual extras: hard-drop trail (VIS-09), secret events (VIS-12), cosmetics (SECRET-01/PROG-03),
   dual energy bridge (DUAL-07).
5. AUD-05 per-track validation in the emulator (sound test plays every track; RMS + loop check).
6. Docs: INTERACTIONS.md, STAGE_PASSPORTS.md, WORLD/BOSS production matrices, ASSET_MANIFEST.md;
   ENV-05 verify_release pipeline; RELEASE_VERIFICATION.md refresh.

## 4. Build status

Last build: OK, 0 warnings. ROM ~1.1 MB.

## 5. Known issues / notes

- CPU max statistic includes scene-loading frames; reset stats (cmd 6) before measuring gameplay.
- Bots play much faster than humans: stage par times / boss HP still need a human-paced calibration.
- Diagnostic paths (UNLOCK_ALL, SET_BOSS_HP, ADD_HEAT) can unlock records/achievements in test runs only.
- Scenarios without a save file see the first-launch intro: wait 40 frames, tap START, wait for scene 3,
  wait 30 frames (title input lock) before the next tap.

## 6. Important architecture decisions

- Simulation -> rules -> events -> presentation. `core/` never includes Butano; events never change rules.
- One dynamic regular BG holds boards + panels + meters (rebuilt each frame, uploaded once).
- Palette-bank budget: board layer 12 banks; backdrop layers 2 banks each (far + near).
- Backgrounds use `bpp_4_manual` with grit palette reduction (`-mRp`).
- Butano sprite fonts start at '!' (no space glyph in the image).
- Sprites drawn over the board must use bg_priority 0 (default 3 hides them behind the board layer).
- Chaos Run: the run seed fixes bosses, the 9-template line-up, layouts and offers; upgrades only touch
  the rules through `build_modifiers()` with the caps in `bm_config.h` (`run_*`).
- Achievements: evaluated live every 30 frames and once more in `record_session` (merged=true).
- Scene ids are appended at the end of `scene_id` (the test interface uses their numbers).
- Project path has spaces + Hebrew: builds run from a `subst` drive (default `M:`).

## 7. Exact build commands

```powershell
.\build.ps1            # regenerate assets + build ROM
.\build.ps1 rom        # ROM only
.\build.ps1 tests      # host unit tests (79)
.\build.ps1 harness    # build headless emulator harness (first time builds libmgba)
.\build.ps1 emu --rom BlockMayhem.gba --script tests/emu/smoke.txt --out screenshots/tmp
```
From an MSYS2 shell at the mapped drive: `tools/bm.sh <command>`. The Bash tool can call
`/c/msys64/usr/bin/bash.exe -lc "cd /m && ./tools/bm.sh rom"` (MSYSTEM=UCRT64 for tests).
Scene ids for scenarios: 2 intro, 3 title, 4 menu, 5 world map, 6 stage card, 7 gameplay, 8 results,
9 options, 10 records, 11 achievements, 12 upgrades, 13 run summary, 14 credits, 15 ending, 16 sound test,
17 loadout, 19 run map, 20 boss rush, 21 world intro.
