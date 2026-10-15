# BLOCK MAYHEM: THE SPICE PROTOCOL — Game Design (as implemented)

This document describes what the code actually does. Numbers: `BALANCE.md`. Status: `TASKS.md`.

## Architecture
- `core/` — portable deterministic simulation (no Butano, no heap, integer math):
  board, pieces/rotation, randomizer, resolver (chain reactions), player (falling piece),
  session (rules, scoring, heat, abilities, garbage, hazards, bosses, dual board), bots, save format.
- `src/` — GBA presentation with Butano: scenes, board renderer, backdrops, effects, HUD, audio.
- Data flow: input → session.update() → board/rule changes → events → presentation (never the reverse).

## Board and pieces
10×20 logical board (2 hidden rows; the lower hidden row is drawn as a spawn chute above the field).
Dual board: two 8×18 boards. Seven shapes, standard modern rotation system with 5-test kicks,
O never rotates. Pieces spawn in the hidden rows and drop one row immediately.
Seven-bag randomizer (seeded); a separate seeded distributor attaches at most one special cell to a piece.

## Line clears and chain reactions
After a lock the resolver repeats: scan full rows → thaw coats in full rows (one pulse removes one coat
point; rows clear once no coat remains, without needing another piece) → clear full rows in place (wave)
→ activate the specials that were in them at their original positions (next wave) → each special hit
queues further specials (one wave per generation) → settle → rescan.
Settling removes cleared rows like a classic game, and closes gaps made by special effects by moving each
cell by the number of cells destroyed beyond it in the settle direction — pre-existing holes survive.

### Special cells
- **Bomb** — 3×3 around its position.
- **Fire** — the four orthogonal neighbours (thaws ice, damages armor, triggers specials).
- **Spark** — arcs up to 4 cells in 4 directions; armor stops an arc (and takes a hit); ice conducts.
- **Magnet** — looks for the closest supported empty gap within 2 columns (own row, below, above;
  left first) and slides an adjacent ordinary block into it; the column it left drops by one.
  No legal move: +3 heat.
- **Rainbow** — triggers up to 2 adjacent specials (bomb > spark > fire > magnet > rainbow, then
  position order); without special neighbours it bursts its 4 neighbours.
- **Ice coat (1 hp)** / **Armor coat (2 hp)** — absorb hits; frozen specials cannot activate.
- **Stone** — indestructible by effects, clears with full rows.
- **Charged cell** (Chain Spark) — pulses range 2 when destroyed/cleared.

## Heat, Overheat and abilities
Heat 0–100 earned by clears, chains and back-to-backs; intensity labels MILD, WARM, HOT, SPICY,
INFERNO, OVERHEAT. Reaching 100 starts Overheat (12 s of active play): faster gravity, doubled
line-clear score, boss damage ×1.25, hot music arrangement. Two rows during the window = success bonus;
otherwise garbage rows arrive (warned during the last 3 s).
Up to three abilities are equipped (Dual Board: one). R activates; targeted abilities freeze the piece
and enter an aiming state (A fire, B cancel, 5 s timeout, nothing spent until fired).

## Bosses (framework)
Bosses have HP, up to 5 phases at HP thresholds, and repeating attack patterns scheduled by locked
pieces. Every attack is telegraphed at least two locks ahead (columns, marked cells, incoming garbage).
Phase changes are a visible, non-interactive transition with all timers frozen.

## Modes (current build)
- **Adventure** — 6 worlds x 5 stages (stage 5 = boss). World map, stage cards with goal icons,
  per-stage objectives, bonus goals, hazards, layouts and ranks C..S+.
- **Chaos Run** — the roguelite (below).
- **Classic** — no specials, heat, abilities or hazards; speed-up and high scores.
- **Chaos Arcade** — endless score attack with specials, heat, Overheat and up to three abilities chosen
  on the loadout screen; telegraphed environmental events rotate through six kinds and speed up with
  the level.
- **Dual Board Relay** — two 8x16 boards, SELECT switches, one equipped ability.
- **Boss Rush** — unlocked by the World 3 boss. Every boss already defeated in Adventure, in world
  order, with the score carried between fights. Two recoveries: a lost fight is retried until they
  run out. Total time and score are shown; the best score and the best full six-boss time are saved.
  Quitting a fight or giving up on the card ends the rush.

## Chaos Run (PART 21, 60)
A run is three acts; each act is three generated encounters and a boss (12 fights). The run seed
fixes the boss line-up (one per act, never repeated; single-board bosses only), the nine encounter
templates (never repeated within a run) and every encounter's layout, hazards and bonus goal.
The run map previews the whole line-up with objective icons so players can plan their build.

Encounter templates (`core/bm_run.cpp`): DEMOLITION YARD (bombs), SPARK GRID (sparks), FIRE PIT
(fires), ICE FIELD (ice), MAGNET MINE (magnets), LINE RUSH (rows), CHAIN LAB (2+ wave chains),
QUAD QUARRY (quads + rows), SURVIVAL SHAFT (survive pieces under garbage drip), PRESSURE COOKER
(lava pressure, act 2+), GRAVITY WELL (settle cycles, act 2+), HEAT WAVE (master an Overheat, act 2+),
ARMOR DEPOT (armor plates, act 2+), CHAOS STORM (mixed hazards + chains, act 3). Special-cell
objectives carry distribution guarantees so they are never impossible. Targets, level, hazard
frequency and starting garbage escalate per act.

After every cleared encounter the player picks one of three seeded offers from 31 upgrades
(23 effects + 8 ARMORY picks that equip a new ability, max three abilities). Rarity is common /
rare / legendary; offers never include upgrades that cannot work with the current build (LASER FOCUS
needs the laser, SAFE VENT the vent, ARMORY needs a free slot). Seven synergies (DEMOLITION, MELTDOWN,
STORM CELL, CLOCKWORK, BUNKER, WILDFIRE, CAPACITOR) add a bounded extra effect when both halves are
owned; the selection screen flags cards that complete one. All caps are listed in BALANCE.md.
Persistent progress only tracks records and the upgrade codex (no power carried between runs).
Quitting an encounter or abandoning from the map ends the run as a defeat.
