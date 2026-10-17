# BLOCK MAYHEM — World Production Matrix (PART 49.4)

Status values: NOT STARTED · IN PROGRESS · IMPLEMENTED · TESTED · POLISHED · BLOCKED.
**TESTED** means automated host tests and/or emulator scenarios on the real ROM verify it (evidence key
below). Nothing is marked **POLISHED**: that is reserved for a review that includes a human playtest and a
listening review of the music (PART 63.3, task AUD-06), which an automated pipeline cannot provide.
Last updated 2026-10-17 (git checkpoint after the boss audit).

| # | World | Environmental graphics | Background animation | Unique gameplay mechanics | Stage 1 | Stage 2 | Stage 3 | Stage 4 | Boss logic | Boss graphics | Boss animation | Music | Sound effects | Tutorialization | Balance verification | Emulator verification | Final production review |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 | CLASSIC CHAOS | TESTED [G] | TESTED [A] | TESTED [M1] | TESTED [S] | TESTED [S] | TESTED [S] | TESTED [S] | TESTED [B] | TESTED [G] | TESTED [BA] | TESTED (technical) [AU] | TESTED [AU] | TESTED [T] | TESTED [BAL] | TESTED [E] | TESTED (automated + screenshot review) [R] |
| 2 | FROZEN FACTORY | TESTED [G] | TESTED [A] | TESTED [M2] | TESTED [S] | TESTED [S] | TESTED [S] | TESTED [S] | TESTED [B] | TESTED [G] | TESTED [BA] | TESTED (technical) [AU] | TESTED [AU] | TESTED [T] | TESTED [BAL] | TESTED [E] | TESTED (automated + screenshot review) [R] |
| 3 | VOLCANO MADNESS | TESTED [G] | TESTED [A] | TESTED [M3] | TESTED [S] | TESTED [S] | TESTED [S] | TESTED [S] | TESTED [B] | TESTED [G] | TESTED [BA] | TESTED (technical) [AU] | TESTED [AU] | TESTED [T] | TESTED [BAL] | TESTED [E] | TESTED (automated + screenshot review) [R] |
| 4 | ELECTRIC CITY | TESTED [G] | TESTED [A] | TESTED [M4] | TESTED [S] | TESTED [S] | TESTED [S] | TESTED [S] | TESTED [B] | TESTED [G] | TESTED [BA] | TESTED (technical) [AU] | TESTED [AU] | TESTED [T] | TESTED [BAL] | TESTED [E] | TESTED (automated + screenshot review) [R] |
| 5 | GRAVITY DIMENSION | TESTED [G] | TESTED [A] | TESTED [M5] | TESTED [S] | TESTED [S] | TESTED [S] | TESTED [S] | TESTED [B] | TESTED [G] | TESTED [BA] | TESTED (technical) [AU] | TESTED [AU] | TESTED [T] | TESTED [BAL] | TESTED [E] | TESTED (automated + screenshot review) [R] |
| 6 | THE FINAL MELTDOWN | TESTED [G] | TESTED [A] | TESTED [M6] | TESTED [S] | TESTED [S] | TESTED [S] | TESTED [S] | TESTED [B] | TESTED [G] | TESTED [BA] | TESTED (technical) [AU] | TESTED [AU] | TESTED [T] | TESTED [BAL] | TESTED [E] | TESTED (automated + screenshot review) [R] |

## Evidence key

- **[G] Graphics** — `tools/check_assets.py` (ASSET_MANIFEST.md): the world's far + near background layers and
  the boss parts parse as valid GBA assets and are referenced by the ROM; every stage of the world is
  screenshotted on the ROM by `tests/emu/stages_wN.txt` (`stage_W_I.png`, reviewed 2026-10-17).
- **[A] Background animation** — `src/bm_backdrop.cpp`: two parallax layers scrolling at different speeds,
  palette cycles per layer defined by `tools/gfx/backgrounds.py` (rotating lights and flows, blinking and
  flickering signals), ambient sprites per world (W1 sparks drifting up from the machines, W2 snow, W3 rising
  embers, W4 electric rain, W5 cosmic dust floating in every direction, W6 debris and embers) and the
  heat-shimmer raster wave of Worlds 3 and 6; the backdrop reacts to combos, Overheat and explosions (`excite`). Runs in every emulator
  scenario; the stress scenario measures its CPU cost (`prof_backdrop`).
- **[M1..M6] World mechanics** — host unit tests: M1 rows, heat, bombs, fire, chains (`clear.*`, `heat.*`,
  `bomb.*`, `fire.*`, `chain.*`); M2 ice coats and freezing (`ice.*`, `bomb.armor_absorbs_and_ice_absorbs`,
  `spark.ice_conducts`); M3 fire, lava pressure, molten rain (`fire.*`, stage sims with hazards firing);
  M4 sparks, surges, magnets, dual relay and CROSS OVER (`spark.*`, `magnet.*`, `chain.disabled_special_does_not_trigger`,
  `exploit.board_switching_alone_gives_nothing`, emulator `visuals.txt` CROSS OVER); M5 gravity shifts and
  settle cycles (`settle.sideways_and_upward_directions`, `compaction.four_directions_preserve_cell_count`,
  `abilities.gravity_pulse_compacts_in_the_chosen_direction`); M6 every hazard rotated (`stages.*` on 6-x).
  The full rule table is INTERACTIONS.md.
- **[S] Stages** — every stage: `stages.all_thirty_stages_have_a_route_to_completion` (12 bot runs, invariants
  every frame, hazards fire), `stages.every_stage_can_be_lost_and_idling_never_wins`, and the ROM scenario
  `stages_wN.txt` (loads, wins, results, progression). Per-stage records: STAGE_PASSPORTS.md.
- **[B] Boss logic** — `bossN.*_full_verification` (tests/test_bosses.cpp): initial HP, phases in order at
  their thresholds, pattern scheduling, telegraphs of at least 2 locks, every attack runs and shows its effect,
  consistent damage, defeat; `exploit.boss_damage_never_exceeds_the_per_resolution_cap`.
- **[BA] Boss animation** — `src/bm_boss_view.cpp` states intro, idle, telegraph, attack, recover, hit,
  phase change, defeat; screenshots `boss1_fight` .. `boss6_dual` (adventure.txt), `stress_boss_defeat`,
  `dizzy_boss` (secrets.txt). See BOSS_PRODUCTION_MATRIX.md.
- **[AU] Audio** — `tests/emu/audio.txt` (generated): the world track and its Overheat arrangement start, keep
  playing three seconds past their loop point, within RMS bounds; every sound effect is audible. "Technical"
  because whether the music is good is human judgement (AUD-06).
- **[T] Tutorialization** — stage cards show the core idea, goal icons and up to three tips per stage
  (`core/bm_stages.cpp` tips; e.g. World 1: controls, the Laser, bombs, chain ignition, boss damage); first
  visit of each world shows its introduction (story.txt `world_intro_1.png`); in-play hints for hazards and
  boss attacks name the threat before it acts (telegraphs).
- **[BAL] Balance** — `balance.human_paced_stage_report` (skilled and casual paced bots per stage, par / S
  targets derived from them), boss win rates and fight lengths, ability efficiency, exploits: BALANCE.md.
- **[E] Emulator** — `stages_wN.txt` plus `adventure.txt`, `worlds.txt` (music RMS per world), `boss_rush.txt`;
  all pass in `tools/verify_release.sh` (docs/verify_report.md).
- **[R] Review** — the 30 stage screenshots and the six boss fights were reviewed for readability, world
  identity and HUD clutter on 2026-10-17; the issues found that day were fixed (palette-exhaustion crash in
  long fights, combination attacks without targets, the Electric Emperor difficulty spike). A human playtest
  of feel and fun is still recommended before calling any world POLISHED.
