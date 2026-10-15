# CONTINUE HERE — BLOCK MAYHEM development journal

Read this first when resuming. Then `TASKS.md` (status of every requirement) and `PROGRESS_REPORT.md`.

## 1. Current project state (2026-10-15, iteration 1)

A real, bootable, playable GBA ROM (`BlockMayhem.gba`, ~850 KB) built with devkitARM + Butano 21.9.0.
Title screen → main menu → CLASSIC / CHAOS ARCADE / DUAL BOARD gameplay → results, with original
graphics, original music (14 tracks + 8 Overheat arrangements) and 44 original sound effects.
Verified in the real mGBA core through the headless harness (`tests/emu/smoke.txt` PASS).

## 2. Implemented systems (see TASKS.md for evidence)

- Core engine (`core/`, portable C++, no heap, deterministic): board, SRS rotation + kicks, seven-bag,
  DAS/ARR, lock delay with 12 resets, hold, ghost, T-TWIST detection, top-out.
- Chain-reaction resolver: waves state machine, thaw pulses, bombs/fire/spark/ice/magnet/rainbow/armor,
  charged cells, disabled specials, settle directions, full compaction.
- Session: scoring (lines, drops, combo, chain, B2B, special cells, perfect clear), heat + Overheat,
  8 abilities with targeting, garbage queues, hazards, data-driven boss framework, dual-board rules.
- Bots (5 strategies) used by tests and the ROM's diagnostic autoplay.
- Save format (CRC-32, two slots) + SRAM integration with read-back verification.
- Presentation (`src/`): board renderer (single BG map), animated backdrops (parallax + palette cycling),
  effects (particles, explosions, beams, call-outs, shake, hitstop, flashes), HUD, pause menu,
  title, main menu, results, audio manager with dynamic hot-arrangement switching.
- Asset generators (`tools/gen_graphics.py`, `tools/gen_audio.py`): fonts, tiles, sprites,
  backgrounds (title, menu, World 1), music composer (XM), SFX synthesizer (WAV).

## 3. Partially implemented

- Dual Board: rules done; HUD needs forced-switch countdown + energy bridge.
- Hazards (freeze, lava, gravity, surge, mixed) and boss attacks: logic in `bm_session.cpp`, untested,
  no stage/boss content yet.
- Main menu lists only the modes that work today (no fake entries).

## 4. Remaining work (highest priority first)

1. Stage content: `core/bm_stages.cpp` (30 stages) + `core/bm_bosses.cpp` (6 bosses), Adventure flow
   (world map, stage card, results with ranks, unlocks), stage/boss validation sims + emulator runs.
2. Boss presentation: multi-part animated boss sprites for all six bosses + boss HUD layout.
3. Backgrounds for Worlds 2–6 (equal quality to World 1).
4. Chaos Run (upgrades, rarity, synergies, caps, screens), Boss Rush.
5. Options, Records, Achievements, Credits, Sound test screens; achievements system.
6. verify_release pipeline, balance simulation runner + BALANCE.md, exploit tests, stress tests.
7. Story: intro cutscene, world intros, ending.

## 5. Build status

Last build: OK (warnings only: `-Wuseless-cast` in bm_bot.cpp, bm_rng.h, bm_game_scene.cpp).

## 6. Known bugs / issues

- CPU max statistic includes scene-loading frames; reset stats (cmd 6) before measuring gameplay.
- Classic HUD: score column crowded near the conveyor at the bottom right (cosmetic).

## 7. Important architecture decisions

- Simulation → rules → events → presentation. `core/` never includes Butano; events never change rules.
- One dynamic regular BG holds boards + panels + meter (rebuilt each frame, uploaded once).
- Palette-bank budget: board layer 12 banks; backdrop layers 2 banks each (far + near).
- Backgrounds use `bpp_4_manual` with grit palette reduction (`-mRp`) — do NOT set
  `palette_reduction: false` (it drops per-tile palette banks).
- Butano sprite fonts start at '!' (no space glyph in the image).
- Line clears empty rows in place, then SETTLE removes cleared rows (standard) and closes effect gaps
  by "cells removed beyond" — pre-existing holes are preserved.
- Overheat timer counts active play frames, pauses during resolutions, transitions and pause.
- Boss attacks are scheduled in locked pieces, telegraphed >= 2 locks ahead; Time Freeze locks do not
  advance boss countdowns.
- Project path has spaces + Hebrew: builds run from a `subst` drive (default `M:`).

## 8. Exact build commands

```powershell
.\build.ps1            # regenerate assets + build ROM
.\build.ps1 rom        # ROM only
.\build.ps1 tests      # host unit tests (61)
.\build.ps1 harness    # build headless emulator harness (first time builds libmgba)
.\build.ps1 emu --rom BlockMayhem.gba --script tests/emu/smoke.txt --save tmp.sav --out screenshots/tmp
```
From an MSYS2 shell at the mapped drive: `tools/bm.sh <command>`.
One-time environment setup: see README.md ("Installing dependencies").

## 9. Exact next step

Write `core/bm_stages.cpp` with all 30 stage definitions (objectives / hazards / specials /
layouts per PART 14-19 and 50.4-50.9) and `core/bm_bosses.cpp` with the six boss phase/attack
tables; add `tests/test_stages.cpp` that runs every stage with bots over many seeds and asserts
reachability; then the Adventure flow (world map → stage card → gameplay → results → unlock).

## 10. Most recent test results

- Host unit tests: 61 passed, 0 failed (`.\build.ps1 tests`).
- Emulator smoke test: PASS — boot, title (audio RMS 3956), menu, classic input, pause,
  arcade autoplay 1200 frames: 20 rows, 60 pieces, chain 3, error_code 0.

## 11. Commits

See `git log --oneline` (foundation, core engine, session layer, first playable ROM).
