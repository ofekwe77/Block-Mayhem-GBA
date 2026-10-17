# BLOCK MAYHEM — Boss Production Matrix (PART 51.6)

One checklist per boss. Statuses: NOT STARTED · IN PROGRESS · IMPLEMENTED · TESTED · POLISHED · BLOCKED.
No boss is marked POLISHED: its mechanics are verified by automated audits and its presentation was reviewed on
emulator screenshots, but a human playtest and a listening review of the boss music (AUD-06) are still open.
Data: `core/bm_bosses.cpp`, `core/bm_config.h` (`boss_hp`); presentation: `src/bm_boss_view.cpp`,
`tools/gfx/bosses.py`. Verification entries: `tests/test_bosses.cpp` (`bossN.*_full_verification`).
Last updated 2026-10-17.

## Common checklist (applies to all six)

| Item | Status | Evidence |
|---|---|---|
| Multi-phase definition, phases change at HP thresholds, one at a time | TESTED | full_verification: phases reached = phase count, no skipped phase, HP at or below the threshold |
| Attack scheduling follows each phase's pattern | TESTED | full_verification: every telegraphed attack is the next pattern entry (shield blocks counted) |
| Telegraphs at least 2 locks ahead (targets shown on the board) | TESTED | full_verification: locks between telegraph and attack >= `boss_min_telegraph_pieces` |
| Every attack kind runs and has an effect | TESTED | full_verification: attack kinds seen and with effect = all kinds of all phases (half the fights without Shield Matrix) |
| Damage rules (rows, specials, abilities; per-resolution cap) | TESTED | full_verification (HP falls by exactly the reported damage, never rises); `exploit.boss_damage_never_exceeds_the_per_resolution_cap` |
| Defeat: HP 0 wins the stage, destruction sequence plays | TESTED | full_verification; ROM `stages_w1..6.txt` (results after the sequence), `stress_boss_defeat.png` |
| Shield Matrix blocks a telegraphed attack | TESTED | full_verification counts shield blocks; INTERACTIONS.md section 4 |
| Unlock progression (next world, Boss Rush from World 3, challenge, music, cosmetics) | TESTED | ROM `stages_wN.txt` (worlds_unlocked), `secrets.txt` (rewards note), `challenges.unlocked_by_world_boss_clears` |
| Intro card (name, epithet), boss HP bar, phase banner | TESTED | `boss1_fight` .. `boss6_fight` screenshots (adventure.txt) |
| Animation states: intro, idle, telegraph, attack, recover, hit flash, enraged tint, phase change, defeat explosions + collapse | TESTED | adventure.txt / stress.txt fights; `boss1_attack.png`, `boss6_phase.png`, `boss6_dual.png` |
| Secret: dizzy face after a quick defeat (< 2.5 min) | TESTED | `secrets.txt` `dizzy_boss.png`, secrets_seen bit 8 |
| Boss music (`mus_boss` / `mus_final` + Overheat arrangements) | TESTED (technical) | `audio.txt` RMS and loop checks; musical quality: AUD-06 (human) |
| Boss sound effects (hit, attack, charge, defeat) | TESTED | `audio.txt` SFX audibility |
| Boss Rush inclusion | TESTED | `boss_rush.txt` (all six in sequence) |

## Individual records

| Boss | HP | Phases (from HP %, attack interval in locks) | Attacks (pattern per phase) | Signature | Graphics (parts) | Balance (skilled pace) | Status |
|---|---:|---|---|---|---|---|---|
| 1 THE BLOCK MONSTER — "FACTORY BRUTE" | 100 | 100% / 8 · 70% / 7 · 35% / 6 | punch, garbage, punch · punch, armour injection, garbage · punch, armour injection, garbage, punch | big wind-up punches: garbage with a shared gap you can plan for | body, eye, mouth, arm (boss1_*) | 34 s fight, 7/8 audit wins | TESTED |
| 2 THE CRYO TITAN — "WARDEN OF THE DEEP COLD" | 140 | 100% / 6 · 65% / 5 · 30% / 5 | ice injection, crystal shards, chill · frozen wall, ice injection, crystal shards · frozen wall, chill, crystal shards, ice injection | ice everywhere: coats, ice blocks on telegraphed columns, a wall of ice in your lowest column, halved heat | body, head, crystal, breath | 49 s, 7/8 | TESTED |
| 3 THE MAGMA HYDRA — "THREE HEADS, ONE FURNACE" | 180 | 100% / 6 · 72% / 5 · 45% / 6 · 18% / 4 | fire breath, heat drain · molten rain, fire breath · combo (molten rain + fire breath), molten rain, fire breath · molten rain, fire breath, heat drain | one head per phase; molten blocks rain on telegraphed columns and the furnace drains your heat | base, neck, head, fire | 58 s, 8/8 | TESTED (combo targets fixed 2026-10-17) |
| 4 THE ELECTRIC EMPEROR — "SOVEREIGN OF THE GRID" | 220 | 100% / 6 · 65% / 6 · 30% / 6 (dual board) | surge, cross garbage · forced switch, garbage, surge · cross garbage, surge, forced switch, garbage | rules both relay boards: disables your specials, attacks the board you leave, forces switches | body, face, orb, bolt | 62 s, 16/24 measured (tuned 2026-10-17) | TESTED |
| 5 THE GRAVITY KING — "LORD OF FALLING THINGS" | 260 | 100% / 6 · 75% / 6 · 45% / 5 · 20% / 4 | gravity left, gravity right · gravity up, garbage · gravity flip, gravity up, garbage · combo (gravity flip + crystal shards), gravity up, garbage | bends the board: compactions in every direction, holes sinking to the floor | body, head, ring, cube | 83 s, 23/24 measured | TESTED |
| 6 THE CORE OF MAYHEM — "HEART OF THE SPICE PROTOCOL" | 350 | 100% / 6 · 80% / 5 · 60% / 6 · 40% / 5 · 15% / 4 (second board unseals in phase 5) | punch, garbage, armour injection · ice injection, fire breath, surge · gravity left, gravity up, gravity right · cross garbage, forced switch, crystal shards · four combos of two learned attacks | every earlier boss's attacks in turn, then two at once on both boards | eye, iris, heart, ring, tentacle, wing | 95 s, 14/24 measured | TESTED |

Balance figures: BALANCE.md (Bosses: HP against damage). The dual-board bosses are the hardest; the final boss
remains the hardest fight after the Emperor tuning.
