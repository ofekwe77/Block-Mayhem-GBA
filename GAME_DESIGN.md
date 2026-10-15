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
Classic, Chaos Arcade, Dual Board Relay. Adventure / Chaos Run / Boss Rush in development.
