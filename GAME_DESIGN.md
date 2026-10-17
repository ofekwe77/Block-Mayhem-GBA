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

## Challenge modifiers (PART 27.3, 30.7)
Optional rules for Classic, Chaos Arcade and Dual Board, toggled on the CHALLENGES page of the setup
screen (R switches pages; Classic shows the page once a challenge is unlocked). Each one is unlocked by
clearing a world's boss stage, is remembered in the save, and scales every point scored; several multiply.
The intro shows the active set ("6 CHALLENGES x2.43") and the score title keeps the multiplier in view.

| Challenge | Rule | Score | Unlock |
|---|---|---|---|
| TURBO | start 4 levels higher (faster gravity) | x1.25 | World 1 |
| NO HOLD | the hold slot is locked | x1.15 | World 2 |
| ONE NEXT | only the next piece is previewed | x1.10 | World 3 |
| FRENZY | special pieces twice as often (Classic gains the arcade specials) | x0.80 | World 4 |
| RISING TIDE | a junk row with one gap is telegraphed every 20 s of play (paused by Time Freeze) | x1.30 | World 5 |
| SHADOW STACK | landed blocks show only a dim outline in their colour (specials stay visible) | x1.50 | World 6 |

Adventure, Chaos Run and Boss Rush never use challenges (their balance is calibrated without them).

## Cosmetics (PART 35.4)
OPTIONS > CUSTOMIZE picks four styles with a live preview built from the real board tiles; locked
entries can be browsed and show how they are earned. Cosmetics change colours and decoration only:
every block theme keeps the seven shapes distinct and special-block symbols keep their own colours.

| Kind | Styles (unlock) |
|---|---|
| Blocks | CLASSIC, CONTRAST (colour-blind friendly, always available), NEON (World 4), PASTEL (15 stages), CHILI (campaign) |
| Board frame | WORLD (each world's metal), CHROME (World 5), CANDY (10 achievements), CARBON (a Chaos Run), GOLD (a full Boss Rush) |
| Title screen | FACTORY, MIDNIGHT (World 1), INFERNO (25 Overheats survived), GOLDEN (campaign, "CHAMPION EDITION") |
| Particles | SHARDS, STARS (5 achievements), SNOW (World 2), EMBERS (World 3), HEARTS (an S+ rank) |

New challenges and styles are announced on the results screen of the session that earned them.

## Secrets (PART 35)
Secret visual events never touch the rules:
- **Maintenance robot** — walks in along the conveyor, waves and cheers after a 6-wave chain or a 6-clear combo.
- **Machine malfunction** — six explosions within four seconds overload the factory: the scenery
  flickers, sparks leap from the edges, smoke rises, then a reboot flash and chime.
- **Lightning storm** — an 8-wave chain darkens the sky and four lightning bolts strike.
- **Dizzy boss** — a boss beaten in under 2.5 minutes pops out of its wreck with spinning eyes and its
  tongue out (in its own colour when a sprite palette is free).
- **S+ celebration** — an S+ rank sets off fireworks behind the results and the rank shimmers.
- **Easter egg** — L R L R SELECT on the title screen makes it rain chili peppers ("SPICY!").

Witnessing four different secrets unlocks the hidden achievement SPICE DETECTIVE (31 achievements, 6 hidden).
Five other hidden achievements reward advanced play (PART 35.2).

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
