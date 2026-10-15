# Balance values

All numbers live in `core/bm_config.h` (single source of truth). This file explains them.
Values marked *(eval pending)* still need simulation-backed review (SIM-03, BAL-03, BAL-04).

## Movement and gravity
| Rule | Value |
|------|-------|
| DAS / ARR | 10 / 2 frames |
| Soft drop | 1 row per 2 frames (1 point per row) |
| Hard drop | instant (2 points per row) |
| Lock delay / resets | 30 frames / 12 (reset counter refreshes on a new lowest row) |
| Gravity frames per row, levels 1..12+ | 48 42 36 30 26 22 18 15 12 10 8 6 |
| Level up | every 10 rows (Classic/Arcade start at level 1, cap 30) |
| Difficulty gravity scaling | Beginner 140%, Standard 100%, Expert 75% of the table (min 4 frames) |
| Overheat gravity | 80% of the current value |

## Scoring
| Rule | Value |
|------|-------|
| Rows 1/2/3/4 | 100 / 300 / 500 / 800 × displayed level (internal level index + 1) |
| T-TWIST 0/1/2/3 rows | 400 / 800 / 1200 / 1600 × level; mini +100 |
| Back-to-back (quad or full T-TWIST) | ×1.5 on the line-clear base |
| Placement combo | 50 × combo index × level (index capped at 20) |
| Chain bonus by waves 2..8+ | 100 250 450 700 1000 1400 2000 × level |
| Special-effect cells | 25 per cell, capped at 500 per single effect |
| Perfect clear | 2000 × level |
| Overheat | line-clear points ×2; success bonus 1500 × level |

## Heat
| Rule | Value |
|------|-------|
| Range | 0–100 |
| Rows 1/2/3/4 | +13 / +30 / +45 / +65 |
| Extra chain waves | +5 each, max +25 per resolution |
| Back-to-back | +10; full T-TWIST with rows +10 |
| Magnet with no legal move | +3 |
| Chill (Cryo Titan) | heat gains ×50% |

## Overheat
| Rule | Value |
|------|-------|
| Trigger | heat reaches 100 (meter consumed to 0) |
| Duration | 720 frames of active play (paused by pause, resolution animations, boss transitions) |
| Success | clear 2 rows during the window |
| Failure | garbage rows: Beginner 1, Standard 2, Expert 2 with split gaps; warning shown in last 3 s |
| Heat during Overheat | accumulates to at most 50; Overheat cannot re-trigger while active |
| Shield Matrix | does not block Overheat-failure garbage (the risk is self-chosen) |

## Abilities (cost / cooldown in locked pieces)
| Ability | Cost | CD | Effect |
|---------|-----:|---:|--------|
| Laser Beam | 40 | 4 | column beam, 10 hits top-down, armor/ice absorb hits, boss +6 dmg |
| Time Freeze | 35 | 5 | 180 frames without automatic falling; boss countdown paused for those locks |
| Gravity Pulse | 45 | 5 | full compaction in a chosen direction, then cascades |
| Shape Shift | 25 | 3 | transform the falling shape (preview, legal only) |
| Nuclear Burst | 80 | 8 | 5×5 power-2 blast (strips armor, destroys iced cells), boss +14 dmg |
| Shield Matrix | 30 | 5 | blocks the next hostile garbage/attack within 10 locks |
| Chain Spark | 45 | 4 | charges a cell (max 2 charged, once per cell); pulses range 2 |
| Emergency Vent | 50 | 6 | removes the 8 highest cells, no score |
Floors: cost ≥ 15, cooldown ≥ 1 regardless of upgrades. Targeting times out after 300 frames without cost.

## Special cells
Bomb 3×3 (power 1); Fire range 1 (4 neighbours); Spark range 4 (cap 6 with upgrades); Ice 1 hp;
Armor 2 hp; Magnet 2 columns ±1 row; Rainbow 2 targets; max 32 waves per resolution (safety).
Distribution: one special piece every 5–7 locks (normal), 3–5 (advanced); rainbow weight 4 vs bomb 40.

## Boss combat
| Rule | Value |
|------|-------|
| HP | Block Monster 100, Cryo Titan 140, Magma Hydra 180, Electric Emperor 220, Gravity King 260, Core of Mayhem 350 *(eval pending)* |
| Damage rows 1/2/3/4 | 5 / 12 / 20 / 35 (+6 full T-TWIST) |
| Chain damage | +4 per extra wave, max +20 |
| Overheat | ×1.25 |
| Cap | 40% of max HP per resolution |
| Telegraph | ≥ 2 locked pieces (+1 Beginner) |
| Phase transition | 90 frames non-interactive, all timers frozen |

## Dual board
Forced switch after 4 locks; CROSS OVER = rows on both boards within 4 locks (+1000 × level, +15 heat);
2+ rows on one board: +6 heat and one row removed from the other board's pending garbage;
4 rows: cancels up to 2 pending garbage rows on the other board.

## Chaos Arcade events (PART 22.2)

Endless, no objectives. A telegraphed environmental event (3 locks of warning) rotates through garbage
drip, freeze cells, surge, molten rain, armor plates and a left/right gravity shift. The first event comes
16 locks in; every level brings events one lock sooner, down to 6 locks apart (`arcade_event_interval`,
`arcade_event_min_interval`). Shield Matrix blocks an event like any hazard.

## Chaos Run (PART 21.6 stacking caps — `core/bm_config.h` `run_*`, enforced in `build_modifiers()`)

| Upgrade effect | Per stack | Cap (incl. synergy) |
|---|---|---|
| HEAT ENGINE heat gain | +10% (max 3 stacks) | 140% (the session also clamps 50..150%) |
| CHAIN KING chain bonus | +25% (max 3) | 200% (DEMOLITION adds +25%) |
| OVERHEAT EXPERT success bonus | +50% (max 2) | 250% (MELTDOWN adds +50%) |
| PRECISION ENGINE triples / quads / full T-TWISTs | +20% (max 2) | 160% (CLOCKWORK adds +20%) |
| FINAL GAMBIT | x1.5 score while the stack is >= 70% of the visible rows | fixed 150% |
| COMBO BATTERY | +2 heat per combo step (max 2) | 6 (CAPACITOR adds +2) |
| IRON DEFENSE | 1 weakened garbage attack per encounter (max 2) | 3 (BUNKER adds +1) |
| EXPLOSIVE MASTER | LV1 bomb reach +1, LV2 bomb power 2 | reach +1, power 2 |
| FIRE STARTER | fire reach +1 (max 2) | 3 (WILDFIRE adds +1) |
| ELECTRIC SURGE | spark range +1 (max 2) | 6 = `spark_range_max` (STORM CELL adds +1) |
| MAGNETIC FIELD | LV1 +1 column, LV2 +1 row | 4 columns, 2 rows |
| RAINBOW MASTER | +1 rainbow target | 3 |
| CRYSTAL BREAKER | +1 damage vs ice / armor | 1 |
| LASER FOCUS | laser cost -5 (max 2) | -10; never below `ability_min_cost` 15 |
| FAST CHARGER | all cooldowns -1 (max 2) | -2; never below `ability_min_cooldown` 1 |
| TIME ENGINEER | Time Freeze +60 frames (max 2) | 300 frames total |
| SAFE VENT | Vent +2 cells (max 2) | 12 cells total |
| HOT START | +20 starting heat per encounter (max 2) | 40 |
| BOMB / STORM SUPPLY | +20 bomb / spark distribution weight (max 2) | +40 |
| SECOND CHANCE | one rescued top-out (top 4 rows vented) | 1 per run |
| REACTIVE ARMOR | absorbs the first hazard of every encounter | 1 per encounter |
| DOUBLE SPARK | one extra pulse per qualifying spark chain | once per resolution |

Run pacing: 3 acts x (3 encounters + boss). Encounter level = 2 / 4 / 6 (+1 from the third
encounter); bosses at level 3 / 5 / 7 with HP x0.70 / x0.90 / x1.10. Clear bonus 1500 x act,
bonus goal 1000 x act, boss bonus 5000 x act. Offer rarity odds (rare / legendary): act 1 22% / 4%,
act 2 28% / 8%, act 3 34% / 12%; the first card after a boss is always rare or better.
