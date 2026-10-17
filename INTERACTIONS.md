# BLOCK MAYHEM — Special-Cell Interaction Table (PART 55.5)

Every interaction between cells, special effects, abilities and environmental attacks, as implemented by
`core/bm_resolver.cpp` (chain resolution) and `core/bm_session.cpp` (abilities, hazards, boss attacks).
Nothing here is left to accidental behaviour: each rule is a branch in that code, and the test named in
the last column checks it on the host (`tools/bm.sh tests`).

## 1. Resolution order (applies to everything below)

1. **Scan.** Complete rows are found. A complete row that still holds an ice or armour coat first gets a
   *thaw pulse*: every coat in the full rows loses one point, then the rows are scanned again (ice 1 point,
   armour 2 points), so a full row always clears without needing another piece.
2. **Line wave.** Complete rows are cleared in place. Every special inside them (not disabled, not already
   activated) is queued for the next wave; charged cells queue a pulse.
3. **Special waves.** Queued specials activate at their positions, one wave at a time, in queue order.
   Anything they destroy that is itself a special is queued for the following wave.
4. **Settle.** Cleared rows are removed (classic behaviour). Gaps opened by special effects close in the
   current settle direction; each cell moves by the number of cells destroyed beyond it, so holes that existed
   before the effect are preserved.
5. Repeat from 1 until the board is stable.

Safety rules: a special activates at most once per resolution; queues are bounded; the number of waves is
capped (dense boards terminate); coordinates are validated at every hit; points per single effect are capped
at `special_cell_points_cap` (500) and boss damage per resolution at `boss_chain_damage_cap` (20).
Tests: chain.dense_specials_terminate, chain.overlapping_explosions_count_cells_once,
bomb.chain_two_bombs_and_no_double_activation, exploit.special_points_are_capped_per_effect,
exploit.boss_damage_never_exceeds_the_per_resolution_cap.

## 2. How a hit works

All destructive effects call one rule, `resolver::hit(x, y, power)`:

| Target cell | Result of a hit with power P |
|---|---|
| Empty | nothing |
| **Stone** | nothing: stone is immune to every special effect and ability (only a line clear removes it) |
| Ice-coated (1 point) / armoured (2 points) | the coat loses P points; if P exceeds what the coat had left, the rest reaches the cell and destroys it, otherwise only the coat breaks |
| Normal block, garbage | destroyed |
| Special (bomb, fire, spark, magnet, rainbow) | destroyed and queued to activate in the next wave |
| Disabled special (surge) | destroyed, does **not** activate |
| Charged cell (Chain Spark) | destroyed and queues a 2-cell pulse |

## 3. Interaction matrix: effect (row) against target cell (column)

| Effect ↓ / target → | Normal / garbage | Bomb, fire, spark, magnet, rainbow | Ice coat | Armour coat | Stone | Shape of the effect | Test |
|---|---|---|---|---|---|---|---|
| **Line clear** | removed | removed; activates next wave | full row: thaw pulse first | full row: two thaw pulses | removed with the row | the complete row | ice.full_row_thaws_then_clears_without_new_piece, armor.full_row_two_pulses_then_clears, clear.single_double_triple_quad |
| **Bomb** | destroyed | destroyed and chained | coat absorbs the blast (power 1) | 1 point off | untouched | 3x3 around the bomb (power 1); EXPLOSIVE MASTER: power 2 / +1 reach | bomb.radius_three_by_three_from_line_clear, bomb.armor_absorbs_and_ice_absorbs, bomb.explosive_master_power_two_breaks_armor_coat |
| **Fire** | the first occupied cell in each of the 4 directions burns | chained (fire sets off a bomb) | thawed (coat removed, cell stays) | 1 point off | stops nothing (hit ignored) | 4 orthogonal neighbours; FIRE STARTER reaches past one gap | fire.hits_four_neighbours_only, fire.thaws_ice_and_triggers_bomb, fire.fire_starter_reaches_past_one_gap |
| **Electric spark** | every occupied cell along 4 arcs of length 4 | chained | ice **conducts**: the coat breaks and the arc continues | armour **blocks** the arc (and takes 1 point) | stone **insulates**: the arc stops | 4 straight arcs, range 4 (ELECTRIC SURGE +1) | spark.range_four_and_armor_blocks, spark.ice_conducts, spark.arc_length_limited |
| **Magnet** | pulls the nearest ordinary block sideways into the nearest supported gap (own row, then rows below/above, up to 2 columns away) | specials are never moved | coated cells are never moved | never moved | never moved | one move; fizzles (no effect) when no legal gap exists | magnet.fills_supported_gap_and_column_settles, magnet.fizzle_grants_no_illegal_move, magnet.completes_a_row_through_settling |
| **Rainbow** | wild burst on its 4 neighbours when no special is adjacent | activates up to 2 adjacent specials by priority bomb > spark > fire > magnet > rainbow | coated specials are not linked | not linked | not hit | 8 neighbours | rainbow.triggers_two_by_priority, rainbow.wild_burst_without_neighbours |
| **Laser** (ability) | first 10 occupied cells of the column, top down | destroyed and chained | coat breaks (power 1) | 1 point off | skipped (counts toward the 10) | one column | abilities.laser_hits_ten_cells_of_its_column_with_coat_and_stone_rules |
| **Nuclear Burst** (ability) | destroyed | destroyed and chained | coat and cell destroyed (power 2) | coat destroyed, cell survives | untouched | 5x5 at the chosen centre | abilities.nuclear_burst_power_two_in_five_by_five |
| **Chain Spark** (ability) | marks up to 2 cells as charged; when destroyed they pulse range 2 | a pulse sets specials off | pulse thaws | pulse 1 point | pulse stops | 4 arcs of length 2; stone cannot be charged | chain.charged_cell_pulses, abilities.chain_spark_charges_cells_but_never_stone |
| **Gravity Pulse** (ability), **gravity shift** (hazard / boss) | compacts the whole board in the chosen direction | specials move with their cell, nothing activates | coats move with their cell | move with their cell | moves too | full compaction, cell count preserved | compaction.four_directions_preserve_cell_count, abilities.gravity_pulse_compacts_in_the_chosen_direction |
| **Emergency Vent** (ability) | removes the 8 highest occupied cells | removed **without** activating | removed with the cell | removed with the cell | removed | top-down, left-to-right scan | abilities.vent_removes_highest_cells |
| **Settle direction** (World 5 hazard) | gaps from special effects close sideways / upwards / downwards | — | — | — | — | per resolution | settle.sideways_and_upward_directions |

## 4. Environmental attacks and boss attacks against cells

All are telegraphed for at least `boss_min_telegraph_pieces` (2) locks before they act. Shield Matrix (and
REACTIVE ARMOR in a Chaos Run) absorbs the next boss attack or environmental event outright; its telegraph
marks are removed and the lava meter resets.

| Attack | Effect on the board | Interaction rules |
|---|---|---|
| Freeze cells (World 2), ice injection, crystal shards, frozen wall | surface cells of the tallest columns gain an ice coat (1 point); shards / wall add ice blocks | never coats stone or an already coated cell; marked a lock ahead (frost mark) |
| Armour plates, armour injection | surface cells gain an armour coat (2 points) | never coats stone or coated cells; marked a lock ahead (armour mark) |
| Lava pressure (World 3) | the meter rises on every lock that clears no row; at its limit a telegraphed molten garbage row rises and the meter resets | the row has one gap; a shield absorbs the eruption (and the meter still resets) |
| Molten rain, fire breath | telegraphed columns receive molten blocks on top | blocks are ordinary garbage-coloured cells |
| Surge (World 4) | specials locked during the surge arrive disabled | disabled specials are destroyed by effects but never activate (chain.disabled_special_does_not_trigger) |
| Garbage rows, punch, cross garbage | rows with one gap pushed in from the bottom | the gap is telegraphed; RISING TIDE challenge rows are not stopped by a shield |
| Gravity shift / flip / left / right / up | full compaction (see Gravity Pulse) | nothing activates; the cell count is preserved |
| Heat drain, chill | heat removed / heat gain halved for some locks | no board change |
| Forced switch (dual) | the active board changes | the other board's state is untouched |

## 5. Determinism (PART 55.6)

Pieces, special decoration, hazards, garbage gaps and boss choices each draw from their own seeded random
stream; decorative effects (particles, shakes, secret events) use separate generators. A given seed and input
sequence always produce the same game: tests session.deterministic_replay and
challenges.presentation_challenges_only_scale_the_score, and the ROM's own boss fight reproduced the host
result exactly (stages_w6.txt).
