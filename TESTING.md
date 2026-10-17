# Testing

Two layers of automated verification plus one command that runs everything. Results below are real runs
(not projections).

## 0. Release verification — `.\build.ps1 verify` (`tools/verify_release.sh`)

Runs, in order, and reports PASS / FAIL for each step (a missing tool, test or scenario is a FAIL, never a
skip; exit code 0 only when all pass); the report is written to `docs/verify_report.md`:

1. toolchain present (devkitARM, Butano, Python, host g++, mGBA development build);
2. assets regenerate byte-identically from `tools/gfx` and `tools/audio`;
3. asset manifest check (`tools/check_assets.py`, writes ASSET_MANIFEST.md): every required presentation
   element maps to files that parse in the expected GBA format, sound data that is audible, and items the ROM
   source uses;
4. ROM build with zero compiler warnings, then the header check (`tools/check_rom.py`: entry branch, boot logo,
   complement checksum, title / game code, SRAM save-type marker);
5. all host unit tests and simulations;
6. the emulator harness build;
7. generated scenarios regenerated from data (30 stage scenarios, the audio scenario);
8. every emulator scenario in `tests/emu/` on the freshly built ROM;
9. performance bounds from the stress scenario (overrun frames at most 1%, sprite peak within 128);
10. documentation consistency (generated passports up to date, every test cited in INTERACTIONS.md and the
    production matrices exists);
11. task inventory: no task left NOT STARTED / IN PROGRESS / BLOCKED except those that need human judgement.

## 1. Host-side tests (portable core, `tests/`)

Run: `.\build.ps1 tests` (builds `tests/build/run_tests.exe` with the MSYS2 UCRT g++ and runs it).
Optional filter: `.\build.ps1 tests bomb`.

| Suite | What it proves |
|-------|----------------|
| shapes.* | 4 distinct minos per rotation; 4 rotations return to spawn; SRS reference states; O never rotates |
| rotation.* | spawn positions; I wall kick; rotations never overlap cells; T kick legality |
| bag.* | every bag holds all seven; deterministic per seed; max gap 12 |
| movement.* | walls; DAS 10 / ARR 2 timing; hard drop lands on ghost; soft drop rows |
| lock.* | 30-frame lock delay; reset limit prevents infinite stalling |
| hold.* | once per piece; returns in spawn orientation |
| clear.* | single/double/triple/quad; classic behaviour preserves holes and overhangs |
| topout.* | block out; lock out |
| board.* | garbage rows have one gap; overflow reports top-out |
| bomb/fire/spark/ice/armor/magnet/rainbow.* | every special-cell rule incl. edges, coats, chains |
| chain.* | dense boards terminate; overlapping explosions count once; disabled / charged cells |
| settle/compaction.* | four settle directions; compaction preserves cell counts |
| session.* | bot game validity; deterministic replay; scoring of a single + hard drop |
| heat.* | Overheat consumes meter, caps gain at 50, failure garbage, pause stops timer |
| abilities.* | denial without heat, exact deduction, cooldown in locked pieces, timeout without cost; real activation through targeting of the Laser (10-hit budget, coats, stone), Nuclear Burst (5x5 power 2), Chain Spark (stone refused), Gravity Pulse (direction, coats travel), Emergency Vent |
| bots.* | 5 strategies x 3 seeds: no invalid state in 2-minute games |
| stages.* | all 30 Adventure stages x 3 bots x 4 seeds: every stage won, 0 invalid states, 0 timeouts, hazards and boss attacks fire (`tests/build/stage_report.md`); every stage can be lost and idling never wins (idle runs lose in 16-77 s) |
| boss1..6.* | one verification entry per boss (PART 56.5), skilled-paced fights with and without abilities: initial HP, phases in order at their thresholds, pattern scheduling, telegraphs >= 2 locks, every attack runs and shows its effect, consistent damage, defeat |
| challenges.* / cosmetics.* | challenge definitions, world-clear unlocks, multiplied score percents, TURBO / NO HOLD / FRENZY / RISING TIDE rules on the real session, presentation-only challenges change nothing but the score; cosmetic unlock rules, accessibility theme always available, save round trip |
| run.* | 31-upgrade pool well formed; maxed build respects every cap through the real session rules; same seed = same run; 300 seeded runs: offers always eligible and distinct, no repeated encounters or bosses, post-boss rare card; every encounter template at every act and every run boss won by bots; 6 full bot runs (`tests/build/run_report.md`) |

| achievements.* | definitions; nothing unlocks without events; live play unlocks + progress without double counting after merge; campaign / run / rush flags; MASTER OF MAYHEM and ARSENAL accumulation |

| save.* | 512-byte layout, CRC-32 check value, blank SRAM = first launch, every flipped byte detected, version mismatch / impossible values reset, newest valid slot wins, interrupted write keeps the previous slot, full-completion round trip |

| arcade.* | Chaos Arcade events: >= 10 events in 6 minutes, >= 5 kinds, always telegraphed, gap shrinks to the floor, never "won", no invalid state |

| balance.* | human-paced calibration of all 30 stages (difficulty-curve assertions) and the ability efficiency table (`balance_report.md`, `ability_report.md`) |
| exploit.* | input spam scores nothing, special points cap, boss damage cap with a maxed build, board switching yields nothing, resource invariants under heavy play, SECOND CHANCE once per run |

Latest run (2026-10-17): **111 passed, 0 failed.**

Every simulated frame also calls `session::validate()` (board/heat/cooldown/boss invariants).

## 2. Emulator tests (real ROM in the mGBA core)

`tools/harness/bm_harness.c` links the static libmgba built by `tools/harness/build_libmgba.sh`.
Scenario scripts (`tests/emu/*.txt`) press buttons, wait for RAM test-interface fields, capture
PNG screenshots and measure audio RMS. Diagnostic mailbox commands (start a mode with autoplay, add
heat, set boss HP...) are a test path only and cannot be reached from the game UI.

| Scenario | Result (2026-10-17) |
|----------|---------------------|
| smoke.txt — boot, title music, menu, Classic input, pause, Arcade autoplay | PASS (error_code 0) |
| adventure.txt — world map, stage card, stage 1-1, all six bosses, phase changes, defeat sequence | PASS |
| worlds.txt — first stage of every world + dual stage 4-3, music RMS per world | PASS (RMS 4244..6041) |
| chaos_run.txt — run map, encounters under autoplay, results, upgrade choice, act 1 boss, act 2 map | PASS (4 cleared, 4 upgrades, error_code 0) |
| boss_rush.txt — diagnostic unlock, rush card, all six bosses in sequence (HP cut after each intro), summary | PASS (6/6, complete, error_code 0) |
| save.txt (`--save` file) — fresh boot, option change, stage 1-1 clear, console reset: settings, clear and achievements restored | PASS |
| story.txt (`--save` file, first launch) — intro cutscene beats, title, World 1 introduction, final ending, credits roll back to the title | PASS |
| stress.txt — 2 min Chaos Arcade, final boss + destruction, 1 min Dual Board, full-board detonations (single and dual): CPU per frame, overruns, sprite and tile-set budgets | PASS (see Performance) |
| audio.txt (generated by `tools/gen_audio_scenario.py` from the XM headers) — all 14 tracks start and keep playing 3 s past their loop point, 8 Overheat arrangements play, victory / game-over stop, all 44 sound effects audible (`audio_check` RMS bounds) | PASS (music RMS 3198-5900, SFX 210-5010) |
| visuals.txt — hard-drop trail right after a manual drop, dual-board energy bridge at rest and during a real CROSS OVER (bot play) | PASS (cross_combos 1, trail and bridge draw counters) |
| stages_w1..6.txt (generated by `tools/gen_stage_scenarios.py`) — every Adventure stage on the ROM: load, play to victory with the autoplay bot, results, stages_cleared / worlds_unlocked progression, no text palette fallback | PASS (30/30 won) |
| secrets.txt — title easter egg (L R L R SELECT chili rain), quick boss win with the dizzy face and the reward note, S+ fireworks, the four title variations, eight block / frame / particle themes in play, all six challenges in Classic, a detonation that calls the robot, the storm and the malfunction | PASS (secrets_seen 63, challenges 63, error_code 0) |
| menus.txt — full main menu, options + volume, CUSTOMIZE (theme change, locked entry, particle preview), sound test (audio), credits, achievements, records pages, loadout setup, Chaos Arcade with the live achievement banner | PASS |

Scenarios that boot without a save file see the first-launch intro and skip it with START.
Screenshots from the latest runs: `screenshots/tmp/`.

## Needs human judgement
Automated checks cannot judge whether the game is fun, whether the music is good or whether the difficulty
feels right to a person: see RELEASE_VERIFICATION.md (human verification) and task AUD-06.

## 3. Performance (PART 33.6, 57.5) — `tests/emu/stress.txt`

CPU is measured by the ROM per frame (per mille of a 16.7 ms frame; the diagnostic bot's own planning time
is excluded because a human player costs nothing there). An *overrun* is a frame that needed more than one
frame of CPU. Latest run (2026-10-16):

| Section | Avg CPU | Worst frame | Overrun frames | Sprite items | Sprite tile sets |
|---|---:|---:|---:|---:|---:|
| Chaos Arcade, 2 min of fast bot play | 35% | 149% | 17 / 6048 (0.3%) | 115 | 116 |
| Core of Mayhem fight + destruction sequence | 49% | 221% | 10 / 855 (1.2%) | 116 | 100 |
| Dual Board Relay, 1 min | 35% | 139% | 5 / 3212 (0.2%) | 116 | 102 |
| 16-row field of specials detonated (12-wave chain) | 40% | 127% | 2 / 443 | 113 | 89 |
| Same on both dual boards | 37% | 96% | 0 / 445 | 108 | 82 |

Engine limits: sprite items and sprite tile sets are raised to 192 (`Makefile` USERFLAGS) because exceeding
the default 128 is fatal; the hardware still shows at most 128 sprites, and the measured peaks stay under 128.
Sprite VRAM peaks at 521 of 1024 tiles.

Optimisations made from these measurements (before: 48% average in quiet play, 6% overrun frames in arcade
play, 160% worst frame in the detonation test):
- Board renderer: incremental drawing keyed on a board revision counter (only the cells under last frame's
  piece and ghost are repainted when the board is unchanged), hot loop compiled as ARM code into IWRAM.
- Board palette animation edits a CPU-side copy and uploads once per frame at most.
- Effects: particle and explosion tiles cached per frame (no per-frame tile lookups), pooled sprites created
  at scene load, per-frame budgets (12 particles, 2 explosions), division-free random numbers.
- Chain call-outs and score pop-ups use pooled glyph labels with pre-created glyph tiles (no VRAM allocation
  mid-chain); colour-only label changes swap palettes instead of regenerating text; the HUD score is a glyph label.
- Cosmetic events (one per destroyed cell) drain at 16 per frame; sounds start at most once per 3 frames per
  effect and 4 per frame; the per-frame invariant validation of the diagnostics runs every 8th frame.
- Section profiling stays in the ROM (`prof_*` fields of the test interface) for future regressions.

## 4. Defects found by verification (and fixed)
- **Combination attacks without targets** (found by the boss audit `tests/test_bosses.cpp`, 2026-10-17): a core
  combo did not prepare its parts, so molten rain + fire breath landed nothing and ice / armour combos marked no
  cells. Both parts are now prepared and telegraphed; three bosses use combos (BALANCE.md).
- **Electric Emperor difficulty spike** (boss audit + 24-seed measurement): as hard as the final boss; eased
  (attack interval, starting level) so the final boss stays the hardest fight.
- **Test bot after a relay switch** (diagnostics only): the bot now replans whenever the active board changes.
- **Sprite palette exhaustion in long fights** (found by stages_w6.txt, 2026-10-17): the glyph-label text cache kept
  the palette of every text colour used in a scene alive; with the Core of Mayhem's palettes all 16 sprite palettes
  were taken and the next new palette crashed the ROM (Butano error screen). Text palettes are no longer cached (a
  colour lives only while a sprite shows it), every text palette request is optional with a fallback (the text keeps
  its colour, counted in `text_failures`), and the dizzy-boss face uses an optional palette. Every scenario now
  asserts `text_failures == 0`. The harness gained `--log` to print the game's own debug log (palette manager dump).
