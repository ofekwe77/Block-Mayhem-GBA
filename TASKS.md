# BLOCK MAYHEM — Task Inventory (PART 59)

Every requirement of `GAME_SPEC.md` (Parts 1–65) mapped to a trackable task.
`tools/task_report.py` parses the tables below to produce the counts in `PROGRESS_REPORT.md`.

**Status:** `NOT STARTED` · `IN PROGRESS` · `IMPLEMENTED` (code exists and runs) · `TESTED` (verified by automated test / emulator evidence) · `POLISHED` (functional + presentation reviewed) · `BLOCKED`
**Verify:** `-` none yet · `UNIT` host unit test · `SIM` automated simulation · `EMU` real ROM in the mGBA core (harness) · `HUMAN` needs human judgement (fun, music quality)

A task is only `TESTED` when the evidence column names the test / scenario that proves it (PART 59.3 definition of done).
Dependencies are listed by task id. Files are relative to the repository root.

## ENV — Environment, build, tooling (PART 1, 2, 38 phase 1, 48, 64)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| ENV-01 | TESTED | EMU | Real GBA toolchain: MSYS2 + devkitARM (gba-dev) + Butano 21.9.0 | 2.2 | - | README.md | ROM builds; `sprites` example + BlockMayhem.gba built |
| ENV-02 | TESTED | EMU | Build wrapper for non-ASCII project path (subst drive) | 2.2 | ENV-01 | build.ps1, tools/bm.sh | `.\build.ps1` builds ROM |
| ENV-03 | TESTED | EMU | Headless emulator harness on real mGBA core (input, screenshots, audio capture, RAM test interface) | 39.8, 57.3 | ENV-01 | tools/harness/* | tests/emu/smoke.txt PASS |
| ENV-04 | TESTED | UNIT | Host unit-test runner for the portable core | 31, 39 | - | tests/*.cpp | `build.ps1 tests` 61/61 |
| ENV-05 | NOT STARTED | - | `verify_release` pipeline (build + unit + sims + emulator scenarios + asset checks) | 56.1, 63.1 | ENV-03, ENV-04 | tools/verify_release.sh | |
| ENV-06 | IN PROGRESS | - | Git checkpoints at milestones | 41, 64.6 | - | .git | commits: foundation, core, session, playable |
| ENV-07 | TESTED | EMU | Test interface struct in ROM RAM + diagnostic mailbox (test path only) | 57.3 | ENV-03 | core/test_interface.h, src/bm_app.cpp | smoke.txt uses `cmd 2` |
| ENV-08 | IMPLEMENTED | - | Continuation via native `/loop` self-pacing (documented) | 48.2-48.4 | - | CONTINUE_HERE.md | |

## CORE — Falling-block engine (PART 4)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| CORE-01 | TESTED | UNIT | Board 10x20 with 2 hidden rows, bitboards, invariant validator | 4.1, 55.2 | - | core/bm_board.* | tests board.*, every sim frame validates |
| CORE-02 | TESTED | UNIT | Seven shapes, SRS rotation states, kick tables; O never rotates | 4.2 | CORE-01 | core/bm_piece.* | shapes.*, rotation.* |
| CORE-03 | TESTED | UNIT | Seeded seven-bag randomizer; 3+ previews | 4.3 | - | core/bm_randomizer.* | bag.* |
| CORE-04 | TESTED | UNIT | Hold once per piece, spawn orientation | 4.3 | CORE-02 | core/bm_player.* | hold.once_per_piece_and_spawn_orientation |
| CORE-05 | TESTED | UNIT | DAS 10 / ARR 2 / soft drop 1 row per 2 frames / hard drop | 4.4 | CORE-02 | core/bm_player.* | movement.* |
| CORE-06 | TESTED | UNIT | Lock delay 30 frames, 12 resets, anti-stall | 4.4 | CORE-05 | core/bm_player.* | lock.* |
| CORE-07 | IMPLEMENTED | SIM | Gravity table per level + difficulty scaling | 4.4, 30.1 | CORE-05 | core/bm_config.h, core/bm_session.cpp | sims run levels 1-12 |
| CORE-08 | IMPLEMENTED | EMU | Ghost piece, toggle in pause/options | 4.5 | CORE-02 | src/bm_board_view.cpp | screenshot classic_play.png |
| CORE-09 | TESTED | UNIT | Lock sequence; no spawn while board effects unresolved | 4.6 | CHAIN-01 | core/bm_player.cpp | session sims + validate() |
| CORE-10 | TESTED | UNIT | Top-out: block out and lock out | 4.6 | CORE-01 | core/bm_player.cpp | topout.* |
| CORE-11 | IMPLEMENTED | - | T-TWIST detection (3-corner, mini/full) + scoring | 7.5 | CORE-02 | core/bm_player.cpp | needs dedicated unit test |
| CORE-12 | IMPLEMENTED | EMU | Spawn chute row visible above the field | 4.1 | CORE-01 | src/bm_board_view.cpp | screenshots |

## SPEC — Special cells (PART 5)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| SPEC-01 | TESTED | UNIT | Bomb 3x3, armor absorbs, chained bombs | 5.2 | CHAIN-01 | core/bm_resolver.cpp | bomb.* |
| SPEC-02 | TESTED | UNIT | Fire: 4 orthogonal neighbours, thaws ice, triggers bombs/sparks | 5.3 | CHAIN-01 | core/bm_resolver.cpp | fire.* |
| SPEC-03 | TESTED | UNIT | Electric arcs range 4, stop at armor, conduct through ice | 5.4 | CHAIN-01 | core/bm_resolver.cpp | spark.* |
| SPEC-04 | TESTED | UNIT | Ice coat 1 hp, full-row thaw pulse then clear (no extra piece) | 5.5 | CHAIN-01 | core/bm_resolver.cpp | ice.* |
| SPEC-05 | TESTED | UNIT | Magnet: deterministic gap fill (closest, left first, supported gap, legal moves), heat on fizzle | 5.6 | CHAIN-01 | core/bm_resolver.cpp | magnet.* |
| SPEC-06 | TESTED | UNIT | Rainbow: up to 2 adjacent specials by priority, wild burst fallback | 5.7 | CHAIN-01 | core/bm_resolver.cpp | rainbow.* |
| SPEC-07 | TESTED | UNIT | Armor 2 hp, damage stages, finite row resolution | 5.8 | CHAIN-01 | core/bm_resolver.cpp | armor.* |
| SPEC-08 | TESTED | UNIT | Separate seeded distribution + guarantees (never breaks 7-bag) | 5.9 | CORE-03 | core/bm_randomizer.cpp | bag tests + sims |
| SPEC-09 | NOT STARTED | - | INTERACTIONS.md complete interaction table | 55.5 | SPEC-01..07 | INTERACTIONS.md | |
| SPEC-10 | IMPLEMENTED | EMU | Readable symbols + palette-animated cores for every special | 5, 23.2 | - | tools/gfx/tiles.py | arcade_autoplay.png shows bomb |
| SPEC-11 | TESTED | UNIT | Disabled specials (surge) and charged cells | 10.7, 17 | CHAIN-01 | core/bm_resolver.cpp | chain.disabled*, chain.charged* |

## CHAIN — Chain reaction engine (PART 6)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| CHAIN-01 | TESTED | UNIT | Deterministic wave state machine (scan/thaw/lines/waves/settle) | 6.2 | CORE-01 | core/bm_resolver.* | 26 specials tests |
| CHAIN-02 | TESTED | UNIT | Safety: single activation, bounded queues, max waves, invalid coords | 6.3, 55.3 | CHAIN-01 | core/bm_resolver.cpp | chain.dense_specials_terminate |
| CHAIN-03 | TESTED | UNIT | Line clear preserves classic behaviour; only effect gaps settle | 6.2 | CHAIN-01 | core/bm_resolver.cpp | clear.standard_clear_preserves_holes_and_overhangs |
| CHAIN-04 | IMPLEMENTED | EMU | Chain tier call-outs NICE! .. TOTAL MELTDOWN! with escalating effects | 6.4 | CHAIN-01 | src/bm_game_scene.cpp | needs screenshot evidence |
| CHAIN-05 | TESTED | UNIT | Settle directions down/up/left/right; full compaction | 18, 10.3 | CHAIN-01 | core/bm_resolver.cpp | settle.*, compaction.* |

## SCORE — Scoring (PART 7)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| SCORE-01 | TESTED | UNIT | 100/300/500/800 x displayed level | 7.1 | CHAIN-01 | core/bm_session.cpp | session.single_line_scores_100_at_level_1 |
| SCORE-02 | TESTED | UNIT | Soft drop 1 / hard drop 2 per row, never repeated | 7.2 | CORE-05 | core/bm_session.cpp | same test (+34 hard drop) |
| SCORE-03 | IMPLEMENTED | SIM | Placement combo 50 x index x level, capped | 7.3 | SCORE-01 | core/bm_session.cpp | needs unit test |
| SCORE-04 | IMPLEMENTED | SIM | Chain bonus by wave count, capped tier 8 | 7.4 | CHAIN-01 | core/bm_session.cpp | needs unit test |
| SCORE-05 | IMPLEMENTED | - | Back-to-back x1.5 for quads / full T-TWISTs only | 7.5 | CORE-11 | core/bm_session.cpp | needs unit test |
| SCORE-06 | IMPLEMENTED | UNIT | 25 pts per special-destroyed cell, capped 500 per effect | 7.6 | CHAIN-01 | core/bm_resolver.cpp | needs explicit cap test |
| SCORE-07 | IMPLEMENTED | UNIT | Perfect clear bonus | 7 | CHAIN-01 | core/bm_session.cpp | observed in test (2134 pts) |

## HEAT — Heat meter and Overheat (PART 8, 9, 61)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| HEAT-01 | IMPLEMENTED | SIM | Heat 0-100, gains 13/30/45/65, +5/wave capped 25, +10 B2B | 8.2 | SCORE-01 | core/bm_session.cpp | needs unit test of gains |
| HEAT-02 | TESTED | UNIT | Spending with exact single deduction | 8.3, 55.4 | ABIL-10 | core/bm_session.cpp | abilities.denied_without_heat_and_exact_deduction |
| HEAT-03 | TESTED | UNIT | Overheat at 100 consumes meter; no re-trigger while active | 9.1, 9.6 | HEAT-01 | core/bm_session.cpp | heat.overheat_consumes_meter_and_caps_gain |
| HEAT-04 | TESTED | UNIT | 720 active frames; pause stops it; resolution animations pause it | 9.2 | HEAT-03 | core/bm_session.cpp | heat.overheat_timer_stops_while_paused |
| HEAT-05 | IMPLEMENTED | EMU | Effects: faster gravity, x2 line score, boss dmg x1.25, visuals, hot music | 9.3 | HEAT-03, AUD-03 | core/bm_session.cpp, src/* | arcade autoplay reached Overheat label |
| HEAT-06 | IMPLEMENTED | - | Success: 2 rows -> bonus + celebration | 9.4 | HEAT-03 | core/bm_session.cpp | needs unit test |
| HEAT-07 | TESTED | UNIT | Failure: garbage by difficulty, visible warning, never instant death | 9.5 | HEAT-03 | core/bm_session.cpp | heat.overheat_failure_queues_visible_garbage |
| HEAT-08 | TESTED | UNIT | Heat during Overheat capped at 50 | 9.6 | HEAT-03 | core/bm_session.cpp | heat.overheat_consumes_meter_and_caps_gain |
| HEAT-09 | IMPLEMENTED | EMU | SPICE intensity labels MILD..INFERNO/OVERHEAT + meter colours | 61.1 | HEAT-01 | src/bm_game_scene.cpp | arcade_autoplay.png |

## ABIL — Active abilities (PART 10, 11)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| ABIL-01 | TESTED | UNIT | LASER BEAM 40/4: column targeting, 10-hit beam, armor damage, bomb triggers | 10.1 | ABIL-10 | core/bm_session.cpp | abilities.* (cost/cd); effect via resolver |
| ABIL-02 | IMPLEMENTED | SIM | TIME FREEZE 35/5: 3 s no gravity, boss countdown pause | 10.2 | ABIL-10 | core/bm_session.cpp | needs unit test |
| ABIL-03 | IMPLEMENTED | - | GRAVITY PULSE 45/5: direction choice, full compaction, cascades | 10.3 | CHAIN-05 | core/bm_session.cpp | needs unit test |
| ABIL-04 | IMPLEMENTED | - | SHAPE SHIFT 25/3: preview + legality | 10.4 | ABIL-10 | core/bm_session.cpp, src/bm_board_view.cpp | needs test |
| ABIL-05 | IMPLEMENTED | - | NUCLEAR BURST 80/8: 5x5 power-2 blast | 10.5 | ABIL-10 | core/bm_session.cpp | needs test |
| ABIL-06 | IMPLEMENTED | - | SHIELD MATRIX 30/5: blocks next hostile garbage/attack, not top-out | 10.6 | ABIL-10 | core/bm_session.cpp | needs test |
| ABIL-07 | IMPLEMENTED | - | CHAIN SPARK 45/4: charge a cell (max 2, once per cell) | 10.7 | ABIL-10 | core/bm_session.cpp | needs test |
| ABIL-08 | TESTED | UNIT | EMERGENCY VENT 50/6: remove 8 highest cells, no score | 10.8 | ABIL-10 | core/bm_session.cpp | abilities.vent_removes_highest_cells |
| ABIL-09 | IMPLEMENTED | EMU | Loadout of 3, SELECT cycles, R activates | 11.1 | - | core/bm_session.cpp, src/* | autoplay uses abilities |
| ABIL-10 | TESTED | UNIT | Targeting state, A confirm / B cancel / 5 s timeout without cost | 10, 11.1 | - | core/bm_session.cpp | abilities.targeting_timeout_cancels_without_cost |
| ABIL-11 | IMPLEMENTED | EMU | Cost / cooldown / disabled state shown in HUD | 8.3 | - | src/bm_game_scene.cpp | arcade_autoplay.png |
| ABIL-12 | IMPLEMENTED | EMU | Ability animations (beam, nuke blast, freeze tint, arrows, sparks, vent steam) | 10 | VIS-04 | src/bm_game_scene.cpp | needs screenshots |
| ABIL-13 | NOT STARTED | - | Ability unlocks through Adventure progression | 27.3 | ADV-01 | | |
| ABIL-14 | NOT STARTED | - | Arcade loadout selection screen | 22.2 | UI-02 | | |

## CTRL — Controls and input (PART 11, 32)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| CTRL-01 | TESTED | EMU | Default layout (D-pad, A/B rotate, L hold, R ability, SELECT cycle, START pause) | 11.1 | CORE-05 | src/bm_app.cpp | smoke.txt taps |
| CTRL-02 | IMPLEMENTED | - | Dual-board controls (SELECT switches board, single ability) | 11.2 | DUAL-02 | core/bm_session.cpp | needs test |
| CTRL-03 | IMPLEMENTED | EMU | Separate input contexts; menu repeat; input lock after transitions | 11.3 | - | src/bm_menu_scenes.cpp | |
| CTRL-04 | IMPLEMENTED | - | Pause never consumes timers or glitches movement | 11.3, 26.6 | - | src/bm_game_scene.cpp | needs emulator test |

## DUAL — Dual Board Relay (PART 12)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| DUAL-01 | IMPLEMENTED | - | Two 8x18 boards, shared queue, own grids/pieces/garbage/combo | 12.2, 12.4 | CORE-01 | core/bm_session.cpp | needs tests |
| DUAL-02 | IMPLEMENTED | - | One active board; inactive pauses; SELECT switches | 12.3 | DUAL-01 | core/bm_session.cpp | |
| DUAL-03 | IMPLEMENTED | - | Forced switch after 4 locks, countdown, never during animations | 12.3 | DUAL-02 | core/bm_session.cpp | countdown display missing |
| DUAL-04 | IMPLEMENTED | - | Cross-board interactions (heat+defence, spark charge, garbage cancel) | 12.5 | DUAL-01 | core/bm_session.cpp | needs tests |
| DUAL-05 | IMPLEMENTED | - | CROSS OVER combo within 4 locks; no switch-farming exploit | 12.6 | DUAL-04 | core/bm_session.cpp | needs tests |
| DUAL-06 | IMPLEMENTED | - | Failure if either board tops out; warnings for both | 12.7 | DUAL-01 | core/bm_session.cpp | |
| DUAL-07 | IN PROGRESS | - | Presentation: energy bridge, active highlight, flashes | 12.8 | DUAL-01 | src/* | layout exists, bridge missing |

## ADV — Adventure mode and stages (PART 13-19, 50)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| ADV-01 | IMPLEMENTED | - | Stage data model: objectives, hazards, layouts, specials, ranks | 13, 50.3 | - | core/bm_content.h | |
| ADV-02 | NOT STARTED | - | 30 stage definitions (bm_stages.cpp) | 14-19 | ADV-01 | | |
| ADV-03 | NOT STARTED | - | World map screen (icons, animated minis, locks, progress) | 13.2 | ADV-02 | | |
| ADV-04 | NOT STARTED | - | Stage intro cards, contextual tutorials | 14, 36 | ADV-02 | | |
| ADV-05 | NOT STARTED | - | Stage results with transparent ranks C..S+ | 27.2 | ADV-02 | src/bm_flow.cpp | stage_rank() exists |
| ADV-06 | NOT STARTED | - | Hazard: freezing events (W2) | 15 | ADV-01 | core/bm_session.cpp | logic implemented, untested |
| ADV-07 | NOT STARTED | - | Hazard: lava pressure meter (W3) | 16 | ADV-01 | core/bm_session.cpp | logic implemented, untested |
| ADV-08 | NOT STARTED | - | Hazard: settle-direction cycles / gravity shifts (W5) | 18 | ADV-01 | core/bm_session.cpp | logic implemented, untested |
| ADV-09 | NOT STARTED | - | Hazard: surges / mixed (W4, W6) | 17, 19 | ADV-01 | core/bm_session.cpp | logic implemented, untested |
| ADV-10 | NOT STARTED | - | STAGE_PASSPORTS.md for all 30 stages | 50.3 | ADV-02 | | |
| ADV-11 | NOT STARTED | - | Automated validation of all 30 stages (sim + emulator) | 50.10, 56.4 | ADV-02 | | |
| ADV-12 | NOT STARTED | - | World unlock progression + replay | 27.1 | ADV-02 | | |

## WORLD — World production parity (PART 14-19, 49)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| W1-ART | IMPLEMENTED | EMU | World 1 factory far+near layers, palette animation (gears, belt, lamps, neon) | 14, 23.3 | - | tools/gfx/backgrounds.py | classic_play.png |
| W2-ART | NOT STARTED | - | World 2 frozen factory layers + animation | 15 | - | | |
| W3-ART | NOT STARTED | - | World 3 volcano layers + animation | 16 | - | | |
| W4-ART | NOT STARTED | - | World 4 neon city layers + animation | 17 | - | | |
| W5-ART | NOT STARTED | - | World 5 gravity dimension layers + animation | 18 | - | | |
| W6-ART | NOT STARTED | - | World 6 final meltdown layers + animation | 19 | - | | |
| W-FRAME | IMPLEMENTED | - | World-specific board frame tints | 49.3 | - | src/bm_board_view.cpp | tints for 6 worlds; decor tiles missing |
| W-MATRIX | NOT STARTED | - | WORLD_PRODUCTION_MATRIX.md | 49.4 | - | | |
| W-INTRO | NOT STARTED | - | World introductions + world-complete celebrations | 49.3 | ADV-03 | | |

## BOSS — Boss framework and encounters (PART 20, 51)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| BOSS-FW-01 | IMPLEMENTED | - | Data-driven boss: HP, phases, piece-scheduled attacks, telegraphs | 20 | - | core/bm_session.cpp | needs tests |
| BOSS-FW-02 | IMPLEMENTED | - | Damage table, chain bonus cap, ability damage, Overheat bonus, 40% cap | 20.2 | BOSS-FW-01 | core/bm_session.cpp | needs tests |
| BOSS-FW-03 | IMPLEMENTED | - | Phase transitions at HP thresholds, visible, timers frozen | 20.3 | BOSS-FW-01 | core/bm_session.cpp | |
| BOSS-FW-04 | IMPLEMENTED | - | Fairness: no attacks in pause/transition, shield, freeze pauses countdown | 20.5 | BOSS-FW-01 | core/bm_session.cpp | needs tests |
| BOSS-ANIM | NOT STARTED | - | Boss animation controller (INTRO..FINISHED states) | 51.2 | - | | |
| BOSS-1 | NOT STARTED | - | Block Monster encounter, art, animation, defeat | 14 | BOSS-FW-01 | | |
| BOSS-2 | NOT STARTED | - | Cryo Titan | 15 | BOSS-FW-01 | | |
| BOSS-3 | NOT STARTED | - | Magma Hydra (multi-head) | 16 | BOSS-FW-01 | | |
| BOSS-4 | NOT STARTED | - | Electric Emperor (dual board) | 17 | BOSS-FW-01 | | |
| BOSS-5 | NOT STARTED | - | Gravity King | 18 | BOSS-FW-01 | | |
| BOSS-6 | NOT STARTED | - | Core of Mayhem (5 phases, final victory sequence) | 19 | BOSS-FW-01 | | |
| BOSS-MATRIX | NOT STARTED | - | BOSS_PRODUCTION_MATRIX.md | 51.6 | - | | |
| BOSS-VAL | NOT STARTED | - | Six boss validation entries (sim + emulator) | 51.5, 56.5 | BOSS-1..6 | | |

## RUN — Chaos Run roguelite (PART 21, 60)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| RUN-01 | NOT STARTED | - | 3 acts x (3 encounters + boss), seeded generation | 21.1 | ADV-01 | | |
| RUN-02 | NOT STARTED | - | 20+ upgrades with real effects | 21.3 | - | core/bm_content.h (modifiers) | modifiers wired into rules |
| RUN-03 | NOT STARTED | - | Rarity (common/rare/legendary) | 21.4 | RUN-02 | | |
| RUN-04 | NOT STARTED | - | Synergies + explicit stacking caps | 21.5, 21.6 | RUN-02 | | |
| RUN-05 | NOT STARTED | - | Upgrade selection screen (3 choices) | 21.2 | RUN-02 | | |
| RUN-06 | NOT STARTED | - | Run summary / defeat screens, saved stats | 21.7 | RUN-01 | | |
| RUN-07 | NOT STARTED | - | Roguelite tests: generation, caps, seeds, completion, failure | 39.6 | RUN-01 | | |

## MODE — Modes (PART 22)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| MODE-01 | TESTED | EMU | CLASSIC: no powers/specials/hazards, speed-up, high scores | 22.1 | CORE-* | src/bm_flow.cpp | smoke.txt classic |
| MODE-02 | IMPLEMENTED | EMU | CHAOS ARCADE endless with specials/heat/abilities | 22.2 | HEAT-*, ABIL-* | src/bm_flow.cpp | smoke.txt arcade autoplay 20 rows |
| MODE-03 | NOT STARTED | - | CHAOS ARCADE controlled environmental events | 22.2 | ADV-06..09 | | |
| MODE-04 | IMPLEMENTED | - | DUAL BOARD RELAY endless mode in menu | 22.5 | DUAL-* | src/bm_flow.cpp | needs emulator test |
| MODE-05 | NOT STARTED | - | ADVENTURE mode flow | 22.3 | ADV-* | | |
| MODE-06 | NOT STARTED | - | CHAOS RUN mode flow | 22.4 | RUN-* | | |
| MODE-07 | NOT STARTED | - | BOSS RUSH (unlock, sequence, lives, total time) | 22.6 | BOSS-* | | |

## VIS — Visuals and animation (PART 23, 24, 34, 43, 52, 62)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| VIS-01 | IMPLEMENTED | EMU | Shaded block art, specials, ice/armor states, ghost, flashes | 23.2 | - | tools/gfx/tiles.py | screenshots |
| VIS-02 | IMPLEMENTED | EMU | Parallax scrolling + palette-cycle animation engine | 23.3, 23.4 | - | src/bm_backdrop.cpp | |
| VIS-03 | IMPLEMENTED | EMU | Particle pool (sparks, smoke, embers, snow, crystals, debris, stars, electric, shards, dust) | 24.9 | - | src/bm_fx.cpp, tools/gfx/sprites.py | |
| VIS-04 | IMPLEMENTED | - | Explosions, laser beam, pop-ups, call-outs | 24.5 | VIS-03 | src/bm_fx.cpp | |
| VIS-05 | IMPLEMENTED | - | Shake profiles small/medium/large/boss/final; exact rest position | 24.6 | - | src/bm_fx.cpp | |
| VIS-06 | IMPLEMENTED | - | Hitstop (simulation paused, bounded) | 24.7 | - | src/bm_fx.cpp | |
| VIS-07 | IMPLEMENTED | EMU | Fade transitions between scenes | 24.8 | - | src/main.cpp | |
| VIS-08 | IMPLEMENTED | EMU | Line-clear flash + shards, quad drama | 24.4 | - | src/* | |
| VIS-09 | NOT STARTED | - | Hard-drop trail sprite | 24.3 | - | | dust only |
| VIS-10 | IMPLEMENTED | - | Reduced shake / reduced flash honoured | 34.4, 52.6 | - | src/bm_fx.cpp | options screen missing |
| VIS-11 | IMPLEMENTED | EMU | Title screen (logo, eye, gears, falling shards, PRESS START) | 26.1 | - | | title.png |
| VIS-12 | NOT STARTED | - | Secret visual events | 35.1 | - | | |

## AUD — Audio (PART 25, 53)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| AUD-01 | IMPLEMENTED | EMU | 44 original synthesized sound effects incl. all required events | 25.5 | - | tools/audio/sfx.py, src/bm_audio.cpp | sfx_count rises in harness |
| AUD-02 | IMPLEMENTED | EMU | 14 original tracks (title, menu, 6 worlds, boss, final, victory, game over, ending, classic) | 25.2 | - | tools/audio/songs.py | title audio RMS 3956 |
| AUD-03 | IMPLEMENTED | - | Dynamic music: hot arrangements switched at same position during Overheat | 25.3 | AUD-02 | src/bm_audio.cpp | needs emulator check |
| AUD-04 | IMPLEMENTED | - | Volume settings applied | 34.4 | - | src/bm_audio.cpp | options screen missing |
| AUD-05 | NOT STARTED | - | Per-track audio validation (all tracks play, loop, non-silent) | 53.5 | AUD-02 | | |
| AUD-06 | NOT STARTED | HUMAN | Musical quality review | 53.3 | AUD-02 | | human judgement |

## UI — User interface (PART 26)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| UI-01 | IMPLEMENTED | EMU | Title screen | 26.1 | - | src/bm_menu_scenes.cpp | title.png |
| UI-02 | IN PROGRESS | EMU | Main menu with all modes; locked entries explain why | 26.2 | MODE-* | src/bm_menu_scenes.cpp | 3 implemented modes listed |
| UI-03 | IMPLEMENTED | EMU | Gameplay HUD single layout | 26.3 | - | src/bm_game_scene.cpp | screenshots |
| UI-04 | NOT STARTED | - | Boss HUD layout | 34.2 | BOSS-1 | | |
| UI-05 | IN PROGRESS | - | Dual HUD layout | 34.3 | DUAL-* | | |
| UI-06 | TESTED | EMU | Original pixel fonts (small + large) | 26.4 | - | tools/gfx/font.py | text renders in screenshots |
| UI-07 | IMPLEMENTED | - | Danger indicators (frame pulse, alarm) | 26.5 | - | src/bm_board_view.cpp | |
| UI-08 | IMPLEMENTED | EMU | Pause menu RESUME/RESTART/CONTROLS/GHOST/QUIT(confirm) | 26.6 | - | src/bm_game_scene.cpp | pause.png |
| UI-09 | NOT STARTED | - | Options screen (difficulty, ghost, shake, flash, volumes) | 34.4 | - | | |
| UI-10 | NOT STARTED | - | Records screen | 26.2 | SAVE-* | | |
| UI-11 | NOT STARTED | - | Achievements screen | 28 | ACH-* | | |
| UI-12 | NOT STARTED | - | Credits screen | 37 | - | | |
| UI-13 | NOT STARTED | - | Sound test (unlockable) | 27.3 | AUD-02 | | |
| UI-14 | IMPLEMENTED | EMU | Results screen | 37 | - | src/bm_menu_scenes.cpp | |

## PROG / ACH / SAVE — Progression, achievements, persistence (PART 27-29)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| PROG-01 | NOT STARTED | - | Sequential world unlocking by boss defeat | 27.1 | ADV-02 | | |
| PROG-02 | IMPLEMENTED | - | Rank formula (score, time, garbage, secondary objective) | 27.2 | ADV-01 | src/bm_flow.cpp | needs test |
| PROG-03 | NOT STARTED | - | Rewards: abilities, modes, cosmetics, sound test, modifiers | 27.3 | PROG-01 | | |
| ACH-01 | NOT STARTED | - | 25+ achievements incl. hidden ones, triggers + progress | 28, 35.2 | - | | |
| ACH-02 | NOT STARTED | - | Unlock notifications | 28 | ACH-01 | | |
| SAVE-01 | IMPLEMENTED | - | Save format: magic, version, size, CRC-32, two slots, sequence | 29.1 | - | core/bm_save.* | needs unit tests |
| SAVE-02 | IMPLEMENTED | EMU | SRAM read/write with read-back verification | 29 | SAVE-01 | src/bm_app.cpp | save_writes counter |
| SAVE-03 | NOT STARTED | - | Save tests: first launch, invalid, version mismatch, full completion | 29.2, 39.7 | SAVE-01 | | |
| SAVE-04 | NOT STARTED | - | Emulator persistence across ROM restarts | 57.6 | SAVE-02 | | |

## BAL / SIM — Balance and simulation (PART 30, 31, 54)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| BAL-01 | IMPLEMENTED | - | Difficulty settings Beginner/Standard/Expert (gravity, telegraphs, penalties) | 30.1 | - | core/bm_session.cpp | options UI missing |
| BAL-02 | NOT STARTED | - | BALANCE.md with every numeric rule | 30.7 | - | | |
| SIM-01 | TESTED | UNIT | Deterministic replay (seed + inputs) | 31.1, 55.6 | - | tests/test_session.cpp | session.deterministic_replay |
| SIM-02 | IMPLEMENTED | UNIT | Bot strategies: random, height, hole, line-clear, resource-aware | 31.3 | - | core/bm_bot.* | bots.all_strategies_run_without_invalid_states |
| SIM-03 | NOT STARTED | - | Balance simulation runner + metrics report (many seeds) | 31.3, 54.1 | SIM-02 | | |
| SIM-04 | NOT STARTED | - | Exploit tests (score/heat/overheat/ability/cooldown/boss/achievement/save) | 31.4, 58.2 | SIM-02 | | |
| BAL-03 | NOT STARTED | - | Ability efficiency analysis table | 54.2 | SIM-03 | | |
| BAL-04 | NOT STARTED | - | Boss HP vs simulated damage evaluation | 20.1 | BOSS-*, SIM-03 | | |

## HW — Hardware and performance (PART 33, 57.5, 62)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| HW-01 | IMPLEMENTED | EMU | CPU usage measured in ROM (bn::core) and exported to harness | 33.6 | ENV-07 | src/bm_app.cpp | cpu_avg 433 per mille (incl. loading) |
| HW-02 | NOT STARTED | - | Stress scenarios (full board, chains, boss, particles, dual) measured | 33.6, 57.5 | HW-01 | | |
| HW-03 | NOT STARTED | - | Sprite budget audit (<=128 OAM, scanline) | 33.3 | - | | |

## STORY / SECRET (PART 35, 36)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| STORY-01 | NOT STARTED | - | Intro cutscene | 36.1 | - | | |
| STORY-02 | NOT STARTED | - | World intros, boss entrances, dialogue cards | 36 | ADV-* | | |
| STORY-03 | NOT STARTED | - | Final ending + credits roll + statistics | 19, 36 | BOSS-6 | | |
| SECRET-01 | NOT STARTED | - | Secret events, easter eggs, unlockable cosmetics | 35 | - | | |

## DOC — Documentation and release (PART 41, 42, 59, 63)

| ID | Status | Verify | Task | Spec | Deps | Files | Evidence |
|----|--------|--------|------|------|------|-------|----------|
| DOC-01 | IMPLEMENTED | - | TASKS.md (this file) | 59.1 | - | TASKS.md | |
| DOC-02 | IMPLEMENTED | - | CONTINUE_HERE.md journal | 41, 48.7 | - | CONTINUE_HERE.md | |
| DOC-03 | IMPLEMENTED | - | PROGRESS_REPORT.md (generated counts) | 59.5 | DOC-01 | PROGRESS_REPORT.md | |
| DOC-04 | IMPLEMENTED | - | RELEASE_VERIFICATION.md (truthful, currently NOT READY) | 63.4 | - | RELEASE_VERIFICATION.md | |
| DOC-05 | IMPLEMENTED | - | README.md (dependencies, build, mGBA, controls, modes, saves, troubleshooting) | 42 | - | README.md | |
| DOC-06 | IN PROGRESS | - | GAME_DESIGN.md | 41 | - | GAME_DESIGN.md | |
| DOC-07 | IN PROGRESS | - | TESTING.md | 41 | - | TESTING.md | |
| DOC-08 | IMPLEMENTED | - | CREDITS.md (original assets, licences) | 42 | - | CREDITS.md | |
| DOC-09 | NOT STARTED | - | ASSET_MANIFEST.md | 56.3 | - | | |
