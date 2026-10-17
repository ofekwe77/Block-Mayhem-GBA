# CONTINUE HERE — BLOCK MAYHEM development journal

Read this first when resuming. Then `TASKS.md` (status of every requirement), `RELEASE_VERIFICATION.md`
(what is verified and how) and `docs/verify_report.md` (latest automated verification run).

## 1. Current project state (2026-10-17, release candidate)

A real, bootable GBA ROM (`BlockMayhem.gba`, ~1.2 MB) built with devkitARM + Butano 21.9.0, warning-free.
Every mode is playable: ADVENTURE (6 worlds x 5 stages, 6 multi-phase bosses), CLASSIC, CHAOS ARCADE
(ability loadout + environmental events), CHAOS RUN (roguelite, 31 upgrades), DUAL BOARD (energy bridge,
CROSS OVER), BOSS RUSH, RECORDS, ACHIEVEMENTS (31, 6 hidden), OPTIONS (difficulty, ghost, shake, flashes,
volumes, CUSTOMIZE, sound test, credits). Challenge modifiers (6, unlocked by world clears) for Classic /
Arcade / Dual; unlockable cosmetics (19 styles in 4 kinds); six secret visual events + a title easter egg.

199 tasks in TASKS.md: 197 TESTED, 2 BLOCKED on human judgement (AUD-06 musical quality, META-06 fun /
feel: they need people, not tools). `.\build.ps1 verify` runs every gate (section 2).

## 2. How to verify everything

```
.\build.ps1 verify        # tools/verify_release.sh -> docs/verify_report.md, exit 0 only if all gates pass
```
Gates: toolchain, assets regenerate byte-identically, asset manifest (45 requirements), ROM build with 0
warnings + header check, all host unit tests and simulations, harness build, generated scenarios, every
emulator scenario in tests/emu (smoke, menus, ui, visuals, secrets, adventure, worlds, stages_w1..6,
chaos_run, boss_rush, save, story, stress, audio), performance bounds, documentation consistency
(generated passports up to date, every cited test exists), task inventory.

## 3. Recent work (this release cycle)

- Presentation: hard-drop trail, dual-board energy bridge, world-specific board frames, world-clear
  fireworks, T-TWIST call-outs, S+ celebration, secret events (robot, malfunction, lightning storm,
  dizzy boss), CUSTOMIZE screen with a live preview, title variations, chili-rain easter egg.
- Rules found and fixed by verification: combination attacks prepared no targets (three bosses); the
  Electric Emperor was as hard as the final boss (eased); sprite-palette exhaustion crash in long fights
  (text palettes no longer cached, optional fallbacks); bot replans after a relay switch (diagnostics).
- Verification: boss audits (6 entries), rule-by-rule tests (scoring, heat, Overheat, abilities, dual,
  difficulty, ranks), 30-stage emulator validation, UI scenario (pause, loadout, abilities, accessibility,
  volumes), verify_release pipeline, asset / ROM / docs checkers.

## 4. Known limitations (also in RELEASE_VERIFICATION.md)

- Human judgement has not been applied by a person: fun, music quality (AUD-06), animation appeal and
  difficulty satisfaction are evaluated only through bots and screenshots.
- Balance is calibrated with human-paced bots; real players may find the dual-board bosses harder.
- Tested in the mGBA core (headless harness); not tested on physical hardware or a flash cart.

## 5. Notes for future work

- CPU max statistic includes scene-loading frames; reset stats (cmd 6) before measuring gameplay.
- Diagnostic paths (UNLOCK_ALL, SET_BOSS_HP, ADD_HEAT, SET_COSMETIC, SET_CHALLENGES, SET_LOADOUT,
  SET_OPTIONS) are test-only mailbox commands, unreachable from the game UI.
- Scenarios without a save file see the first-launch intro: wait 40 frames, tap START, wait for scene 3,
  wait 30 frames (title input lock) before the next tap.
- Patch C/C++ sources with script files (Write tool), not shell heredocs: heredocs turn `\n` into newlines.
- Sprite palettes: 16 total. Never cache palettes for the whole scene; use the *_optional creators.

## 6. Important architecture decisions

- Simulation -> rules -> events -> presentation. `core/` never includes Butano; events never change rules.
- One dynamic regular BG holds boards + panels + meters (rebuilt each frame, uploaded once).
- Palette-bank budget: board layer 12 banks; backdrop layers 2 banks each (far + near).
- Backgrounds use `bpp_4_manual` with grit palette reduction (`-mRp`).
- Butano sprite fonts start at '!' (no space glyph in the image).
- Sprites drawn over the board must use bg_priority 0 (default 3 hides them behind the board layer).
- Chaos Run: the run seed fixes bosses, the 9-template line-up, layouts and offers; upgrades only touch
  the rules through `build_modifiers()` with the caps in `bm_config.h` (`run_*`).
