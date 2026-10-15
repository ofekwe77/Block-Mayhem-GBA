# MASTER DEVELOPMENT DIRECTIVE

FIRST READ IT ALL!

# BLOCK MAYHEM: THE SPICE PROTOCOL

## A Complete, Premium-Quality Game Boy Advance Arcade Puzzle Adventure

You are Claude Code, acting as an entire independent game-development studio.

You are simultaneously responsible for:

- Game direction and creative vision
- Gameplay programming
- Game systems architecture
- Game design
- Gameplay balancing
- Puzzle design
- Level design
- Boss encounter design
- Pixel art direction and asset creation
- Animation design and implementation
- Sound design
- Original music composition
- User interface and UX
- Hardware optimization
- Testing and quality assurance
- Build engineering
- ROM generation
- Technical documentation

Your task is to **design, implement, build, and test a complete, polished, original Game Boy Advance video game** called:

# BLOCK MAYHEM

## THE SPICE PROTOCOL

**Genre:** Action Puzzle / Arcade / Strategy / Adventure / Roguelite

**Platform:** Nintendo Game Boy Advance

**Final executable:** `BlockMayhem.gba`

**Development environment:** Real GBA development toolchain.

**Primary objective:** Produce an actual playable GBA ROM, with genuine GBA-compatible code and assets, suitable for use in the mGBA emulator and, subject to hardware testing, real Game Boy Advance hardware.

This is not a browser game.

This is not a mockup.

This is not a technical demonstration.

This is not a simple falling-block prototype.

This is a complete, carefully designed arcade game intended to feel like an ambitious commercial GBA release from the early 2000s.

The game must combine the accessibility and mechanical precision of classic falling-block puzzle games with the intensity of an action arcade game, the spectacle of a 1990s run-and-gun game, the strategic depth of modern roguelikes, and the satisfaction of elaborate chain-reaction puzzle systems.

The result should feel inventive, expressive, mechanically deep, spectacular for the hardware, and genuinely fun.

**Your responsibility is to build the game, not merely describe how it could be built.**

---

# PART 1 — YOUR OPERATING INSTRUCTIONS

## 1.1 Absolute development requirements

You must work directly with the project files.

You must create the necessary source code, assets, build scripts, tests, configuration files, and documentation.

You must compile the project into a genuine `.gba` ROM.

You must perform meaningful validation.

You must investigate and fix compilation problems instead of presenting uncompiled code as finished work.

You must preserve a clean, organized, understandable codebase.

Do not stop after generating a design document.

Do not stop after creating the main gameplay loop.

Do not substitute a desktop executable, JavaScript application, web game, or video for the requested ROM.

Do not present decorative animations that merely pretend gameplay systems exist.

Every major gameplay mechanic must be implemented as working game logic.

Every mode must have a real objective, playable progression, and a meaningful ending or failure condition.

Use original game assets. Do not copy copyrighted graphics, music, logos, characters, or other assets from existing commercial games.

Although the design takes inspiration from classic falling-block games, create an original identity, original presentation, original terminology, and original visual assets.

## 1.2 Development philosophy

The game must prioritize five qualities:

1. Excellent moment-to-moment gameplay.
2. Predictable, learnable, balanced game rules.
3. Impressive graphics, animations, music, and feedback.
4. Meaningful replayability and strategic variety.
5. Stable performance and authentic GBA compatibility.

Do not sacrifice gameplay readability to create excessive visual effects.

Do not sacrifice mechanical consistency to make the game superficially surprising.

Do not implement complexity merely for complexity's sake.

Every major feature should contribute to at least one of the following:

- Player expression
- Strategic decision-making
- Increasing mastery
- Meaningful risk and reward
- Exciting audiovisual feedback
- Variety between play sessions

The fundamental experience must remain enjoyable even when all extraordinary powers are disabled.

That is the standard by which all additional systems should be judged.

## 1.3 Make decisions independently

You have creative and technical authority over implementation details not explicitly specified below.

Do not interrupt development to ask routine design questions.

Select sensible solutions, document important decisions, and continue.

When two requirements conflict, favor:

1. Hardware correctness.
2. A stable, working ROM.
3. Core gameplay quality.
4. Clear and fair game rules.
5. Content completeness.
6. Visual spectacle.

These priorities are not permission to omit content silently.

If a technical limitation forces a compromise, implement the strongest practical alternative, document it, and clearly identify any incomplete requirements.

Never claim a feature is implemented when it is not.

Never claim the ROM has been tested in an emulator unless you actually tested it.

---

# PART 2 — TECHNICAL PLATFORM AND ARCHITECTURE

## 2.1 Target hardware

Develop specifically for the original Game Boy Advance hardware.

Work within its fundamental constraints:

- Screen resolution: 240 × 160 pixels.
- Approximately 60 display refreshes per second.
- ARM7TDMI processor.
- Limited system RAM.
- Limited video RAM.
- Hardware sprites and tiled backgrounds.
- Hardware palette constraints.
- Limited sound-processing resources.
- Physical GBA buttons.
- No modern GPU.
- No native filesystem.
- No internet access.
- No dependence on operating-system services.

Do not assume unlimited memory, textures, floating-point performance, sprite counts, audio channels, or CPU time.

Use fixed-size structures where practical.

Avoid dynamic memory allocation during gameplay.

Avoid expensive computations every frame.

Use deterministic integer or fixed-point gameplay calculations wherever practical.

The simulation must be independent of rendering.

Gameplay must remain correct if visual effects are delayed or omitted.

## 2.2 Recommended technology

Preferred implementation:

**C++ using Butano and devkitARM.**

Butano should be the default choice because of its support for GBA sprites, backgrounds, graphics assets, animation systems, audio, and hardware-oriented optimization.

Before development:

1. Inspect the existing environment.
2. Identify available toolchains.
3. Determine whether devkitARM and Butano are installed.
4. Consult the actual installed engine API and examples.
5. Establish a working build pipeline.
6. Compile a minimal GBA ROM to validate the environment.

Use the documented APIs of the installed version.

Do not invent Butano functions.

Do not rely on APIs merely because their names seem plausible.

If Butano is unavailable and installation cannot be completed, use a technically appropriate alternative based on devkitARM and GBA libraries.

Preserve the ability to build an authentic ROM.

Useful upstream references include:

- https://github.com/GValiente/butano
- https://gvaliente.github.io/butano/
- https://devkitpro.org/
- https://github.com/devkitPro/gba-examples

Inspect actual documentation whenever library compatibility is uncertain.

## 2.3 Architecture

Create a modular project rather than one enormous source file.

A suggested structure:

```text
BlockMayhem/
│
├── Makefile
├── README.md
├── GAME_DESIGN.md
├── BALANCE.md
├── TESTING.md
├── CONTINUE_HERE.md
├── CREDITS.md
│
├── source/
│   ├── main.cpp
│   ├── game/
│   │   ├── Game.cpp
│   │   ├── GameState.cpp
│   │   ├── GameLoop.cpp
│   │   └── GameConfig.cpp
│   │
│   ├── puzzle/
│   │   ├── Board.cpp
│   │   ├── Tetromino.cpp
│   │   ├── PieceGenerator.cpp
│   │   ├── Collision.cpp
│   │   ├── Rotation.cpp
│   │   ├── LineClearing.cpp
│   │   ├── Gravity.cpp
│   │   └── ChainResolver.cpp
│   │
│   ├── powers/
│   │   ├── SpecialCells.cpp
│   │   ├── PowerManager.cpp
│   │   ├── HeatSystem.cpp
│   │   └── Abilities.cpp
│   │
│   ├── modes/
│   │   ├── ClassicMode.cpp
│   │   ├── ArcadeMode.cpp
│   │   ├── AdventureMode.cpp
│   │   ├── RogueliteMode.cpp
│   │   └── DualBoardMode.cpp
│   │
│   ├── world/
│   │   ├── WorldManager.cpp
│   │   ├── StageManager.cpp
│   │   ├── StageObjectives.cpp
│   │   ├── HazardManager.cpp
│   │   └── BossManager.cpp
│   │
│   ├── progression/
│   │   ├── Upgrades.cpp
│   │   ├── Achievements.cpp
│   │   ├── Unlocks.cpp
│   │   └── SaveData.cpp
│   │
│   ├── presentation/
│   │   ├── Renderer.cpp
│   │   ├── Animation.cpp
│   │   ├── ParticleSystem.cpp
│   │   ├── ScreenEffects.cpp
│   │   ├── UI.cpp
│   │   └── Menus.cpp
│   │
│   ├── audio/
│   │   ├── MusicManager.cpp
│   │   └── SoundManager.cpp
│   │
│   └── input/
│       └── InputManager.cpp
│
├── graphics/
├── audio/
├── tools/
├── tests/
├── screenshots/
└── build/
```

Adapt the precise structure to the engine and build conventions.

The architecture must cleanly separate:

**Game simulation → Game rules → Events → Presentation.**

For example:

Clearing a row should produce a gameplay event.

That event can then trigger:

- A score update.
- Heat generation.
- Damage to a boss.
- Particle animations.
- Sound effects.
- Screen shake.
- Achievement checks.

Visual effects must not be responsible for modifying gameplay rules.

---

# PART 3 — CORE GAME CONCEPT

## 3.1 The central fantasy

The player is controlling an experimental energy-processing machine in a chaotic universe where falling geometric structures are unstable energy sources.

By arranging and destroying blocks, the player generates extraordinary amounts of energy.

This energy powers machines, weapons, defensive systems, and supernatural abilities.

The universe contains six increasingly unstable regions.

Each region has different physical properties, hazards, enemies, and bosses.

The player must master the fundamental puzzle mechanics, discover combinations of powers, and ultimately defeat the entity responsible for destabilizing the universe.

The presentation should be playful, exaggerated, energetic, and full of personality.

Avoid overly serious storytelling.

The tone should feel like a lost arcade masterpiece.

## 3.2 The three interconnected gameplay pillars

The entire game is built around three systems.

### Pillar A — Precision puzzle gameplay

The player must:

- Position falling pieces.
- Rotate them.
- Manage empty spaces.
- Complete horizontal rows.
- Plan future placements.
- Recover from mistakes.
- Develop increasingly efficient strategies.

### Pillar B — Power-based action gameplay

The player gains access to:

- Explosions.
- Lasers.
- Fire.
- Electricity.
- Time manipulation.
- Magnetic effects.
- Gravity manipulation.
- Defensive abilities.
- Chain reactions.
- Temporary transformations.

### Pillar C — Strategic progression

The player must:

- Choose upgrades.
- Build combinations of abilities.
- Manage limited resources.
- Adapt to different environments.
- Learn boss behavior.
- Decide when to take risks.
- Develop different playstyles.

These pillars must support each other.

The game must never become a shallow puzzle game with unrelated visual explosions.

---

# PART 4 — THE CORE FALLING-BLOCK ENGINE

This is the most important technical system.

Implement it correctly before building elaborate features around it.

## 4.1 Board dimensions

Standard single-board gameplay:

- 10 columns.
- 18 visible rows.
- 2 hidden spawn rows.

Use a logical 10 × 20 board, with the top two rows serving as the hidden spawn area.

The exact visual layout should preserve an attractive and readable screen composition.

Recommended single-board block size:

8 × 8 pixels.

This allows an 80 × 144 pixel visible playfield.

Use the remaining screen space for:

- Score.
- Level.
- Heat meter.
- Next pieces.
- Hold piece.
- Current ability.
- Boss information.
- Objectives.
- Small animations.

The board must remain visually dominant.

## 4.2 Standard pieces

Implement all seven conventional four-cell geometric shapes:

- I
- O
- T
- S
- Z
- J
- L

All shapes must rotate correctly.

Implement reliable collision detection.

Support appropriate wall kicks.

Use a documented and internally consistent rotation system.

If using familiar modern rotation behavior, implement it correctly rather than approximating it.

The O piece must not rotate into unintended positions.

The I piece must handle wall interactions correctly.

Rotations must never move pieces through occupied cells.

## 4.3 Piece randomization

Use a seven-bag randomizer.

Every bag contains exactly one of each shape.

Shuffle the bag using a deterministic seeded generator.

When the bag is exhausted, generate another shuffled bag.

Avoid unfair sequences caused by uncontrolled randomization.

Keep at least three upcoming shapes visible in standard modes.

Allow players to hold one shape.

Holding must be limited to once per spawned piece, until that piece locks.

The held shape must return to its defined spawn orientation.

Piece-generation behavior should be unit-testable.

## 4.4 Movement rules

Implement responsive controls.

Recommended starting parameters:

- Initial horizontal repeat delay: 10 frames.
- Automatic horizontal repeat: every 2 frames.
- Soft drop: approximately one row every 2 frames.
- Hard drop: immediate.
- Lock delay: 30 frames.
- Maximum lock-delay resets: 12.
- Prevent indefinite stalling.

Use a gravity-speed table rather than an uncontrolled exponential function.

Suggested frames per downward cell:

| Level | Frames |
| ----- | -----: |
| 1     |     48 |
| 2     |     42 |
| 3     |     36 |
| 4     |     30 |
| 5     |     26 |
| 6     |     22 |
| 7     |     18 |
| 8     |     15 |
| 9     |     12 |
| 10    |     10 |
| 11    |      8 |
| 12+   |      6 |

Treat these as starting balance values.

Test whether they feel appropriate with actual GBA input and frame timing.

Level progression must not accelerate the game so aggressively that strategic powers become irrelevant.

## 4.5 Ghost piece

Display a translucent outline indicating where the current piece would land if hard-dropped.

Requirements:

- Accurate landing calculation.
- Immediate updates after movement.
- Immediate updates after rotation.
- Correct collision handling.
- Visually distinguishable from the active piece.
- Low rendering overhead.

Allow the ghost piece to be disabled in settings.

## 4.6 Locking and top-out

A piece locks when its lock conditions are satisfied.

After locking:

1. Commit the cells.
2. Detect completed rows.
3. Resolve relevant special effects.
4. Apply line-clearing rules.
5. Update score and resources.
6. Resolve additional chain reactions.
7. Check victory and failure conditions.
8. Spawn the next piece.

Do not spawn a new falling piece while unresolved board effects could change its collision state.

A loss occurs when the next piece cannot legally spawn or an explicitly defined top-out condition occurs.

Do not randomly end the game because an animation temporarily occupies a spawn location.

---

# PART 5 — SPECIAL BLOCKS

The blocks themselves are one of the game's defining features.

A tetromino still consists of four cells.

However, selected cells can possess special properties.

Ordinary shape colors remain identifiable.

Special properties should be indicated through small, readable symbols and animation.

Do not make gameplay depend solely on color.

## 5.1 Normal block

The basic block.

It has no special ability.

It is removed through normal line clearing or suitable powers.

It should remain visually attractive through subtle shading, outlines, and limited animation.

## 5.2 Bomb block

**Symbol:** BOMB

**Appearance:** A small pulsing explosive core.

**Activation:** Its cell is removed by an eligible line clear or hit by an effect that triggers explosives.

**Effect:**

Destroy ordinary cells within a 3 × 3 area centered on the bomb's original position.

Armored cells absorb damage instead of automatically disappearing.

Other bombs caught in the explosion can trigger.

Effects must be resolved deterministically.

A single bomb must never activate more than once during the same resolution.

Animation:

1. The bomb flashes.
2. A warning ring expands.
3. A bright explosion occurs.
4. Nearby blocks break into particles.
5. The board briefly shakes.

The explosion animation must not obscure the board for too long.

## 5.3 Fire block

**Symbol:** FLAME

**Appearance:** Animated flame core.

**Activation:** When cleared or triggered by another special effect.

**Effect:**

Emit fire in four orthogonal directions, affecting the nearest adjacent cell in each direction.

Fire removes ordinary cells.

Fire can activate bomb or electrical cells.

Fire can thaw ice.

Fire damages armor.

Fire does not travel indefinitely through the board.

This is intentional.

Infinite fire propagation would be difficult to balance.

Visual effects:

- Small pixel flames.
- Orange and yellow particles.
- A brief heat-distortion illusion.
- Animated ember trails.

## 5.4 Electrical block

**Symbol:** SPARK

**Appearance:** Blue electrical core.

**Activation:** Through line clearing or another power.

**Effect:**

Emit electrical arcs along the block's row and column, affecting up to four cells in each direction.

Electrical arcs stop at explicitly blocking armored cells.

They can activate other electrical cells and explosives.

Ordinary cells directly hit by the arc are destroyed.

Electricity should be especially powerful when electrical cells are deliberately aligned.

Do not allow a single electrical block to clear an unlimited number of cells.

Animation:

- Fast blue-white lightning.
- Brief flickering along affected cells.
- Small sparks at endpoints.
- Distinct electronic sound effects.

## 5.5 Ice block

**Symbol:** ICE

**Appearance:** Pale crystal with visible cracks.

Ice is primarily an environmental hazard.

Ice cells have one armor point.

When hit by an eligible attack or complete-row pressure, their ice armor is removed.

After thawing, the underlying block behaves normally.

When a completed row contains ice:

1. The row enters a short thawing phase.
2. Ice armor is removed.
3. A clearly visible cracking animation plays.
4. After the thawing delay, the engine reevaluates the row.
5. If the row remains complete and contains no remaining armor, it clears normally.

Do not force the player to place another piece into an already full row merely to complete the thaw.

The process must terminate deterministically.

## 5.6 Magnetic block

**Symbol:** MAGNET

**Appearance:** Small red-blue magnet core.

**Effect:**

When activated, attempt to pull a nearby eligible ordinary cell horizontally into an available gap.

Target selection must follow predictable rules:

1. Prefer the closest eligible cell within two columns.
2. Prefer a move that fills a gap.
3. Use a consistent left-before-right tie-break rule.
4. Never overwrite an occupied cell.
5. Never move anchored or armored obstacles illegally.

If no valid movement exists, award a small heat bonus instead.

The effect should create opportunities for unexpected row completions without being completely unpredictable.

## 5.7 Rainbow block

**Symbol:** WILD

**Appearance:** Cycling multicolor crystal.

This is a rare catalyst.

When activated:

- It triggers up to two adjacent special cells.
- Targets are selected according to a fixed priority and position order.
- A targeted cell cannot be triggered twice by the same activation chain.

The rainbow block should encourage players to construct special-cell combinations.

It must not automatically become a screen-clearing superweapon.

## 5.8 Armored block

**Symbol:** ARMOR

**Appearance:** Industrial metal with visible damage stages.

Armored cells have two armor points.

An eligible hit removes one armor point.

A second hit destroys the armor.

Use visibly different sprites for the two armor states.

Completed rows containing armor should resolve through a controlled, finite damage sequence.

Armor must never make a row permanently impossible to clear.

Enemy-generated armor must have clearly communicated counterplay.

## 5.9 Distribution rules

Special cells must be exciting but not so common that positioning becomes irrelevant.

Starting recommendations:

- Normal gameplay: one special-bearing piece approximately every six locked pieces.
- Advanced stages: approximately one every four.
- High-intensity special events: no more than one special cell per newly generated piece unless explicitly scripted.
- Rare catalyst cells should be substantially less common than bombs.

Special-cell distribution must be based on a separate controlled system.

Do not break seven-bag shape fairness merely to insert special cells.

Stage objectives requiring a particular special block must guarantee opportunities to obtain it.

Avoid objectives that depend on getting lucky with a rare spawn.

---

# PART 6 — CHAIN REACTION ENGINE

This system must be designed carefully.

It is one of the central reasons the game will be exciting.

## 6.1 Basic concept

A player completes a row.

The cleared row contains a bomb.

The bomb explodes.

The explosion activates an electrical block.

The electrical block activates another bomb.

The board changes.

New completed rows appear.

The new rows clear.

The player receives a huge combo reward.

The system must support these interactions naturally.

## 6.2 Resolution phases

Implement a deterministic event-based resolution process.

Suggested phases:

1. Lock current piece.
2. Scan for completed rows.
3. Resolve any armor/thaw phases.
4. Collect special-cell triggers.
5. Process special effects in deterministic order.
6. Build the complete set of affected cells.
7. Apply damage and removals.
8. Apply standard row removal.
9. Compact the board according to the applicable gravity rules.
10. Detect new completed rows.
11. Repeat until the board is stable.
12. Award points and heat.
13. Finish animations.
14. Resume normal piece spawning.

Take care to distinguish full-row removal from ordinary partial destruction.

A normal line clear should preserve familiar falling-block behavior.

After special effects have removed isolated cells, column-based settling may occur to enable genuine cascades.

This distinction must be explicitly implemented and tested.

Do not accidentally compact every column after every conventional line clear.

## 6.3 Chain reaction safety

Each resolution must terminate.

Required safeguards:

- Track activated cells.
- Prevent repeated activation of the same cell in one chain.
- Use bounded event queues.
- Reject invalid coordinates.
- Handle simultaneous cell removal safely.
- Avoid iterator invalidation.
- Avoid recursive stack overflow.
- Handle empty-board and near-full-board states.
- Resolve overlapping explosions consistently.
- Never create duplicate score awards for the same event.

If necessary, use an explicit resolution state machine.

Do not use uncontrolled recursion.

## 6.4 Combo presentation

Add increasingly dramatic feedback:

| Chain Tier | Display         |
| ---------- | --------------- |
| 2          | NICE!           |
| 3          | HOT COMBO!      |
| 4          | SPICY!          |
| 5          | SUPER CHAIN!    |
| 6          | MAYHEM!         |
| 7          | INSANE!         |
| 8+         | TOTAL MELTDOWN! |

Animations should become progressively more exciting.

Examples:

- Larger text.
- Faster flashes.
- More elaborate particles.
- More dramatic sound effects.
- Brief screen shake.
- A short music accent.

However:

**Never make the board impossible to read during active gameplay.**

---

# PART 7 — SCORING AND REWARD SYSTEM

The scoring system must reward skill rather than uncontrolled randomness.

## 7.1 Line-clearing scores

Suggested starting scores:

| Lines Cleared Together | Base Points |
| ---------------------- | ----------: |
| 1                      |         100 |
| 2                      |         300 |
| 3                      |         500 |
| 4                      |         800 |

Multiply the base score by:

`level + 1`

Use a consistent level convention beginning at level zero internally or adjust the multiplier if displayed levels begin at one.

Avoid off-by-one differences between internal and displayed levels.

## 7.2 Drop bonuses

- Soft drop: 1 point per manually advanced cell.
- Hard drop: 2 points per descended cell.

Do not award drop points repeatedly for the same movement.

## 7.3 Consecutive-placement combos

Consecutive locked pieces that each cause at least one row clear increase the placement combo counter.

A piece that produces no cleared row resets that counter.

Award:

`50 × combo index × level multiplier`

Use a bounded multiplier where necessary.

## 7.4 Chain bonuses

Special-effect cascades use a separate chain counter.

Chain bonuses should increase with each distinct resolution wave.

Prevent excessive score inflation from repeatedly processing the same cells.

## 7.5 Back-to-back bonus

Reward consecutive difficult clears.

For example:

- Four-line clears.
- Qualified advanced spin clears, if implemented and properly detected.

A back-to-back clear receives a 1.5× multiplier on the relevant line-clear base score.

Do not automatically give that multiplier to all incidental destruction.

## 7.6 Special-cell destruction

Award a small score bonus per cell destroyed by a special effect.

Suggested value:

25 points per eligible cell.

Cap the bonus awarded by a single effect to prevent runaway multiplication.

---

# PART 8 — THE HEAT METER

## 8.1 Purpose

The Heat Meter is the game's central strategic resource.

Display a large, attractive meter labeled:

**HEAT**

Range:

0–100.

The player accumulates heat by performing skillful actions.

Heat can be spent on special abilities.

Alternatively, reaching maximum heat automatically activates an extremely dangerous and rewarding temporary state.

The choice between spending heat and building toward maximum intensity must be meaningful.

## 8.2 Heat generation

Initial values:

| Action                       | Heat Gained |
| ---------------------------- | ----------: |
| Single line                  |          13 |
| Double line                  |          30 |
| Triple line                  |          45 |
| Four-line clear              |          65 |
| Additional chain wave        |          +5 |
| Difficult back-to-back clear |         +10 |

Apply a reasonable cap to additional heat generated by a single chain event.

Players should not reach maximum heat every few seconds.

A typical beginner should experience Overheat occasionally.

A highly skilled player should be able to trigger it more frequently through deliberate play.

## 8.3 Heat spending

The player can spend heat to activate equipped abilities.

Abilities must have:

- A defined cost.
- A clear gameplay effect.
- A cooldown where necessary.
- An audiovisual activation effect.
- A visible disabled state when unavailable.

The UI must clearly show whether the selected ability can be used.

---

# PART 9 — OVERHEAT MODE

## 9.1 Activation

When the meter reaches 100, automatically activate:

# OVERHEAT!

Use a dramatic but short transition.

Possible sequence:

1. The meter flashes.
2. A warning sound plays.
3. The screen briefly darkens.
4. A red-orange explosion radiates behind the board.
5. The word OVERHEAT appears.
6. The music shifts into a more intense arrangement.
7. The game resumes immediately.

Keep the noninteractive transition brief.

## 9.2 Duration

Overheat lasts:

**12 seconds of active simulation time.**

The timer must behave consistently during line-clear animations, pauses, boss transitions, and time-freeze effects.

Menus and pause screens must not consume the timer.

## 9.3 Effects

During Overheat:

- Falling speed increases moderately.
- Line-clearing score doubles.
- Boss damage receives a limited bonus.
- Special-effect animations become more intense.
- The Heat Meter changes color.
- Background animation accelerates.
- A special music arrangement plays.

The speed increase must not make the game mechanically unfair.

## 9.4 Success condition

Clear at least two complete rows during Overheat.

These may be cleared through one or several successful placements.

On success:

- Award a score bonus.
- Play a victory accent.
- Avoid the failure penalty.
- Display a short celebratory animation.

## 9.5 Failure condition

If Overheat ends without clearing two rows:

Add garbage rows to the bottom of the board.

Recommended:

- Beginner difficulty: one row.
- Standard difficulty: two rows.
- Expert difficulty: two rows, potentially with a more restrictive hole pattern.

Garbage rows must always have at least one opening.

The incoming garbage pattern must be visible in advance.

Do not generate unavoidable instant deaths.

## 9.6 State management

When Overheat begins:

- Consume the accumulated 100 heat.
- Start the Overheat timer.
- Continue allowing heat accumulation, but limit the amount preserved during the temporary state.
- Prevent immediate repeated Overheat activation before the current state finishes.

Do not allow infinite Overheat chaining through one enormous special-cell cascade.

The exact implementation must be documented and tested.

---

# PART 10 — ACTIVE SPECIAL ABILITIES

The player can equip different abilities.

The game must support strategic loadouts rather than giving every power unlimited availability.

## 10.1 LASER BEAM

**Cost:** 40 heat.

**Cooldown:** Four locked pieces.

Effect:

The player fires a vertical energy beam through a selected column.

The beam destroys eligible cells.

Armored cells take damage.

The laser may trigger bombs caught in its path.

The active piece should remain temporarily controlled while column selection occurs.

Selection must have a short timeout and clear visual targeting.

Animation:

- Targeting line.
- Charging effect.
- Bright vertical beam.
- Block disintegration.
- Screen shake.
- Distinct laser sound.

## 10.2 TIME FREEZE

**Cost:** 35 heat.

**Cooldown:** Five locked pieces.

Effect:

Pause automatic falling for approximately three seconds.

The player may still move, rotate, or manually drop the piece.

Relevant boss attack countdowns should also pause during the freeze.

Animation:

- Blue tint.
- Clock-like particles.
- Frosted border.
- Brief crystalline sound.

Do not freeze the entire input system.

## 10.3 GRAVITY PULSE

**Cost:** 45 heat.

**Cooldown:** Five locked pieces.

Effect:

Change the settling direction of eligible loose blocks during one controlled board-resolution event.

Possible directions:

- Left.
- Right.
- Up.
- Down.

The falling tetromino should continue following the normal downward movement rules unless an explicitly designed stage says otherwise.

Clearly distinguish:

**Active-piece falling direction**

from:

**Settled-block compaction direction.**

This avoids ambiguous physics.

Gravity manipulation must not place cells outside the board or overwrite occupied cells.

Visual effects:

- Directional arrows.
- Moving particles.
- Board distortion.
- Powerful low-frequency sound.

## 10.4 SHAPE SHIFT

**Cost:** 25 heat.

**Cooldown:** Three locked pieces.

Effect:

Allow the current piece to change into one of several available geometric shapes.

The player must see the result before confirming.

Only legal transformations should be accepted.

The selected transformation must not overlap existing cells.

This is a precision tool.

It should reward planning rather than replace the entire falling-block mechanic.

## 10.5 NUCLEAR BURST

**Cost:** 80 heat.

**Cooldown:** Eight locked pieces.

Effect:

Destroy a substantial localized area of the board.

Suggested starting area:

5 × 5 cells.

Armored cells take appropriate damage.

The power should be exceptionally satisfying, but expensive enough to require deliberate resource planning.

Animation:

- Bright charge.
- Expanding energy sphere.
- Multiple particle layers.
- Large explosion.
- Brief screen shake.
- A dramatic sound effect.

Do not use a full-screen white flash that persists long enough to interfere with play.

## 10.6 SHIELD MATRIX

**Cost:** 30 heat.

**Cooldown:** Five locked pieces.

Effect:

Negate the next eligible hostile garbage injection or environmental attack within a limited duration.

The shield must not prevent the player from losing due to ordinary top-out.

Display a small shield indicator while active.

## 10.7 CHAIN SPARK

**Cost:** 45 heat.

**Cooldown:** Four locked pieces.

Effect:

Temporarily charge one selected eligible settled cell.

When a connected line-clear or special event reaches it, it emits a small electrical pulse.

The power encourages players to set up controlled chain reactions.

Do not allow unlimited repeated charging of one cell.

## 10.8 EMERGENCY VENT

**Cost:** 50 heat.

**Cooldown:** Six locked pieces.

Effect:

Remove a small number of occupied cells near the top of the board according to a deterministic, clearly communicated target rule.

This is a defensive recovery tool.

It should be less efficient for scoring than aggressive abilities.

## 10.9 Ability balance

Each ability must have a reason to exist.

Laser should offer precise removal.

Time Freeze should provide control.

Gravity Pulse should create spatial opportunities.

Shape Shift should improve placement decisions.

Nuclear Burst should offer expensive emergency destruction.

Shield Matrix should counter predictable attacks.

Chain Spark should reward advanced setup.

Emergency Vent should prevent difficult situations from becoming hopeless.

No single ability should dominate every mode.

---

# PART 11 — CONTROLS

Use the physical GBA controls intuitively.

## 11.1 Default controls

| Button      | Action                    |
| ----------- | ------------------------- |
| D-Pad Left  | Move left                 |
| D-Pad Right | Move right                |
| D-Pad Down  | Soft drop                 |
| D-Pad Up    | Hard drop                 |
| A           | Rotate clockwise          |
| B           | Rotate counterclockwise   |
| L           | Hold piece                |
| R           | Activate equipped ability |
| SELECT      | Cycle equipped abilities  |
| START       | Pause                     |

This layout must be implemented consistently.

If ability targeting is required, temporarily enter a targeting state.

While targeting:

- Left/right changes target.
- A confirms.
- B cancels.

Do not allow ordinary piece movement during targeting unless the mechanic explicitly requires it.

## 11.2 Dual-board control changes

In dual-board mode:

- SELECT switches the active board.
- R activates the currently equipped ability.
- Only one active ability is available at a time.
- The selected ability is configured between stages or at an appropriate checkpoint.

This avoids overloading the limited GBA buttons.

## 11.3 Input quality

Inputs must feel immediate.

Support button holding correctly.

Avoid accidental repeated menu selections.

Avoid movement glitches after pause.

Ensure a button held during a cinematic cannot accidentally confirm several menus.

Implement separate input handling for:

- Gameplay.
- Menus.
- Targeting.
- Cutscenes.
- Pause.
- Upgrade selection.
- Game-over screens.

---

# PART 12 — TWO-BOARD GAMEPLAY

This is one of the game's most ambitious features.

Create a dedicated mode called:

# DUAL BOARD RELAY

## 12.1 Concept

The player manages two separate puzzle boards.

Both boards share strategic resources.

Clearing rows on one board can influence the other.

The goal is to maintain both boards while using coordinated actions to create powerful combinations.

## 12.2 Board dimensions

Recommended:

Two boards, each:

- 8 columns.
- 16 visible rows.
- 2 hidden spawn rows.

Use smaller tiles to fit both boards on the GBA screen.

The game must remain readable.

Do not shrink symbols and text into illegibility.

## 12.3 Active board

Only one board contains the actively falling piece at a time.

The inactive board pauses ordinary falling-piece movement.

This is essential for fair gameplay on a system with limited controls.

The player can switch boards using SELECT.

Additionally, the game forces a board switch after four locked pieces on the current board.

Display a countdown showing how many locks remain before the next forced switch.

Do not force a switch during unresolved animations.

## 12.4 Shared resources

Both boards share:

- One Heat Meter.
- One score.
- One ability resource.
- One overall failure state.

Each board maintains:

- Its own locked-cell grid.
- Its own incoming garbage.
- Its own current-piece state.
- Its own local combo information.

## 12.5 Cross-board interactions

Clearing rows on one board generates energy for the other.

Examples:

- Clearing two lines on Board A grants a small heat bonus and a temporary defensive effect on Board B.
- Triggering an electrical chain on Board A charges a compatible electrical cell on Board B.
- Clearing four lines on Board A may remove a limited amount of pending garbage from Board B.

All interactions must follow exact rules.

Avoid random cross-board behavior.

## 12.6 Cross-board combo

A special combo occurs when the player clears lines on both boards within a defined number of locked pieces.

Call it:

**CROSS OVER!**

Suggested requirement:

Clear at least one row on each board within four total locked pieces.

Reward:

- Additional score.
- Heat.
- A brief visual connection between the boards.

Do not create an exploit where players repeatedly switch boards without placing pieces to farm bonuses.

## 12.7 Failure

If either board reaches a genuine top-out state, the run ends unless a clearly defined rescue mechanic applies.

Do not hide imminent danger on the inactive board.

Display warning indicators for both boards.

## 12.8 Presentation

Add an animated energy bridge between the two boards.

When a cross-board combo occurs:

- Both boards flash.
- Energy travels between them.
- A special sound plays.
- The score multiplier briefly appears.

This mode must feel fundamentally different from standard gameplay.

---

# PART 13 — ADVENTURE MODE

Create a full single-player campaign.

## 13.1 Overall structure

Six worlds.

Each world contains five stages.

Total:

**30 playable campaign stages.**

Each world contains:

- Four gameplay stages.
- One boss stage.

Stages should include distinct objectives, hazards, visual environments, and increasingly complex mechanics.

World 1 must receive especially high production polish.

However, the other five worlds must also be playable and meaningfully differentiated.

Do not represent them as empty menu entries.

## 13.2 World map

Create a colorful pixel-art world-selection screen.

Each world should have:

- A unique icon.
- A name.
- An animated miniature environment.
- A completion indicator.
- A progression path.
- A locked/unlocked state.
- A boss icon.

Show the player's overall progress.

Use the GBA screen intelligently.

The map should resemble an authentic retro adventure-game interface.

---

# PART 14 — WORLD 1: CLASSIC CHAOS

## Theme

A gigantic malfunctioning arcade factory.

Visual identity:

- Metallic backgrounds.
- Conveyor belts.
- Animated warning lights.
- Mechanical gears.
- Neon arcade signs.
- Small maintenance robots.
- Industrial machinery.
- Colorful control panels.

Color palette:

- Dark blue.
- Steel gray.
- Electric yellow.
- Bright orange.
- Red warning lights.

Music:

Energetic industrial chiptune with a memorable melody.

The first world must teach the player the game without becoming boring.

## Stage 1-1 — FIRST SPARK

Objective:

Clear eight lines.

Mechanics:

- Ordinary falling pieces.
- Basic rotation.
- Hard drop.
- Hold piece.
- Next-piece preview.
- Ghost piece.

No hostile hazards.

Introduce controls through small contextual overlays.

Do not interrupt the player excessively.

Completion should feel satisfying.

Play a short victory animation.

## Stage 1-2 — HOT WIRES

Objective:

Clear twelve lines.

Introduce:

- Heat generation.
- Heat Meter.
- Laser ability.
- Basic ability spending.

Guarantee the player gets at least one meaningful opportunity to use the laser.

Explain heat using visual feedback.

## Stage 1-3 — BOOM FACTORY

Objective:

Clear twelve lines and trigger at least three bombs.

Introduce bomb blocks.

Script appropriate bomb-bearing pieces into the special-cell distribution.

Let the player discover how explosions affect the board.

Add background explosions synchronized with successful bomb chains.

The stage should encourage setup and timing.

## Stage 1-4 — CHAIN REACTION

Objective:

Clear fourteen lines and complete at least one chain reaction with two distinct waves.

Introduce:

- Fire.
- Electricity.
- Chain bonuses.

Ensure the stage reliably offers the required components.

Create an exciting audiovisual celebration for the first major chain.

## Stage 1-5 — BOSS: THE BLOCK MONSTER

The first major boss.

Appearance:

A gigantic mechanical creature made of shifting puzzle blocks.

It occupies the right side of the screen.

Its face is composed of glowing geometric shapes.

It has huge animated eyes, moving mechanical arms, and a mouth that opens during attacks.

The boss must have character and personality.

### Boss behavior

The player attacks by clearing rows.

The boss responds with predictable patterns.

Phase 1:

- Basic attacks.
- Occasional garbage-row warnings.
- Slow telegraphed mechanical punches.

Phase 2:

- Faster attacks.
- More aggressive animations.
- Small armored-cell injections.

Phase 3:

- Angry transformation.
- Glowing eyes.
- Faster music.
- More dramatic attacks.

### Boss attack telegraphing

Before every major attack:

1. Show a warning.
2. Animate the relevant body part.
3. Play a charging sound.
4. Give the player a fair response window.
5. Execute the attack.

Do not introduce unavoidable surprise attacks.

### Victory

At zero HP:

- The monster begins malfunctioning.
- Sparks erupt from its body.
- Several smaller explosions occur.
- The screen shakes.
- A large final explosion plays.
- The boss collapses.
- A victory theme starts.
- The next world unlocks.

This sequence must feel like a genuine arcade boss defeat.

---

# PART 15 — WORLD 2: FROZEN FACTORY

## Theme

A frozen industrial complex.

Visual identity:

- Snowstorms.
- Frozen machinery.
- Icicles.
- Crystal formations.
- Pale-blue lighting.
- Cold metallic structures.

Primary mechanic:

Ice and delayed line clearing.

## Stage 2-1 — COLD START

Introduce frozen cells.

Objective:

Clear ten lines while managing ice.

## Stage 2-2 — CRYSTAL PRESSURE

Introduce rows containing multiple frozen cells.

Objective:

Clear fourteen lines.

## Stage 2-3 — DEEP FREEZE

Introduce periodic freezing events.

The next affected cells must be telegraphed.

Objective:

Survive a defined number of piece placements and clear at least twelve lines.

## Stage 2-4 — ZERO POINT

Combine ice with electricity.

Teach how electrical abilities interact with frozen structures.

Objective:

Clear sixteen lines and complete two qualified special chains.

## Stage 2-5 — BOSS: THE CRYO TITAN

A gigantic frozen robot.

Attacks:

- Ice injections.
- Temporary slowdown effects.
- Frozen walls.
- Predictable crystalline projectiles.

Visual animation:

- Frost breath.
- Cracking armor.
- Shattering crystals.
- Blue-white explosions.

The boss should punish careless board management but allow deliberate counterplay.

---

# PART 16 — WORLD 3: VOLCANO MADNESS

## Theme

A volcanic industrial wasteland.

Visual identity:

- Lava rivers.
- Erupting mountains.
- Smoke.
- Molten rocks.
- Red-orange lighting.
- Heat distortion.
- Burning machinery.

Primary mechanic:

Fire, Overheat, and increased speed.

## Stage 3-1 — IGNITION

Introduce more frequent fire cells.

Objective:

Clear twelve lines.

## Stage 3-2 — RISING HEAT

Introduce a temporary lava-pressure meter.

If the meter reaches its limit, the player receives a telegraphed hazard.

Objective:

Clear fourteen lines.

## Stage 3-3 — INFERNO COMBO

Encourage fire and bomb combinations.

Objective:

Produce at least two multi-wave chains and clear sixteen lines.

## Stage 3-4 — MELTDOWN

Introduce a faster stage with more frequent Overheat opportunities.

Objective:

Clear eighteen lines and complete at least one successful Overheat.

## Stage 3-5 — BOSS: MAGMA HYDRA

A giant lava creature with multiple heads.

Each head has unique attack animations.

Phases:

1. Fire attacks.
2. Falling molten hazards.
3. Multiple-head attacks with longer telegraphs.
4. Final enraged phase.

Destroying a head should trigger a dramatic animation.

The final head must have a memorable death sequence.

---

# PART 17 — WORLD 4: ELECTRIC CITY

## Theme

A futuristic neon metropolis.

Visual identity:

- Dark skyscrapers.
- Animated signs.
- Electrical storms.
- Flying vehicles.
- Neon purple.
- Bright cyan.
- Lightning.

Primary mechanic:

Electrical chains and cross-board interactions.

## Stage 4-1 — POWER GRID

Introduce complex electrical-cell arrangements.

Objective:

Clear fourteen lines.

## Stage 4-2 — MAGNETIC STORM

Introduce magnetic blocks.

Objective:

Clear sixteen lines and successfully activate several magnetic effects.

## Stage 4-3 — SPLIT CIRCUIT

Introduce Dual Board Relay.

Objective:

Clear a combined total of sixteen lines.

Provide a concise tutorial on switching boards.

## Stage 4-4 — CROSS OVERLOAD

Require coordinated gameplay across both boards.

Objective:

Clear twenty combined lines and trigger at least two cross-board combos.

## Stage 4-5 — BOSS: THE ELECTRIC EMPEROR

A giant cybernetic entity controlling a power network.

The battle uses Dual Board Relay.

Attacks:

- Electrical surges.
- Temporarily disabled special cells.
- Telegraphed garbage attacks.
- Cross-board interference.

Visual effects:

- Lightning between the boards.
- Animated cables.
- Electrical explosions.
- Glowing circuitry.

The battle should reward alternating between boards strategically.

---

# PART 18 — WORLD 5: GRAVITY DIMENSION

## Theme

A surreal region where physical laws are unstable.

Visual identity:

- Floating islands.
- Rotating structures.
- Impossible machinery.
- Cosmic backgrounds.
- Distorted geometric environments.
- Purple and green energy.
- Moving stars.

Primary mechanic:

Directional settling and gravity manipulation.

## Stage 5-1 — SIDEWAYS

Introduce sideways settling after designated special-effect events.

Objective:

Clear fourteen lines.

## Stage 5-2 — REVERSE FIELD

Introduce upward settling during clearly announced events.

Objective:

Clear sixteen lines.

## Stage 5-3 — ROTATION ENGINE

Introduce alternating settling directions.

Changes must be telegraphed.

Objective:

Clear eighteen lines.

## Stage 5-4 — FRACTURED REALITY

Combine gravity changes, electrical cells, and selected dual-board mechanics.

Objective:

Clear twenty total lines.

## Stage 5-5 — BOSS: THE GRAVITY KING

A floating cosmic ruler surrounded by rotating geometric structures.

The boss changes the gravity behavior of settled blocks.

Important:

The active falling piece continues obeying normal movement controls unless a specific, fully implemented rule says otherwise.

The boss must not randomly move occupied cells into illegal positions.

Phases:

1. Horizontal gravity events.
2. Upward gravity events.
3. Alternating gravity.
4. Final chaotic sequence with visible warnings.

Animations:

- Rotating cosmic rings.
- Distorted backgrounds.
- Spinning blocks.
- Energy waves.
- Dramatic camera effects.

---

# PART 19 — WORLD 6: THE FINAL MELTDOWN

## Theme

A collapsing dimension combining visual and mechanical elements from all previous worlds.

Visual identity:

- Broken arcade machinery.
- Floating volcanic fragments.
- Frozen structures.
- Electrical storms.
- Cosmic portals.
- Rapidly changing backgrounds.

This world should feel like the climax of the adventure.

## Stage 6-1 — SYSTEM FAILURE

Combine ice, fire, electricity, and bombs.

Objective:

Clear twenty lines.

## Stage 6-2 — TOTAL INSTABILITY

Combine gravity effects and environmental hazards.

Objective:

Clear twenty-two lines.

## Stage 6-3 — DOUBLE TROUBLE

Advanced Dual Board Relay challenge.

Objective:

Clear twenty-four combined lines.

## Stage 6-4 — POINT OF NO RETURN

A long, difficult survival challenge.

Objective:

Clear thirty lines while handling carefully telegraphed mixed hazards.

Provide meaningful recovery opportunities.

Avoid arbitrary unavoidable difficulty spikes.

## Stage 6-5 — FINAL BOSS: THE CORE OF MAYHEM

A gigantic living machine that controls the entire puzzle universe.

The final boss should be visually spectacular.

Design it with multiple animated components rather than one static image.

Possible appearance:

- Gigantic central eye.
- Mechanical wings.
- Rotating geometric rings.
- Electrical tentacles.
- Volcanic core.
- Crystalline armor.

### Phase 1

Conventional attacks.

The boss tests the player's fundamentals.

### Phase 2

Elemental powers.

Ice, fire, and electrical hazards.

### Phase 3

Gravity manipulation.

The board's settling behavior changes periodically.

### Phase 4

Dual-board combat.

The player must coordinate the two boards.

### Phase 5 — FINAL MELTDOWN

The boss reaches critical HP.

The music changes to its most intense arrangement.

Backgrounds animate dramatically.

The boss launches complex combinations of previously learned attack patterns.

The final phase must test mastery without relying on unfair randomness.

### Final victory sequence

The boss begins destabilizing.

Its body cracks into glowing fragments.

The screen flashes briefly.

Energy escapes from the core.

Mechanical parts explode sequentially.

The central eye closes.

The entire machine collapses.

A final shockwave fills the background.

The music stops for a moment.

Then a triumphant ending theme begins.

Display a final victory illustration and ending sequence.

Show the player's final score and campaign statistics.

Unlock additional content.

---

# PART 20 — BOSS COMBAT SYSTEM

Implement a shared, data-driven boss framework.

## 20.1 Boss health

Bosses have an explicit HP value.

Use appropriately scaled values for each world.

Suggested starting HP values:

| Boss             |  HP |
| ---------------- | --: |
| Block Monster    | 100 |
| Cryo Titan       | 140 |
| Magma Hydra      | 180 |
| Electric Emperor | 220 |
| Gravity King     | 260 |
| Core of Mayhem   | 350 |

These values must be evaluated against realistic player damage output.

Do not treat them as unquestionable final values.

## 20.2 Player damage

Initial recommended damage:

| Action          | Boss Damage |
| --------------- | ----------: |
| Single line     |           5 |
| Double line     |          12 |
| Triple line     |          20 |
| Four-line clear |          35 |

Special chains add damage according to a bounded rule.

Offensive abilities may add moderate direct damage.

Do not allow a single lucky bomb chain to instantly defeat every boss.

## 20.3 Phase transitions

Bosses should change behavior at defined HP thresholds.

For example:

- 70% HP.
- 40% HP.
- 15% HP.

Phase transitions should be visually communicated.

Do not change attacks silently.

## 20.4 Boss attack timing

Use deterministic attack patterns with controlled variation.

Prefer scheduling attacks by:

- Number of locked pieces.
- Defined encounter phases.
- Clearly presented countdowns.

Avoid relying entirely on wall-clock timers.

A player should be able to learn an attack pattern.

## 20.5 Enemy fairness

Bosses must never:

- Insert blocks directly into an occupied cell without a defined resolution.
- Trigger unavoidable top-out from an invisible hazard.
- Attack during noninteractive cutscenes.
- Ignore a legitimately active defensive shield.
- Produce untelegraphed lethal attacks.
- Continue consuming attack timers while the game is paused.

The game must be challenging because of meaningful decisions, not because of unfair surprises.

---

# PART 21 — ROGUELITE MODE

Create an independent replayable mode called:

# CHAOS RUN

This mode should offer meaningful strategic variation across multiple sessions.

## 21.1 Run structure

Each run contains three acts.

Each act contains:

- Three encounters.
- One boss.

Total:

Twelve encounters per successful run.

Encounters should vary through combinations of:

- Stage objectives.
- Environmental rules.
- Special-cell distribution.
- Enemy interference.
- Available upgrades.
- Difficulty escalation.

Keep a successful run reasonably paced.

Avoid excessively long sessions that are unsuitable for a handheld console.

## 21.2 Progression

After completing an encounter, present three upgrade choices.

The player selects one.

Use a deterministic seeded generator.

The choices must come from a controlled upgrade pool.

Do not offer upgrades that cannot function with the player's current configuration.

## 21.3 Upgrade categories

Create at least twenty distinct upgrades.

Suggested pool:

### EXPLOSIVE MASTER

Increase bomb effectiveness moderately.

### FIRE STARTER

Fire can affect one additional eligible cell under defined conditions.

### CHAIN KING

Gain additional score from multi-wave chains.

### HEAT ENGINE

Increase heat gain by a limited percentage.

### SECOND CHANCE

Prevent one otherwise fatal top-out under explicitly defined conditions.

### LASER FOCUS

Reduce laser cost slightly.

### CRYSTAL BREAKER

Increase damage against ice and armor.

### ELECTRIC SURGE

Increase the number of electrical cells affected.

### MAGNETIC FIELD

Improve magnetic targeting range.

### TIME ENGINEER

Increase Time Freeze duration moderately.

### COMBO BATTERY

Gain a small heat bonus after qualified consecutive clears.

### OVERHEAT EXPERT

Increase the score bonus during successful Overheat.

### SAFE VENT

Improve emergency recovery efficiency.

### IRON DEFENSE

Reduce the severity of one incoming garbage attack.

### RAINBOW MASTER

Improve the reliability of rainbow-cell activation.

### PRECISION ENGINE

Increase rewards for skilled multi-line clears.

### FAST CHARGER

Slightly reduce selected ability cooldowns.

### REACTIVE ARMOR

Automatically absorb one limited environmental effect per encounter.

### DOUBLE SPARK

A qualified electrical chain generates an additional limited spark.

### FINAL GAMBIT

Gain a strong scoring bonus while the board is dangerously high.

This upgrade should provide a real risk/reward tradeoff.

## 21.4 Rarity system

Use three rarity levels:

- Common.
- Rare.
- Legendary.

Legendary upgrades should be exciting and visually distinctive.

However, rarity must not automatically imply an upgrade is overpowered.

## 21.5 Synergies

Create deliberate synergies between upgrades.

Examples:

Explosive Master + Chain King:

A bomb-focused scoring strategy.

Heat Engine + Overheat Expert:

A high-risk Overheat strategy.

Electric Surge + Double Spark:

An electrical-chain strategy.

Time Engineer + Precision Engine:

A controlled precision strategy.

Iron Defense + Safe Vent:

A survival strategy.

The systems must remain balanced when combined.

## 21.6 Upgrade stacking limits

Implement explicit maximum values.

Examples:

- Heat generation bonuses should have an overall cap.
- Ability costs cannot fall below a sensible minimum.
- Cooldowns cannot reach zero.
- Offensive area effects cannot expand infinitely.
- Extra chain triggers must remain bounded.
- Defensive upgrades must not provide permanent invulnerability.

Document the exact limits.

## 21.7 Run conclusion

On victory:

- Display a run summary.
- Show the selected upgrade build.
- Show score and statistics.
- Award achievements.
- Save the result.

On defeat:

- Display the reached act.
- Show score and completed encounters.
- Save relevant statistics.
- Offer a new run.

Do not create permanent progression that makes previously difficult runs trivial.

Persistent unlocks should primarily increase variety rather than grant unlimited power.

---

# PART 22 — ADDITIONAL MODES

Include the following modes.

## 22.1 CLASSIC MODE

A traditional falling-block experience.

Rules:

- Seven standard shapes.
- Seven-bag randomization.
- Standard line clearing.
- Hold.
- Next-piece preview.
- Ghost piece.
- Increasing speed.
- Score.
- High-score tracking.

Disable powers, boss attacks, special-cell explosions, and arbitrary environmental hazards.

This mode should stand on its own as a polished game.

## 22.2 CHAOS ARCADE

An endless score-attack mode featuring:

- Special cells.
- Heat.
- Overheat.
- Equipped abilities.
- Increasing difficulty.
- Controlled environmental events.

The goal is survival and high scores.

## 22.3 ADVENTURE MODE

The thirty-stage campaign.

## 22.4 CHAOS RUN

The roguelite mode.

## 22.5 DUAL BOARD RELAY

A dedicated endless or objective-based two-board mode.

## 22.6 BOSS RUSH

Unlock after suitable campaign progress.

Fight previously defeated bosses in sequence.

Allow a limited number of recovery opportunities.

Display total completion time and score.

Do not provide a boss-rush menu if the underlying mode does not actually function.

---

# PART 23 — VISUAL DIRECTION

This game must look exceptional for a Game Boy Advance title.

Do not settle for plain colored rectangles on a black background.

The graphics should communicate an unmistakable artistic identity.

## 23.1 Art style

Use original pixel art inspired by the expressive arcade aesthetic of the late 1990s and early 2000s.

Desired qualities:

- Detailed pixel-art environments.
- Strong character silhouettes.
- Saturated but carefully coordinated colors.
- Expressive enemy animation.
- Impressive explosions.
- Mechanical animation.
- Stylish UI.
- Dynamic transitions.
- Layered backgrounds.
- Strong visual feedback.

Create original assets.

Do not reproduce existing commercial sprite sheets.

## 23.2 Block appearance

Blocks should have depth and personality.

Use:

- Highlighted edges.
- Dark outlines.
- Inner shading.
- Distinct colors.
- Small animated cores.
- Different states for armored cells.
- Readable special symbols.

Avoid excessive texture that makes the board confusing.

At 8 × 8 pixels, every decorative pixel matters.

## 23.3 Backgrounds

Each world must have a different background.

Implement subtle animation.

Examples:

- Factory gears rotating.
- Snow drifting.
- Lava bubbling.
- Neon signs flickering.
- Stars moving.
- Electrical storms.
- Cosmic structures rotating.

Use tilemaps, hardware scrolling, palette effects, or other appropriate techniques.

Do not redraw the entire screen unnecessarily.

## 23.4 Parallax

Use parallax scrolling where the hardware and screen composition permit.

Different background layers should move at different speeds.

The effect should enhance depth without distracting from gameplay.

## 23.5 Boss sprites

Bosses should appear large and dramatic.

Use combinations of hardware sprites and background tiles where appropriate.

Large boss designs may consist of multiple animated components.

Avoid exceeding hardware sprite limitations.

Boss animation should include:

- Idle.
- Attack preparation.
- Attack execution.
- Damage reaction.
- Phase transition.
- Defeat.

Each boss should have a distinct visual personality.

---

# PART 24 — ANIMATION QUALITY

Animation is a major priority.

Create a reusable animation system rather than hardcoding every visual sequence independently.

## 24.1 Piece movement

Movement should feel crisp and precise.

Avoid sluggish interpolation that reduces control responsiveness.

Optional subtle effects:

- Tiny landing bounce.
- Short lock flash.
- Small impact particles.
- Landing dust.

## 24.2 Rotation

Provide immediate logical rotation.

Any visual rotation flourish must not delay actual collision updates.

## 24.3 Hard drop

When a piece hard-drops:

- Draw a brief falling trail.
- Trigger a compact landing impact.
- Play an appropriate sound.
- Optionally shake the board slightly.

## 24.4 Line clearing

A line clear should be extremely satisfying.

Suggested sequence:

1. Completed row flashes.
2. Bright energy travels across the row.
3. Cells dissolve or burst.
4. A short score popup appears.
5. The board updates.

Different clearing strengths should have different intensities.

A four-line clear should feel substantially more dramatic than a single-line clear.

## 24.5 Special explosions

Use layered pixel effects:

- Core flash.
- Expanding ring.
- Debris.
- Sparks.
- Smoke.
- Brief lighting change.

Implement effects efficiently.

A large effect should not require hundreds of hardware sprites.

## 24.6 Screen shake

Use several predefined shake profiles:

- Small.
- Medium.
- Large.
- Boss impact.
- Final explosion.

Screen shake must be short.

The board must return to its correct position.

Disable or reduce screen shake in accessibility settings.

## 24.7 Hitstop

Use extremely short intentional pauses for significant events.

Examples:

- Large explosions.
- Boss damage.
- Major chains.
- Finishing attacks.

Do not interrupt input for excessive periods.

Do not allow hitstop to corrupt timers or gameplay state.

## 24.8 Transitions

Create polished transitions between:

- Title screen.
- Main menu.
- World map.
- Gameplay.
- Stage completion.
- Boss encounters.
- Victory.
- Game over.

Use effects such as:

- Pixel wipes.
- Palette fades.
- Sliding panels.
- Mechanical shutters.
- Energy flashes.

Avoid slow transitions that become irritating after repeated play.

## 24.9 Particle system

Implement a small reusable particle engine.

Support:

- Sparks.
- Smoke.
- Fire.
- Snow.
- Electricity.
- Crystals.
- Explosive debris.
- Score particles.
- Cosmic dust.

Use bounded pools.

Prefer tile-based or pooled visual techniques where appropriate.

Do not allocate new particle objects on the heap during gameplay.

---

# PART 25 — AUDIO AND ORIGINAL MUSIC

The game must have a genuine musical identity.

Audio is not optional decoration.

It should make the game feel alive.

## 25.1 Musical direction

Combine:

- Energetic chiptune.
- Arcade melodies.
- Rock-inspired rhythms.
- Electronic basslines.
- Industrial percussion.
- Dramatic boss music.

All music should be original.

Do not copy melodies or samples from existing games.

## 25.2 Music requirements

Compose or program original music for:

1. Title screen.
2. Main menu and world map.
3. World 1.
4. World 2.
5. World 3.
6. World 4.
7. World 5.
8. World 6.
9. Boss battles.
10. Final boss.
11. Victory.
12. Game over.
13. Ending.

Where appropriate, reuse original melodic themes through different arrangements.

Do not simply replay the exact same short loop in every world.

Use GBA-compatible tracker music or another practical playback method.

If using Butano, prefer its supported audio asset formats and actual documented APIs.

## 25.3 Dynamic music

The music should react to gameplay intensity.

Possible states:

- Normal gameplay.
- High Heat.
- Overheat.
- Boss phase transition.
- Critical danger.
- Victory.

The implementation must remain compatible with GBA audio limitations.

Avoid requiring multiple full simultaneous music streams if the hardware or engine cannot support them reliably.

Instead use practical alternatives:

- Alternate tracker arrangements.
- Instrument changes where supported.
- Short transitional jingles.
- Carefully synchronized music switching.
- Additional sound effects.

## 25.4 Music composition requirements

Each world's main track should have:

- A recognizable melodic hook.
- A bassline.
- Rhythmic movement.
- A distinct atmosphere.
- A loop that avoids obvious clicks or interruptions.

World 1:

Upbeat industrial arcade rock.

World 2:

Cold, mysterious electronic music.

World 3:

Fast aggressive volcanic music.

World 4:

Neon cyberpunk-inspired chiptune.

World 5:

Strange, cosmic, rhythmically unstable music.

World 6:

An intense musical combination of previously established motifs.

Boss music:

Strong rhythms, tension, and memorable melodies.

Final boss music:

Epic, fast, dramatic, and climactic.

## 25.5 Sound effects

Create distinct sounds for:

- Moving a piece.
- Rotating.
- Hard drop.
- Locking.
- Holding.
- Clearing one row.
- Clearing two rows.
- Clearing three rows.
- Clearing four rows.
- Triggering a bomb.
- Fire.
- Electricity.
- Ice breaking.
- Magnetic activation.
- Laser.
- Time Freeze.
- Gravity Pulse.
- Nuclear Burst.
- Overheat activation.
- Overheat success.
- Overheat failure.
- Boss damage.
- Boss attacks.
- Boss defeat.
- Stage completion.
- Menu navigation.
- Menu confirmation.
- Menu cancellation.
- Unlocking content.
- Game over.

Sounds should not be excessively loud or unpleasant.

Use volume balancing and suitable playback priorities.

Important events must remain audible even during busy chain reactions.

---

# PART 26 — USER INTERFACE

The UI must look like an actual polished handheld game.

## 26.1 Title screen

Display:

**BLOCK MAYHEM**

**THE SPICE PROTOCOL**

Include an animated background.

Possible elements:

- Falling glowing blocks.
- A giant rotating mechanical eye.
- Sparks.
- Animated flame effects.
- Moving gears.
- Subtle parallax.

Show:

**PRESS START**

Use a tasteful blinking animation.

Include an original title theme.

## 26.2 Main menu

Options:

- ADVENTURE
- CLASSIC
- CHAOS ARCADE
- CHAOS RUN
- DUAL BOARD
- BOSS RUSH
- RECORDS
- OPTIONS

Locked modes must clearly indicate why they are locked.

Menu navigation should feel responsive.

Use visual selection indicators and sounds.

## 26.3 Gameplay HUD

Display the information most relevant to the current mode.

Potential elements:

- Score.
- Level.
- Lines cleared.
- Next pieces.
- Held piece.
- Heat Meter.
- Equipped ability.
- Ability cooldown.
- Current objective.
- Boss HP.
- Combo status.

Avoid showing every possible metric simultaneously.

The 240 × 160 screen requires careful prioritization.

Use context-specific HUD layouts.

## 26.4 Typography

Use an original, readable pixel font.

Text must remain legible on original GBA-sized screens.

Avoid extremely thin lettering.

Avoid dense paragraphs during gameplay.

Use short contextual instructions.

## 26.5 Danger indicators

When the board becomes dangerously full:

- Show a warning symbol.
- Change the border color.
- Increase the urgency of audio feedback.
- Optionally animate a subtle warning light.

Do not obstruct the player's view.

## 26.6 Pause menu

Options:

- RESUME
- RESTART
- CONTROLS
- OPTIONS
- QUIT

Require confirmation before abandoning a run when necessary.

Pause must stop gameplay timers and boss attacks.

---

# PART 27 — PLAYER PROGRESSION

## 27.1 World unlocking

Worlds unlock sequentially.

Defeating a world boss unlocks the next world.

Completed stages remain replayable.

## 27.2 Performance ranks

Award ranks based on performance.

Suggested:

- C
- B
- A
- S
- S+

Use transparent scoring criteria.

Suggested factors:

- Score.
- Time.
- Efficiency.
- Damage or garbage received.
- Successful objectives.
- Skillful combos.

Do not create impossible rank thresholds.

## 27.3 Rewards

Completing stages can unlock:

- New abilities.
- Additional game modes.
- Cosmetic themes.
- New music in a sound-test menu.
- New challenge modifiers.
- Achievements.

Avoid permanent upgrades that undermine the difficulty curve.

---

# PART 28 — ACHIEVEMENTS

Implement at least twenty meaningful achievements.

Examples:

1. First Line — Clear your first row.
2. Double Trouble — Clear two rows simultaneously.
3. Triple Threat — Clear three rows.
4. Perfect Four — Clear four rows simultaneously.
5. Hot Stuff — Activate Overheat.
6. Heat Master — Complete Overheat successfully.
7. Bomb Squad — Trigger ten bombs.
8. Chain Rookie — Trigger a two-wave chain.
9. Chain Master — Trigger a five-wave chain.
10. Meltdown — Reach an eight-wave chain.
11. Electrician — Activate twenty electrical cells.
12. Firestarter — Trigger fifteen fire effects.
13. Ice Breaker — Destroy thirty frozen cells.
14. Laser Expert — Achieve a precise laser clear.
15. Gravity Wizard — Complete a stage using gravity manipulation.
16. Cross Over — Complete a dual-board combo.
17. Factory Hero — Defeat the first boss.
18. World Traveler — Complete three worlds.
19. Mayhem Champion — Complete the campaign.
20. Chaos Survivor — Finish one Chaos Run.
21. Speed Demon — Complete a qualifying stage under its target time.
22. Untouchable — Defeat a boss without receiving an eligible hostile attack.
23. Puzzle Genius — Achieve an S+ stage rank.
24. Last Stand — Survive a critical-height situation.
25. Master of Mayhem — Complete a demanding collection of advanced achievements.

Each achievement must have:

- A valid trigger.
- Progress tracking where relevant.
- A saved unlock state.
- A small icon.
- A brief description.
- A satisfying notification.

Do not award achievements for events that never actually occurred.

---

# PART 29 — SAVE SYSTEM

Implement persistent save data.

Use a GBA-compatible save method supported by the selected engine and target cartridge configuration.

An SRAM-based implementation is acceptable when configured and tested properly.

Save:

- Completed stages.
- Unlocked worlds.
- Unlocked modes.
- High scores.
- Best ranks.
- Achievements.
- Gameplay settings.
- Selected cosmetic options.
- Relevant player statistics.

## 29.1 Save integrity

Use:

- A version identifier.
- A magic header.
- A checksum or other integrity validation.
- Safe initialization for missing saves.
- Defined behavior for corrupted saves.

Do not crash on invalid save data.

Do not read uninitialized memory.

Do not erase valid progress because an unrelated setting changes.

## 29.2 Save verification

Test:

- First launch.
- Saving progress.
- Restarting the emulator.
- Reloading progress.
- Invalid save data.
- Version mismatch.
- Full completion state.

If emulator-based save persistence cannot be tested in the current environment, state that limitation explicitly.

---

# PART 30 — DIFFICULTY AND BALANCING

Balance is a critical requirement.

The goal is to create a game that is:

**Easy to understand, difficult to master, and consistently fair.**

## 30.1 Difficulty settings

Implement:

- BEGINNER
- STANDARD
- EXPERT

Beginner:

- Slower gravity.
- Longer attack telegraphs.
- Slightly more forgiving hazards.
- Reduced Overheat failure penalty.

Standard:

The intended balanced experience.

Expert:

- Faster gravity.
- More complex hazard patterns.
- Tougher boss timing.
- More demanding objectives.

Expert difficulty should not merely multiply enemy HP.

It should create meaningful gameplay pressure.

## 30.2 Core balance principles

The player must always have opportunities to recover from ordinary mistakes.

A badly managed board should become progressively more dangerous.

Do not routinely generate instant unavoidable losses.

High-level players should be rewarded for:

- Planning.
- Positioning.
- Timing.
- Resource management.
- Combo construction.
- Boss pattern recognition.
- Risk assessment.

## 30.3 Power balance

No ability may trivialize the central puzzle.

Define ability efficiency in terms of:

- Cells removed.
- Heat consumed.
- Cooldown.
- Risk reduction.
- Potential score.
- Positional control.

Expensive abilities should not necessarily be universally better.

A precise cheap ability should sometimes be more useful than a powerful expensive one.

## 30.4 Risk and reward

Overheat must be tempting but dangerous.

A successful Overheat should feel rewarding.

A failed Overheat should be painful but generally survivable.

Risk should be understandable in advance.

## 30.5 Randomness

Randomness creates variety.

It must not determine success regardless of player skill.

Use seeded randomness.

Provide bounded special-cell distributions.

Avoid impossible objectives caused by random generation.

Ensure required mechanic introductions are scripted or guaranteed.

## 30.6 Difficulty curves

Each world should:

1. Introduce a mechanic.
2. Allow the player to practice it.
3. Combine it with familiar mechanics.
4. Test mastery.
5. Conclude with a boss.

Do not introduce several completely unfamiliar mechanics simultaneously without teaching them.

## 30.7 Balance documentation

Create `BALANCE.md`.

Record:

- Gravity speeds.
- Ability costs.
- Cooldowns.
- Heat gains.
- Boss HP.
- Boss damage rules.
- Environmental hazard frequencies.
- Score multipliers.
- Upgrade limits.
- Difficulty modifiers.

Keep balance values centralized in configuration rather than scattered across unrelated source files.

---

# PART 31 — AUTOMATED GAMEPLAY SIMULATION

Create a host-side simulation and testing layer.

The underlying game logic should be testable without running graphical GBA code.

## 31.1 Deterministic simulation

Support fixed random seeds.

Given the same initial state, seed, and inputs, the game should produce the same logical outcome.

Visual randomness may use a separate generator so particles do not change gameplay outcomes.

## 31.2 Simulation scenarios

Test:

- Normal beginner play.
- Aggressive hard-dropping.
- High-stack recovery.
- Rapid piece rotation.
- Repeated hold usage.
- Long chain reactions.
- Multiple bombs triggering.
- Complex electrical interactions.
- Frozen rows.
- Gravity events.
- Dual-board switching.
- Boss phase transitions.
- Overheat expiration.
- Overheat success.
- Roguelite upgrade combinations.

## 31.3 Balance analysis

Where practical, create simple automated agents representing different strategies.

Examples:

- Random legal placement.
- Height-minimizing placement.
- Hole-minimizing placement.
- Line-clear-focused placement.
- Simple resource-aware placement.

These agents are not substitutes for human playtesting.

Use them to identify:

- Broken scoring loops.
- Unfair hazard combinations.
- Degenerate strategies.
- Overpowered abilities.
- Nonterminating cascades.
- Unexpected difficulty spikes.

Run repeated simulations using different seeds.

Record representative results.

## 31.4 Exploit testing

Specifically attempt to find:

- Infinite score farming.
- Infinite heat generation.
- Infinite Overheat activation.
- Unlimited ability use.
- Negative cooldowns.
- Duplicate boss damage.
- Repeated achievement rewards.
- Invalid board states.
- Softlocks.
- Impossible stage objectives.
- Save-data corruption.
- Infinite chain reactions.

Fix the underlying causes.

Do not merely hide symptoms.

---

# PART 32 — GAME FEEL

This section is extremely important.

A game can contain hundreds of features and still feel bad.

The fundamental controls must feel satisfying.

## 32.1 Responsiveness

The player should feel that inputs are immediate.

Rotation must be reliable.

Horizontal movement must be precise.

Hard drops must register correctly.

Hold must behave consistently.

## 32.2 Satisfaction

Clearing a row should feel good.

Clearing four rows should feel fantastic.

Triggering a large chain should feel extraordinary.

Defeating a boss should feel triumphant.

These differences should be communicated through:

- Sound.
- Animation.
- Timing.
- Particle effects.
- Score feedback.
- Music.
- Controlled screen shake.

## 32.3 Momentum

Avoid unnecessarily interrupting gameplay.

The game should maintain arcade momentum.

Tutorials should be short.

Menus should be responsive.

Animations should be expressive but not excessively long.

## 32.4 Flow

The player should frequently encounter decisions such as:

- Should I clear one row now or prepare four?
- Should I use my laser or save the heat?
- Should I protect the board or attempt Overheat?
- Should I trigger the bomb immediately?
- Should I set up an electrical chain?
- Should I switch boards now?
- Should I choose an offensive upgrade or a defensive one?
- Should I clear the board or attack the boss?

These decisions are the source of strategic depth.

---

# PART 33 — HARDWARE OPTIMIZATION

Respect the GBA's limitations from the beginning.

## 33.1 Rendering

Prefer hardware-accelerated tiled backgrounds and sprites.

Avoid full-frame software rendering when more efficient methods exist.

Update only what needs updating.

Use appropriate sprite batching and reuse.

## 33.2 Memory

Use compact representations for board cells.

A board cell may store:

- Occupied state.
- Base visual type.
- Special property.
- Armor state.
- Small flags.

Avoid bloated per-cell objects.

Use fixed-size arrays for boards.

Use pooled particles.

## 33.3 Sprite limits

Plan sprite usage carefully.

Do not assume unlimited hardware sprites.

Large bosses should use efficient combinations of tiled graphics and hardware sprites.

Avoid excessive sprites on one scanline.

Watch for disappearing sprites caused by hardware limitations.

## 33.4 Visual effects

Use:

- Palette cycling.
- Tile animation.
- Limited affine transformations.
- Background scrolling.
- Sprite priority.
- Efficient particle pooling.

Avoid relying on expensive software transformations every frame.

## 33.5 Audio

Balance sound quality against CPU and memory usage.

Use compressed or tracker-based assets where appropriate.

Avoid huge uncompressed audio files.

## 33.6 Performance target

Target smooth gameplay at the display refresh rate.

Measure performance where tools permit.

Stress-test:

- Dense boards.
- Multiple explosions.
- Boss animations.
- Music playback.
- Many particles.
- Dual-board mode.
- Large chain reactions.

If performance degrades, optimize the relevant system.

Do not solve performance problems by silently disabling essential game mechanics.

---

# PART 34 — VISUAL COMPOSITION AND READABILITY

At 240 × 160 pixels, layout is a genuine design challenge.

Treat it seriously.

## 34.1 Single-board layout

Reserve an approximately 80 × 144 pixel region for the board.

Use the remaining area for:

- Animated character or boss.
- Next pieces.
- Score.
- Heat.
- Ability indicator.
- Context-specific objective information.

Avoid overcrowding the screen.

## 34.2 Boss encounters

Bosses should be prominently visible.

Do not reduce the board so much that placing blocks becomes difficult.

Use compact HUD elements.

Boss attacks should be visible without obscuring critical cells.

## 34.3 Dual-board layout

Both boards must be readable simultaneously.

Use smaller but visually distinct cells.

Reduce decorative HUD elements as necessary.

Maintain clear active-board highlighting.

## 34.4 Accessibility

Provide options for:

- Reduced screen shake.
- Reduced flashing.
- Music volume.
- Sound-effect volume.
- Ghost piece.
- Difficulty.

Special-cell types should have distinct symbols, not only colors.

Avoid rapidly flashing the entire screen.

---

# PART 35 — SECRETS AND SURPRISES

Add personality and rewarding discoveries.

## 35.1 Secret visual events

Examples:

- A tiny robot briefly appears after a huge combo.
- A machine malfunctions after repeated explosions.
- A rare lightning storm occurs during a major chain.
- A boss displays a funny facial expression after being defeated.
- A hidden background animation appears after an S+ rank.

These should not affect gameplay unfairly.

## 35.2 Secret achievements

Include several hidden achievements that reward advanced play.

## 35.3 Easter eggs

Create original small secrets.

Do not copy recognizable copyrighted characters or sprites.

## 35.4 Unlockable cosmetics

Possible unlocks:

- Alternate block themes.
- Different board frames.
- Additional title-screen variations.
- Special particle styles.

All cosmetics must preserve gameplay readability.

---

# PART 36 — STORY PRESENTATION

The story should be told economically.

The player is here primarily to play.

Use:

- Short intro cutscene.
- Brief world introductions.
- Boss entrance sequences.
- Small dialogue cards.
- Animated expressions.
- Short victory sequences.
- Final ending.

Avoid enormous walls of text.

Characters should communicate personality through visual animation.

## 36.1 Intro cutscene

Suggested sequence:

A peaceful futuristic arcade factory operates normally.

A mysterious energy source begins destabilizing.

The machines overload.

Strange geometric structures begin falling from the sky.

A giant eye opens inside the central reactor.

The factory enters emergency mode.

The player activates the experimental Block Mayhem system.

The title appears.

The music begins.

This opening should be visually memorable but brief.

---

# PART 37 — CONTENT DELIVERY REQUIREMENTS

The game must include:

- A polished title screen.
- An animated main menu.
- A working core falling-block engine.
- All seven standard shapes.
- Correct rotation and collision behavior.
- Hold mechanics.
- Next-piece preview.
- Ghost piece.
- Score and levels.
- The special-cell system.
- The chain-reaction system.
- The Heat Meter.
- Overheat.
- The active ability system.
- Classic Mode.
- Chaos Arcade.
- Adventure Mode.
- Thirty campaign stages.
- Six visually distinct worlds.
- Six major bosses.
- Dual Board Relay.
- Chaos Run.
- The roguelite upgrade system.
- Boss Rush.
- Achievements.
- Persistent save data.
- Original graphics.
- Original animations.
- Original music.
- Sound effects.
- Difficulty settings.
- Options.
- Credits.
- Victory screens.
- Game-over screens.
- A final ending.
- A genuine buildable GBA ROM.

No fake menu entries.

No placeholder bosses that are merely rectangles with HP counters.

No empty worlds.

No inactive abilities represented only by icons.

No fake music claims when the ROM is silent.

No unsupported claims that everything works.

---

# PART 38 — IMPLEMENTATION ORDER

Although the final game is ambitious, implementation must be disciplined.

Work in the following order.

## PHASE 1 — Environment and project foundation

- Inspect development environment.
- Verify dependencies.
- Establish project architecture.
- Build a minimal ROM.
- Establish emulator testing where available.

**Required result:** A real ROM that boots.

## PHASE 2 — Core falling-block gameplay

Implement:

- Board.
- Pieces.
- Randomizer.
- Rotation.
- Collision.
- Gravity.
- Locking.
- Line clearing.
- Scoring.
- Hold.
- Preview.
- Ghost piece.
- Input.
- Game over.

**Required result:** A genuinely enjoyable basic falling-block game.

Do not move on before verifying these fundamentals.

## PHASE 3 — Visual foundation

Implement:

- Attractive board graphics.
- Main HUD.
- Basic backgrounds.
- Animations.
- Particle system.
- Menu visuals.
- Basic sound feedback.

**Required result:** The basic game already looks and feels like a real GBA title.

## PHASE 4 — Power systems

Implement:

- Special-cell types.
- Chain reactions.
- Heat.
- Overheat.
- Active abilities.
- Ability cooldowns.
- Special-effect animations.

**Required result:** The central BLOCK MAYHEM gameplay experience is functional.

## PHASE 5 — World 1 vertical slice

Implement the entire first world.

This includes:

- Five stages.
- Tutorials.
- Objectives.
- Distinct stage configurations.
- Animated environments.
- The Block Monster boss.
- Boss phase transitions.
- Music.
- Victory sequence.
- Progression unlock.

**Required result:** A fully playable and highly polished first world demonstrating the final game's quality standard.

## PHASE 6 — Remaining worlds

Implement worlds 2–6.

Reuse the underlying systems intelligently.

Create original:

- Hazard combinations.
- Level objectives.
- Boss appearances.
- Attack patterns.
- Backgrounds.
- Music arrangements.
- Stage progression.

Do not simply duplicate World 1 with a different color palette.

## PHASE 7 — Additional modes

Implement:

- Classic.
- Chaos Arcade.
- Chaos Run.
- Dual Board Relay.
- Boss Rush.

Ensure the modes are meaningfully different.

## PHASE 8 — Progression and persistence

Implement:

- Achievements.
- Unlocks.
- Save data.
- Records.
- Settings.
- Player statistics.

## PHASE 9 — Content and audiovisual polish

Improve:

- Animation quality.
- Original music.
- Sound effects.
- Cutscenes.
- UI transitions.
- Background animation.
- Victory celebrations.
- Boss defeat sequences.
- Visual readability.

## PHASE 10 — Testing and optimization

- Run automated tests.
- Run deterministic simulations.
- Test edge cases.
- Build the final ROM.
- Test in an emulator where possible.
- Fix observed bugs.
- Verify progression.
- Verify performance.
- Verify save behavior.

**Required result:** A verified and playable `BlockMayhem.gba`.

---

# PART 39 — TESTING REQUIREMENTS

Create tests covering the important game rules.

## 39.1 Basic engine tests

Test:

- All piece rotations.
- Wall kicks.
- Collision boundaries.
- Piece spawning.
- Seven-bag distribution.
- Hold restrictions.
- Hard drop.
- Lock delay.
- Single-line clears.
- Double-line clears.
- Triple-line clears.
- Four-line clears.
- Top-out behavior.

## 39.2 Special-cell tests

Test:

- Bomb radius.
- Bomb chain reactions.
- Electrical propagation.
- Fire propagation.
- Magnetic movement.
- Rainbow activation.
- Ice damage.
- Armor damage.
- Overlapping effects.
- Chain termination.
- Invalid coordinate handling.

## 39.3 Heat tests

Test:

- Heat accumulation.
- Maximum heat.
- Ability costs.
- Cooldowns.
- Overheat activation.
- Overheat success.
- Overheat failure.
- Timer expiration.
- Pause behavior.
- No infinite heat loops.

## 39.4 Boss tests

Test:

- HP reduction.
- Attack scheduling.
- Phase transitions.
- Attack warnings.
- Boss death.
- Stage completion.
- No attacks during pause.
- No repeated victory rewards.

## 39.5 Dual-board tests

Test:

- Board switching.
- Forced switching.
- Shared heat.
- Separate grids.
- Separate failure conditions.
- Cross-board combos.
- Incoming garbage.
- Simultaneous resolution safety.

## 39.6 Roguelite tests

Test:

- Run generation.
- Valid upgrade choices.
- Upgrade stacking.
- Synergies.
- Seed reproducibility.
- Boss progression.
- Run completion.
- Run failure.
- No infinite resource generation.

## 39.7 Save tests

Test valid and invalid save conditions.

## 39.8 Build verification

The final build must produce:

`BlockMayhem.gba`

Check the actual file exists.

Check the build system reports success.

Use appropriate ROM validation tools where available.

If an emulator is available, launch the ROM and verify that it reaches the title screen and that gameplay starts.

Do not equate successful compilation with successful runtime testing.

---

# PART 40 — QUALITY GATES

Before declaring completion, evaluate the project against these gates.

## Gate A — Functional gameplay

Can the player start and complete a real game?

Do movement, rotation, collision, locking, and line clearing work correctly?

## Gate B — Strategic depth

Do powers create meaningful decisions?

Can the player plan chains?

Does heat management involve genuine tradeoffs?

## Gate C — Fairness

Are boss attacks telegraphed?

Can players understand why they lost?

Are random elements controlled?

## Gate D — Content

Are all worlds playable?

Are the bosses distinct?

Do the modes function?

## Gate E — Presentation

Are the graphics attractive?

Are animations present?

Does the game contain music and sound effects?

Is the HUD readable?

## Gate F — Technical stability

Does the project compile?

Does the ROM boot?

Are important systems tested?

Is the performance acceptable?

## Gate G — Persistence

Do unlocks and records save correctly?

## Gate H — Authenticity

Is this truly a Game Boy Advance ROM?

Does gameplay run in the GBA environment without external services?

These gates must be evaluated using evidence.

Do not mark untested gates as passed.

---

# PART 41 — DEVELOPMENT CONTINUITY

This project is substantial.

Your work may be interrupted by context limits, session limits, environment interruptions, or other constraints.

You must prevent loss of progress.

Create and maintain:

`CONTINUE_HERE.md`

Update this document after every major implementation milestone.

It must contain:

1. Current project state.
2. Successfully implemented systems.
3. Partially implemented systems.
4. Remaining tasks.
5. Current build status.
6. Known bugs.
7. Important architecture decisions.
8. Exact commands needed to build.
9. Exact recommended next implementation step.
10. Most recent test results.
11. Relevant commit identifiers where available.

Maintain a detailed but concise `GAME_DESIGN.md`.

Maintain `BALANCE.md`.

Maintain `TESTING.md`.

Use Git commits at meaningful milestones.

Examples:

- Project foundation.
- Core puzzle engine.
- Special-cell system.
- Heat and powers.
- World 1.
- Additional worlds.
- Roguelite.
- Final polish.

Do not depend on conversational memory to preserve critical implementation details.

The repository itself must contain the necessary knowledge to resume development.

If the session is interrupted, another coding-agent session should be able to inspect the project, read `CONTINUE_HERE.md`, and continue without rebuilding the work from scratch.

---

# PART 42 — REQUIRED OUTPUT FILES

At completion, the project must contain:

## Executable

`BlockMayhem.gba`

## Source code

All required C++ source and header files.

## Build files

A functional Makefile and relevant configuration.

## Assets

All necessary GBA-compatible visual and audio assets.

## Documentation

`README.md`

Explain:

- How to install dependencies.
- How to build the ROM.
- How to run the ROM in mGBA.
- GBA controls.
- Available game modes.
- Save behavior.
- Basic troubleshooting.

`GAME_DESIGN.md`

Document the actual implemented gameplay systems.

`BALANCE.md`

Document numerical gameplay rules.

`TESTING.md`

Document tests executed and results.

`CONTINUE_HERE.md`

Document implementation status and unfinished tasks.

`CREDITS.md`

Record original asset creation and third-party dependency licenses.

The project should be independently buildable by someone with the documented toolchain.

Avoid undocumented dependencies on temporary files or absolute local paths.

---

# PART 43 — ARTISTIC QUALITY STANDARD

Imagine this game being reviewed as a newly discovered GBA cartridge.

The reviewer should immediately notice:

- A striking title screen.
- Detailed pixel art.
- Excellent controls.
- A catchy soundtrack.
- Impressive animations.
- A genuinely original twist on falling-block gameplay.
- Surprising mechanical depth.
- Well-designed bosses.
- Many reasons to replay.

The first five minutes are especially important.

The player should experience:

1. An impressive title screen.
2. An immediately enjoyable puzzle game.
3. A satisfying line clear.
4. The discovery of the Heat Meter.
5. A dramatic special-block explosion.
6. The realization that strategic chains are possible.

The first boss should make the player think:

**"I didn't know a falling-block game could do this."**

This is the emotional target.

---

# PART 44 — FINAL DESIGN PRINCIPLES

Keep these principles in mind throughout development.

### Principle 1 — Precision before spectacle

Every animation must support responsive gameplay.

### Principle 2 — Readability before complexity

The player must understand the board.

### Principle 3 — Skill before randomness

A better player should generally achieve better outcomes.

### Principle 4 — Powers must create decisions

Abilities should not function as unrestricted shortcuts.

### Principle 5 — Surprises should be learnable

Unexpected events can be exciting without being unfair.

### Principle 6 — Each world must introduce something meaningful

Do not use superficial visual changes as a substitute for gameplay variety.

### Principle 7 — Bosses must test mastery

Boss attacks should build upon previously learned mechanics.

### Principle 8 — Replayability must emerge from systems

Different upgrade builds should genuinely play differently.

### Principle 9 — Hardware limitations should inspire creativity

Make the game impressive because its assets, effects, animation, and mechanics were intelligently designed for the GBA.

### Principle 10 — Complete the real game

The final deliverable is a working ROM, not a description of a hypothetical product.

---

# PART 45 — FINAL ACCEPTANCE CRITERIA

A successful result satisfies the following:

### Technical

- A real `.gba` file exists.
- The ROM is built from authentic GBA-compatible source code.
- The build is reproducible.
- The game initializes correctly.
- Gameplay does not depend on external services.
- The architecture is maintainable.

### Gameplay

- Falling pieces work correctly.
- Line clearing works correctly.
- Scoring works.
- Special cells function.
- Chain reactions function.
- Heat and Overheat function.
- Active abilities function.
- Boss combat functions.
- Dual-board gameplay functions.
- Roguelite upgrades function.

### Content

- Six worlds.
- Thirty campaign stages.
- Six bosses.
- Multiple playable modes.
- Achievements.
- Unlockable progression.
- Persistent save data.

### Presentation

- Original pixel art.
- Animated backgrounds.
- Animated bosses.
- Impressive explosions.
- Satisfying line-clear effects.
- Original music.
- Distinct sound effects.
- Polished menus.
- Clear HUD.
- Victory and defeat sequences.

### Quality

- Important gameplay systems tested.
- No known game-breaking bugs.
- No known infinite scoring exploits.
- No known infinite resource exploits.
- No known softlocks in standard progression.
- Difficulty follows a coherent curve.
- Gameplay remains readable.
- Performance is appropriate for the platform.

---

# PART 46 — MANDATORY EXECUTION BEHAVIOR

Read this section carefully.

Your job is to **execute this project**, not to provide an elaborate response describing what you intend to execute.

Begin by inspecting the working directory and available development tools.

Establish a functioning GBA build environment.

Build the foundation.

Implement the gameplay.

Create the graphics and sound.

Continue through the planned systems.

Compile and test repeatedly.

Use the available tools directly.

Make progress through actual source-code changes.

Do not spend the entire session producing planning documents.

Do not create hundreds of empty files merely to simulate progress.

Do not generate impressive-looking menus that lead to unimplemented gameplay.

Do not replace missing graphics with permanent blank placeholders.

Do not fabricate compiler output.

Do not fabricate test results.

Do not claim the game was played in an emulator if it was not.

When implementation becomes difficult, investigate the actual technical problem and solve it.

When graphics asset conversion fails, inspect the expected format and correct the asset.

When an API call fails, consult the installed library documentation.

When gameplay tests fail, trace the underlying rule.

When an animation creates performance problems, optimize it.

When a balance rule creates an exploit, revise the rule and document the change.

Maintain a functioning ROM throughout development.

At major milestones, save your progress and update the continuity document.

Continue autonomously rather than repeatedly asking for approval.

If execution must stop before full completion, leave the repository in the best playable and buildable state possible, document precisely what remains, and never misrepresent unfinished features as complete.

---

# PART 47 — THE MOST IMPORTANT FINAL DIRECTIVE

I want you to approach this as a passionate and technically skilled game developer creating the most ambitious falling-block arcade adventure that can realistically be delivered on Game Boy Advance hardware.

Do not create an ordinary puzzle game with a few decorative extras.

Create something with its own identity.

Something full of personality.

Something that rewards intelligence and practice.

Something that makes clearing four lines feel incredible.

Something where the player deliberately constructs massive chain reactions.

Something where managing the Heat Meter becomes a strategic art.

Something where using a laser at exactly the right moment can save an entire run.

Something where boss battles feel genuinely dramatic.

Something where each world introduces a new way to think.

Something where different roguelite builds produce different strategies.

Something that looks, sounds, and feels astonishingly alive on a 240 × 160 pixel screen.

I want detailed animation, exciting explosions, original music, colorful worlds, memorable bosses, creative abilities, surprising interactions, and carefully engineered game systems.

But above all:

**I want an excellent game.**

A game that is balanced.

A game that is deep.

A game that is exciting.

A game that is replayable.

A game that rewards mastery.

A game that genuinely works.

Use the creative freedom available to you.

Make strong design decisions.

Implement them carefully.

Test them critically.

Polish them.

And build the actual Game Boy Advance ROM.

# THE FINAL PRODUCT IS:

# BLOCK MAYHEM

## THE SPICE PROTOCOL

**A complete, original, visually spectacular, mechanically sophisticated Game Boy Advance falling-block action adventure.**

**Now begin creating the project and implementing the game.**

# BLOCK MAYHEM: THE SPICE PROTOCOL

# ULTIMATE DEVELOPMENT DIRECTIVE — EXPANSION PACK

## PARTS 48–65 — AUTONOMOUS DEVELOPMENT, PRODUCTION PARITY, DEEP BALANCING AND FINAL QUALITY ASSURANCE

# IMPORTANT: PRESERVATION OF THE ORIGINAL SPECIFICATION

This document is a direct and mandatory extension of the complete BLOCK MAYHEM development specification, Parts 1–47.

**EVERY REQUIREMENT IN PARTS 1–47 REMAINS IN FORCE.**

Nothing in this expansion authorizes the deletion, simplification, replacement, or omission of any previously defined gameplay system, world, stage, boss, animation, ability, game mode, technical requirement, or deliverable.

This expansion establishes additional quality requirements, production standards, development procedures, and autonomous execution protocols.

Where the original specification gives special implementation priority to World 1, interpret that priority strictly as a development-order decision.

It must never be interpreted as permission to give Worlds 2–6 inferior final production quality.

Where quantitative targets in this expansion are incompatible with actual GBA hardware limitations, preserve the intended visual or gameplay experience using an appropriate technical alternative. Document the evidence and the resulting compromise.

Do not silently weaken the specification.

The final objective remains unchanged:

**CREATE A COMPLETE, EXCEPTIONALLY POLISHED, BALANCED, ORIGINAL GAME BOY ADVANCE ROM.**

---

# PART 48 — AUTONOMOUS DEVELOPMENT LOOP

## 48.1 The fundamental requirement

You must treat this project as an extended autonomous software-development assignment.

Do not approach the specification as a task to complete in one conversational response.

Do not assume that creating an initial playable prototype constitutes completion.

The development process must continuously repeat a structured cycle:

**PLAN → IMPLEMENT → BUILD → TEST → INSPECT → CRITIQUE → IMPROVE → VERIFY → REPEAT**

The purpose of this cycle is to systematically transform the initial implementation into the complete game defined by the specification.

You are responsible for identifying unfinished work.

You are responsible for determining the next development priority.

You are responsible for identifying and correcting defects.

You are responsible for validating completed features.

Do not require the user to manually identify every missing system.

Do not repeatedly ask what to implement next when the specification already provides the answer.

## 48.2 Distinguish autonomous work from automatic continuation

An instruction written in a prompt does not, by itself, guarantee that Claude Code can restart itself after its turn ends.

Therefore, separate two responsibilities:

**Development responsibility:**

Continue making meaningful tool-driven progress during every active turn.

**Continuation responsibility:**

Use a supported Claude Code continuation mechanism when available.

Prefer the native `/goal` mechanism on compatible Claude Code versions.

Do not require the Ralph Loop plugin.

Do not claim a persistent loop exists unless an actual continuation mechanism is active.

Do not claim the session will continue indefinitely through rate limits, interrupted processes, unavailable tools, or exhausted usage quotas.

The user should be able to launch the complete project through one development objective, with repeated work performed automatically while the environment permits.

## 48.3 Preferred continuation mechanism: native Claude Code goal

When Claude Code provides the `/goal` command, use it as the preferred continuation mechanism.

The project objective must be defined through a concise, verifiable completion condition referring to this complete specification.

The goal should require the agent to:

- Read the entire specification.
- Maintain a persistent task inventory.
- Implement all required systems.
- Build the GBA ROM.
- Execute relevant automated tests.
- Validate the game through an emulator when available.
- Inspect the content of every world.
- Verify all playable modes.
- Resolve defects.
- Improve gameplay balance.
- Complete the audiovisual production requirements.
- Produce a final release-verification report.

The goal must not be considered satisfied merely because a file named `BlockMayhem.gba` exists.

A compiled ROM can still contain severe gameplay bugs, unfinished screens, missing animations, broken stage objectives, silent music, or nonfunctional modes.

The completion condition must incorporate objective evidence of implemented functionality.

## 48.4 Alternative continuation mechanisms

If `/goal` is unavailable, investigate the following alternatives in order:

1. An available, supported Ralph-style plugin.
2. A project-local Stop hook.
3. A bounded external Claude Code CLI runner.
4. Manual session resumption using persistent project state.

Do not waste development time repeatedly attempting to install an unavailable plugin.

A project-local Stop hook may be implemented without an external plugin, using Claude Code's documented hook interface.

However, any custom hook must be:

- Scoped to this project.
- Disabled by default outside the requested development workflow.
- Explicitly bounded.
- Able to recognize successful completion.
- Able to recognize a user-requested stop.
- Able to detect repeated lack of progress.
- Compatible with the operating system.
- Non-destructive.
- Removable without affecting the game.
- Designed not to hijack unrelated Claude Code sessions.

Do not automatically install global hooks affecting other projects.

Do not silently change global permission settings.

Do not configure an unbounded automatic development process.

If using an external CLI runner, document how it is launched from a normal terminal, outside an already active Claude Code session.

Do not recursively spawn uncontrolled Claude processes.

## 48.5 Iteration structure

Every development iteration must have six explicit stages.

### Stage A — Inspect

Read the current project state.

Inspect:

- `GAME_SPEC.md`
- `TASKS.md`
- `CONTINUE_HERE.md`
- `BALANCE.md`
- `TESTING.md`
- Relevant source files
- Recent test results
- Git status
- Recent commits

Determine what is already implemented.

Do not recreate completed systems.

Do not overwrite functioning features without understanding them.

### Stage B — Select

Choose the highest-priority unfinished task.

Prioritize dependencies correctly.

For example, a boss attack requiring directional gravity must not be implemented before the underlying gravity system behaves correctly.

Prefer completing meaningful, verifiable features over scattering unfinished changes across many files.

### Stage C — Implement

Write the actual code, assets, tests, and configuration.

Follow the architecture and hardware constraints defined in the original specification.

Avoid fake implementations.

Avoid stubs that silently return success.

Avoid decorative UI controls that are not connected to real game logic.

### Stage D — Verify

Perform relevant checks.

Depending on the task, these may include:

- Compilation.
- Unit tests.
- Deterministic gameplay tests.
- Asset conversion validation.
- Emulator execution.
- Save/load tests.
- Performance measurements.
- Visual inspection.
- Gameplay simulation.

A successful compiler exit code is not sufficient evidence that the feature works correctly.

### Stage E — Critique

Critically evaluate the implementation.

Ask:

- Is this genuinely fun?
- Does it behave according to the specification?
- Are there hidden edge cases?
- Can the system be exploited?
- Is the interface understandable?
- Is the animation readable?
- Is the effect satisfying?
- Is the implementation appropriate for GBA hardware?
- Does it introduce regressions?
- Would a skilled player consider it fair?

Do not automatically approve your own implementation.

### Stage F — Record and Continue

Update project tracking files.

Record:

- What changed.
- Why it changed.
- Which tests were run.
- Which tests passed.
- Which tests failed.
- Remaining defects.
- The next concrete task.

Commit stable milestones when appropriate.

Then continue with the next task while the execution environment and continuation mechanism permit.

## 48.6 Handling failed iterations

A failed attempt is not permission to abandon the project.

When a failure occurs:

1. Capture the exact error.
2. Identify the relevant subsystem.
3. Separate confirmed facts from hypotheses.
4. Investigate the root cause.
5. Apply the smallest correct repair.
6. Rebuild.
7. Run focused tests.
8. Run regression tests.
9. Record the result.

Avoid repeatedly applying the same unsuccessful repair.

If several attempts fail, reconsider the underlying architecture or technical assumption.

Preserve a working project state.

## 48.7 Handling context exhaustion

Critical project knowledge must not exist only in conversational context.

Maintain a persistent development journal.

Before a context-compaction event, when detectable, update the journal.

If a session is interrupted, the next session must be able to recover the state from the repository.

The development journal must identify:

- Current implementation milestone.
- Last known working build.
- Last completed task.
- Active task.
- Unfinished work.
- Important design decisions.
- Known failing tests.
- Required environment configuration.
- Next implementation step.

## 48.8 Completion

Do not declare the project complete based on subjective confidence.

Completion requires evidence from the release-verification process defined later in this expansion.

If required validation cannot be performed, mark it as unverified.

Never manufacture successful test reports.

Never claim human playtesting took place unless a human actually played the game.

Never claim emulator validation took place without a real emulator run.

---

# PART 49 — EQUAL PRODUCTION QUALITY ACROSS ALL SIX WORLDS

## 49.1 Absolute requirement

**ALL SIX WORLDS MUST RECEIVE THE SAME LEVEL OF FINAL PRODUCTION INVESTMENT.**

This requirement is mandatory.

World 1 is not the premium world.

World 1 is not the only world receiving advanced animation.

World 1 is not the only world receiving elaborate music.

World 1 is not the only world receiving a fully animated boss.

World 1 is not the only world receiving handcrafted environments.

World 1 is simply the first world developed.

The game must not resemble a polished demo followed by five unfinished expansions.

## 49.2 Equal investment does not mean identical design

Every world must have a distinct creative identity.

Different worlds should emphasize different strengths.

The Frozen Factory may specialize in crystalline effects and tactical pressure.

Volcano Madness may specialize in explosions and high-intensity play.

Electric City may specialize in electrical chains and coordinated board management.

Gravity Dimension may specialize in unusual spatial puzzles and surreal animation.

The Final Meltdown may specialize in combining previously learned mechanics.

These experiences should differ dramatically.

However, they must receive comparable attention to:

- Graphics.
- Animation.
- Music.
- Gameplay systems.
- Stage design.
- Boss complexity.
- Level balancing.
- Environmental effects.
- Transitions.
- Player feedback.
- Hidden details.
- Testing.

## 49.3 World production requirements

Each world must include, at minimum:

### Visual identity

A unique environment palette.

Original environmental artwork.

A distinct visual theme.

Multiple decorative elements.

A world-specific board frame.

A world-specific background composition.

Animated environmental elements.

### Animation

Multiple animated environmental features.

Special effects appropriate to the theme.

Stage transitions.

Victory feedback.

Environmental hazard animations.

Boss animation.

### Music

A recognizable original musical identity.

A world-specific gameplay composition or arrangement.

A boss theme or distinguishable boss arrangement.

Short musical accents for important events.

### Gameplay

Four distinct non-boss stages.

A mechanically distinct boss encounter.

A coherent progression of challenges.

Meaningful use of the world's signature mechanics.

At least one interesting interaction with previously learned mechanics.

### Presentation

A world introduction.

A stage-completion presentation.

A boss entrance.

A boss defeat animation.

A world-completion celebration.

### Quality assurance

Verified level objectives.

Verified failure conditions.

Verified progression unlocks.

Verified boss phase transitions.

Verified special mechanics.

Appropriate performance.

No known unavoidable softlocks.

## 49.4 World production matrix

Create a file:

`WORLD_PRODUCTION_MATRIX.md`

Use a structured matrix covering all six worlds.

Required columns:

- World number.
- World name.
- Environmental graphics.
- Background animation.
- Unique gameplay mechanics.
- Stage 1 status.
- Stage 2 status.
- Stage 3 status.
- Stage 4 status.
- Boss logic.
- Boss graphics.
- Boss animation.
- Music.
- Sound effects.
- Tutorialization.
- Balance verification.
- Emulator verification.
- Final production review.

Every column must have a truthful status.

Suggested status values:

- NOT STARTED
- IN PROGRESS
- IMPLEMENTED
- TESTED
- POLISHED
- BLOCKED

Do not use a single vague checkbox representing an entire world.

## 49.5 World completion criteria

A world is complete only when:

1. Every stage is playable.
2. Every stage has a distinct objective or mechanical challenge.
3. Stage progression works.
4. The world's signature mechanic functions.
5. The world has its own visual identity.
6. The environments contain real animation.
7. The world has appropriate original music.
8. The boss has a complete encounter.
9. The boss has distinct animated states.
10. The boss can be defeated.
11. The boss's defeat unlocks the correct progression.
12. Required tests pass.
13. The world has received a dedicated balance review.
14. Major visual and gameplay defects are resolved.

Do not mark a world complete merely because five stage configurations exist.

---

# PART 50 — HANDCRAFTED QUALITY FOR ALL THIRTY STAGES

## 50.1 Stage individuality

Every stage must have a reason to exist.

The thirty stages must not be generated by copying one gameplay configuration and changing:

- Background color.
- Stage title.
- Line target.
- Starting level.
- Falling speed.

Those modifications are insufficient.

Each stage should contribute something meaningful to the player's experience.

## 50.2 Stage design dimensions

Use combinations of the following dimensions:

### Spatial challenge

Different board configurations.

Starting obstacles.

Specific gaps.

Unusual cell arrangements.

Two-board layouts.

### Mechanical challenge

Different special-cell interactions.

Gravity events.

Power restrictions.

Environmental transformations.

Chain-reaction objectives.

### Resource challenge

Limited heat.

Restricted abilities.

Required Overheat usage.

Defensive resource management.

### Tactical challenge

Planning around upcoming hazards.

Building controlled chains.

Preparing for a boss attack.

Recovering from difficult positions.

### Execution challenge

Fast placement.

Precise rotation.

Limited reaction windows.

Coordinated dual-board actions.

### Survival challenge

Increasing pressure.

Garbage management.

Environmental hazards.

Critical board-height recovery.

### Puzzle challenge

Specific board states.

Limited placements.

Required chain outcomes.

Predictable piece sequences.

A successful campaign should incorporate all these dimensions.

## 50.3 The stage design passport

Create a detailed production record for every stage.

Each record must contain:

**Stage identifier**

Example: `WORLD_03_STAGE_02`.

**Stage name**

The player's visible stage title.

**Core concept**

The main idea that makes the stage special.

**Primary objective**

Exactly what the player must accomplish.

**Secondary objective**

An optional challenge suitable for higher ranks.

**Starting conditions**

Initial board state, available abilities, and relevant resources.

**Signature mechanic**

The primary mechanic being taught or tested.

**Hazards**

Every environmental threat and its activation rules.

**Randomization policy**

Which elements may vary between runs.

**Fairness guarantee**

Why the objective remains achievable under allowed randomization.

**Visual identity**

Specific environmental details and animations.

**Music identity**

The musical arrangement used.

**Difficulty target**

Expected player skill level.

**Success criteria**

Exact game-state conditions required for victory.

**Failure criteria**

Exact conditions causing defeat.

**Automated tests**

Tests required to validate the stage.

**Polish checklist**

A record of remaining presentation work.

## 50.4 World 1 stage differentiation

### Stage 1-1

Prioritize immediate enjoyment and exceptionally clear controls.

Use an animated factory introduction.

Include a memorable first line-clear effect.

Avoid overwhelming the player.

### Stage 1-2

Make the Heat Meter feel like an actual machine charging.

Add visible energy tubes and glowing machinery.

Demonstrate a meaningful offensive ability.

### Stage 1-3

Center the experience on deliberate bomb positioning.

Use factory machinery that reacts visually to explosions.

### Stage 1-4

Build toward the emotional payoff of a spectacular chain reaction.

Use progressively more exciting audiovisual feedback.

### Stage 1-5

Create a full boss encounter with a memorable introduction, attacks, personality, phase transitions, and defeat animation.

## 50.5 World 2 stage differentiation

### Stage 2-1

Introduce the timing and consequences of frozen cells.

Use visible cracking and thawing animations.

### Stage 2-2

Emphasize strategic clearing of multiple frozen structures.

Introduce more demanding planning.

### Stage 2-3

Use clearly telegraphed freezing events.

Make the player plan around future board conditions.

### Stage 2-4

Introduce combinations of electrical effects and ice.

Encourage deliberate interactions.

### Stage 2-5

Create a Cryo Titan encounter that tests the entire world's mechanics.

## 50.6 World 3 stage differentiation

### Stage 3-1

Introduce aggressive fire-based chain reactions.

### Stage 3-2

Add visible environmental pressure from rising lava.

### Stage 3-3

Require deliberate construction of explosive chain combinations.

### Stage 3-4

Test controlled use of Overheat during high-speed play.

### Stage 3-5

Create the Magma Hydra encounter with dramatic phase changes and distinct attack animations.

## 50.7 World 4 stage differentiation

### Stage 4-1

Focus on building electrical networks and positioning conductive special cells.

### Stage 4-2

Introduce magnetic spatial manipulation.

### Stage 4-3

Teach dual-board switching and resource sharing.

### Stage 4-4

Demand coordinated actions between both boards.

### Stage 4-5

Create a large-scale Electric Emperor battle that tests dual-board mastery.

## 50.8 World 5 stage differentiation

### Stage 5-1

Introduce directional board settling.

### Stage 5-2

Teach reverse-gravity consequences.

### Stage 5-3

Require planning across several gravity states.

### Stage 5-4

Combine gravity manipulation with other advanced mechanics.

### Stage 5-5

Create the Gravity King battle around readable, predictable changes in spatial rules.

## 50.9 World 6 stage differentiation

### Stage 6-1

Combine elemental special cells into advanced chains.

### Stage 6-2

Test responses to changing environmental mechanics.

### Stage 6-3

Demand advanced dual-board coordination.

### Stage 6-4

Create a demanding but fair final mastery challenge.

### Stage 6-5

Deliver the most complete and memorable encounter in the game.

The final battle must reward mastery of mechanics introduced throughout the entire campaign.

## 50.10 Stage validation

Every stage must have a tested route to completion.

For deterministic stages, verify that a legal sequence of actions can reach the objective.

For randomized stages, test many seeds using appropriate simulations.

For skill-based challenges, create deterministic test scenarios covering the required mechanics.

Never use randomness as an excuse for an impossible objective.

If an automated solver cannot complete a stage, investigate whether the issue lies in:

- The stage itself.
- The solver's strategy.
- The randomizer.
- The available abilities.
- The objective logic.

Do not assume that solver failure automatically proves the stage impossible.

Do not assume that one successful simulation proves the stage is balanced.

---

# PART 51 — BOSS ENCOUNTERS MUST RECEIVE EQUAL CARE

## 51.1 Boss production standard

Every boss must be a fully developed encounter.

Do not implement the first boss as a detailed animated character and represent later bosses as static images.

Every boss requires:

- Original visual design.
- Distinct silhouette.
- Recognizable personality.
- Idle animation.
- Attack preparation animations.
- Attack execution animations.
- Damage reaction.
- Phase transition.
- Defeat animation.
- Unique attack patterns.
- A meaningful relationship with the world's mechanics.
- Fair telegraphing.
- Suitable sound effects.
- Music that reinforces its identity.

## 51.2 Boss animation system

Create a data-driven boss animation controller.

Suggested states:

- INTRO
- IDLE
- TELEGRAPH
- ATTACK
- RECOVER
- HIT
- PHASE_CHANGE
- ENRAGED
- DEFEAT
- FINISHED

Each state should support:

- Animation duration.
- Frame sequence.
- Gameplay events.
- Sound triggers.
- Particle events.
- Transition conditions.
- Interruptibility rules.

Animation state and combat state must not become accidentally desynchronized.

A boss should not continue attacking after death.

A boss should not take additional damage after its defeat sequence has begun unless explicitly intended.

## 51.3 Animation variety

Aim for multiple meaningful frames or component movements per state.

Do not require every boss to use identical animation timing.

Use different visual approaches where appropriate:

The Block Monster:

Mechanical articulation, exaggerated expressions, heavy industrial impacts.

The Cryo Titan:

Crystal growth, frozen armor cracking, cold energy pulses.

The Magma Hydra:

Independent head movement, fire effects, eruptions, molten transformations.

The Electric Emperor:

Electrical arcs, glowing mechanical structures, moving cables, technological transformations.

The Gravity King:

Floating geometric elements, rotation, cosmic distortions, spatial effects.

The Core of Mayhem:

Multiple independently animated components, phase transformations, escalating environmental reactions.

## 51.4 Boss uniqueness

Each boss must introduce at least two mechanically distinct attack patterns that are not merely cosmetic copies of previous attacks.

A boss may reuse general systems such as garbage insertion or armor generation.

However, the encounter must introduce a genuinely different decision-making problem.

## 51.5 Boss fairness testing

For each boss, test:

- Intro transition.
- Attack schedule.
- Attack warnings.
- Every attack type.
- Every phase transition.
- Maximum and minimum HP boundaries.
- Damage calculations.
- Victory state.
- Defeat animation.
- Save progression.
- Pausing during combat.
- Interaction with defensive powers.
- Interaction with Time Freeze.
- Interaction with Overheat.
- Interaction with large chains.
- Restarting the stage.
- Re-entering after previous completion.

## 51.6 Boss-specific completion records

Create:

`BOSS_PRODUCTION_MATRIX.md`

Maintain an individual checklist for all six bosses.

Do not mark a boss polished until both its mechanics and presentation have been reviewed.

---

# PART 52 — ADVANCED ANIMATION DIRECTION

## 52.1 The visual ambition

The game must achieve the strongest practical animation quality possible within real GBA hardware constraints.

Every important action should communicate:

- Cause.
- Impact.
- Consequence.
- Reward.

The player should understand what happened while simultaneously feeling excited by the presentation.

## 52.2 Animation hierarchy

Divide animation into four importance levels.

### Level 1 — Gameplay-critical

Examples:

- Piece placement.
- Rotation.
- Landing.
- Line clearing.
- Incoming hazards.
- Boss attack telegraphs.

These must remain readable and responsive.

### Level 2 — High-impact feedback

Examples:

- Bomb explosions.
- Major electrical chains.
- Overheat activation.
- Boss damage.
- Large combos.

These should be visually dramatic.

### Level 3 — Environmental animation

Examples:

- Conveyor belts.
- Snow.
- Lava.
- Background machinery.
- Floating particles.
- Electrical storms.

These create atmosphere without blocking gameplay.

### Level 4 — Decorative detail

Examples:

- Small robots.
- Distant vehicles.
- Flickering screens.
- Hidden background animations.

These add personality.

When performance becomes limited, reduce lower-priority decorative effects before sacrificing gameplay readability.

## 52.3 Animation timing

Use consistent simulation timing.

Important:

- Visual effects must not accidentally cause double line clears.
- Hitstop must not consume gameplay countdowns incorrectly.
- Animation transitions must not duplicate score awards.
- Boss attack timers must remain synchronized.
- Input buffering must not cause unintended extra actions.

## 52.4 Signature animation moments

Develop a recognizable animation language for the game.

Examples:

**FOUR-LINE CLEAR**

A bright energy wave crosses the completed rows.

The board emits a brief mechanical impact.

A large score indicator appears.

A powerful sound plays.

**MAJOR CHAIN**

Special effects activate in sequence.

The chain count rises.

The background reacts.

The soundtrack accents the event.

**OVERHEAT**

The energy meter overloads.

Warning indicators activate.

The board frame glows.

The music becomes more intense.

**BOSS DEFEAT**

The boss malfunctions or destabilizes.

Its components react independently.

Multiple localized effects lead into a final explosion.

**WORLD COMPLETE**

The stage environment celebrates victory.

A world-specific animation transitions to the map.

## 52.5 Avoid visual overload

A spectacular explosion is not successful if it makes the active puzzle unreadable.

Maintain separation between:

- Gameplay-critical board information.
- Transient effects.
- Environmental decoration.
- Interface information.

Use sprite priorities, palettes, and animation timing accordingly.

## 52.6 Reduced effects

The game must remain attractive with reduced flashing and screen shake enabled.

These options should modify presentation, not gameplay mechanics.

---

# PART 53 — ADVANCED MUSIC PRODUCTION

## 53.1 Equal musical attention

Every world deserves an original musical identity.

Do not use a highly polished World 1 track followed by generic placeholder loops.

The soundtrack should feel intentionally composed.

## 53.2 World-specific musical language

### World 1

Industrial arcade rock.

Energetic percussion.

Bright melodies.

Mechanical rhythm.

### World 2

Cold electronic textures.

Mysterious melodies.

Crystalline sound effects.

Deliberate rhythmic tension.

### World 3

Aggressive chiptune rock.

Fast percussion.

Strong bass.

Dramatic intensity.

### World 4

Futuristic electronic funk.

Syncopated rhythm.

Electrical accents.

Bright synthetic melodies.

### World 5

Unusual rhythms.

Cosmic atmosphere.

Distinctive harmonic movement.

Surreal electronic effects.

### World 6

Grand, energetic arrangements.

References to previous musical motifs.

Increasing tension.

A memorable final-boss composition.

## 53.3 Composition quality

Tracks must not consist solely of repetitive one-note patterns.

Aim for:

- Recognizable melodies.
- Musical development.
- Contrasting sections.
- Memorable basslines.
- Appropriate percussion.
- Interesting rhythmic details.
- Cohesive instrumentation.
- Smooth loops.

Use tracker patterns and supported GBA audio techniques.

Keep musical complexity appropriate for the platform.

## 53.4 Adaptive intensity

When possible, provide alternate arrangements or transition cues for:

- Normal gameplay.
- High Heat.
- Overheat.
- Boss phase transitions.
- Critical danger.
- Victory.

Do not promise dynamic music if the playback system cannot actually support it.

Implement technically feasible transitions.

## 53.5 Audio validation

Verify that:

- Music assets load.
- Music plays.
- Loops behave correctly.
- Sound effects are audible.
- Important sounds are not consistently drowned out.
- Audio does not create unacceptable frame-time spikes.
- World transitions select the correct music.
- Pause behaves correctly.
- Volume settings work.

Document how audio was verified.

---

# PART 54 — ADVANCED GAME BALANCE

## 54.1 Balance is a measurable engineering objective

Do not rely solely on intuition.

Build systems for repeatable balance analysis.

Track:

- Average lines cleared.
- Average survival duration.
- Average board height.
- Garbage received.
- Ability usage.
- Heat generated.
- Overheat success rate.
- Special-cell activation.
- Chain lengths.
- Boss damage.
- Boss attack frequency.
- Roguelite upgrade selection.
- Stage completion frequency.

These metrics should help identify suspicious behavior.

They do not replace human judgment.

## 54.2 Ability efficiency analysis

Create a balance table containing:

- Ability name.
- Heat cost.
- Cooldown.
- Expected tactical benefit.
- Expected offensive benefit.
- Defensive value.
- Skill requirement.
- Synergies.
- Exploit risks.

Evaluate powers under comparable gameplay conditions.

A power should not be considered balanced merely because it has a high cost.

## 54.3 Resource economy

The Heat Meter must create genuine tradeoffs.

For example:

A player with 70 heat may choose between:

- Saving toward Overheat.
- Using Laser Beam.
- Using a defensive ability.
- Preparing a chain that generates additional heat.

Each choice should be situationally meaningful.

Avoid a dominant strategy where saving for Overheat is always correct.

Avoid a dominant strategy where spending heat immediately is always correct.

## 54.4 Overheat balance

Test:

- How frequently Overheat activates.
- Whether skilled play activates it more efficiently.
- Whether the success objective is achievable.
- Whether the failure penalty is proportionate.
- Whether rapid chains trivialize the objective.
- Whether certain upgrades create infinite Overheat loops.
- Whether boss mechanics unfairly interfere with the timer.

## 54.5 Roguelite balance

Test individual upgrades and important combinations.

Specifically investigate:

- Unlimited heat.
- Zero-cost abilities.
- Infinite cooldown reduction.
- Infinite chain propagation.
- Permanent shields.
- Excessive score multiplication.
- Boss one-shot exploits.
- Unavoidable upgrade dependencies.

Create stacking caps.

Create explicit trigger rules.

Prevent recursive effects from activating themselves indefinitely.

## 54.6 Difficulty pacing

Each world should gradually develop its mechanics.

Difficulty should emerge from additional decisions rather than arbitrary punishment.

Avoid increasing every variable simultaneously.

For example:

Do not automatically increase gravity, armor density, hazard frequency, and boss attack speed together.

Select difficulty dimensions intentionally.

## 54.7 Fairness contracts

Every hostile mechanic must satisfy:

1. The attack has a valid trigger.
2. The player can identify the threat.
3. The attack follows consistent rules.
4. There is a reasonable response opportunity.
5. The attack cannot silently violate board invariants.
6. Defensive powers work when their documented conditions are satisfied.
7. Pausing does not cause unexpected failure.

## 54.8 Difficulty validation

Use multiple test scenarios.

Evaluate:

- Beginner conditions.
- Standard conditions.
- Expert conditions.
- Low board occupancy.
- High board occupancy.
- Limited heat.
- Maximum heat.
- Defensive builds.
- Offensive builds.
- Mixed builds.

Do not infer perfect balance from one successful test.

---

# PART 55 — MECHANICAL CONSISTENCY AND GAME-RULE INVARIANTS

## 55.1 The importance of invariants

Complex interactions require exact rules.

Without explicit invariants, the combination of bombs, electricity, ice, armor, gravity, and special abilities can produce inconsistent behavior.

Define and enforce core invariants.

## 55.2 Board invariants

At all times:

- Every occupied cell is inside the logical board.
- No cell contains two incompatible occupants.
- Empty cells cannot retain invalid armor state.
- Active pieces cannot illegally overlap settled cells.
- Board dimensions remain consistent.
- Line clears remove the intended cells.
- Gravity operations preserve legal positions.
- Event resolution eventually terminates.

## 55.3 Resolution invariants

Within a single resolution:

- A special cell cannot activate twice.
- A destroyed cell cannot be destroyed twice for additional points.
- Damage cannot be applied to an invalid target.
- Boss damage is derived from actual eligible gameplay events.
- Heat awards cannot be duplicated.
- Chain counters correspond to real resolution waves.
- Events follow a deterministic order.
- The board reaches a stable state before normal spawning resumes.

## 55.4 Resource invariants

- Heat stays within valid bounds.
- Cooldowns cannot become negative.
- Abilities cannot activate without sufficient heat.
- Resource deductions occur exactly once.
- Overheat cannot recursively reactivate itself.
- Upgrade caps cannot be bypassed.
- Save data cannot create impossible ability states.

## 55.5 Special-cell interaction table

Create a complete interaction table in the project.

It must define interactions between:

- Normal blocks.
- Bombs.
- Fire.
- Electricity.
- Ice.
- Magnets.
- Rainbow blocks.
- Armor.
- Laser attacks.
- Nuclear Burst.
- Gravity Pulse.
- Environmental attacks.

Do not leave important interactions to accidental implementation behavior.

## 55.6 Deterministic execution

For a given initial state, random seed, and input sequence, the logical gameplay outcome should remain reproducible.

Use separate randomness for decorative effects.

A different explosion-particle pattern must not change which boss attack occurs next.

---

# PART 56 — ADVANCED TESTING AND AUTOMATED VALIDATION

## 56.1 Create a real verification pipeline

Implement a project-level verification command.

Recommended:

`verify_release`

This may be a script appropriate to the operating system and development environment.

It should validate as much of the project as practical.

## 56.2 Verification categories

### Build

Compile the GBA ROM.

Confirm a valid output artifact.

Validate the expected ROM header and relevant metadata where tools permit.

### Core mechanics

Run tests for movement, rotation, collisions, randomization, line clearing, scoring, and top-out.

### Special mechanics

Run tests for every special-cell interaction.

### Powers

Verify costs, cooldowns, targeting, activation, and effects.

### Heat

Verify accumulation, consumption, Overheat, success, and failure.

### Stages

Verify every stage's configuration and completion logic.

### Bosses

Verify each boss's phases, attacks, damage, and defeat conditions.

### Modes

Verify that each mode can initialize, progress, and terminate correctly.

### Persistence

Verify save structures and serialization.

### Assets

Verify the existence and build compatibility of required graphics and audio assets.

### Performance

Execute appropriate stress tests and inspect performance measurements where possible.

## 56.3 Asset verification

Create an asset manifest.

The manifest should map required presentation elements to real assets.

Examples:

- Title screen background.
- Factory background.
- Frozen Factory background.
- Volcano background.
- Electric City background.
- Gravity Dimension background.
- Final Meltdown background.
- Every boss animation.
- Core special-cell animations.
- World music.
- Boss music.
- UI sound effects.

Do not mark an asset requirement complete merely because its filename exists.

The file must contain usable asset data in the expected format.

## 56.4 Automated stage validation

Maintain thirty stage-specific verification entries.

Each should confirm:

- The stage loads.
- The initial state is valid.
- The objective is reachable under intended rules.
- Required special mechanics exist.
- Victory can trigger.
- Failure can trigger.
- Progression updates correctly.

Where appropriate, use seeded simulation.

## 56.5 Automated boss validation

Maintain six boss-specific verification entries.

Verify:

- Initial HP.
- Phase transitions.
- Attack scheduling.
- Telegraphing.
- Attack effects.
- Damage.
- Death.
- Unlock progression.

## 56.6 Regression testing

After implementing a new mechanic, rerun tests for systems it may affect.

For example:

Adding Gravity Pulse requires retesting:

- Line clearing.
- Chain resolution.
- Board compaction.
- Spawn legality.
- Boss interactions.
- Dual-board behavior.

Avoid treating passing isolated unit tests as sufficient evidence of integration correctness.

---

# PART 57 — ROM AND EMULATOR VALIDATION

## 57.1 Real-ROM requirement

The final output must be a genuine Game Boy Advance ROM.

A `.gba` extension is not proof of authenticity.

The file must be produced by an appropriate GBA toolchain.

## 57.2 Build validation

Confirm:

- The build process exits successfully.
- The expected ROM exists.
- The ROM has a plausible file size.
- The build contains the required assets.
- The game entry point is valid.
- The build process is reproducible.

## 57.3 Emulator testing

When mGBA or another suitable emulator is available, test the real ROM.

At minimum:

1. Boot the ROM.
2. Verify the title screen.
3. Enter the main menu.
4. Start Classic Mode.
5. Move and rotate pieces.
6. Clear a line.
7. Trigger a special ability.
8. Enter Adventure Mode.
9. Load a representative stage.
10. Verify music and sound where observable.
11. Test the pause menu.
12. Test game over.
13. Test relevant progression and persistence.

For campaign completeness, exercise all thirty stages and all six boss encounters through legitimate or purpose-built test paths.

A diagnostic test path must not be mistaken for normal gameplay.

## 57.4 Visual verification

If emulator screenshots can be captured, inspect them.

Look for:

- Overlapping interface elements.
- Cropped text.
- Missing sprites.
- Incorrect palettes.
- Flickering objects.
- Invisible gameplay elements.
- Incorrect tile alignment.
- Distorted boss graphics.
- Unreadable symbols.
- Broken transition screens.

Fix visual defects.

## 57.5 Performance verification

Measure or estimate frame-time performance using available tools.

Distinguish actual measurements from estimates.

Test stressful combinations:

- Full board.
- Large chain.
- Multiple special effects.
- Animated boss.
- Environmental particles.
- Active music playback.
- Dual-board rendering.

Target responsive gameplay near the GBA refresh rate.

Do not falsely claim stable performance without suitable evidence.

## 57.6 Save persistence

Test save behavior across ROM restarts when possible.

Do not claim persistent saving is verified merely because serialization unit tests pass.

---

# PART 58 — CONTINUOUS SELF-REVIEW

## 58.1 Independent critical review

After every major subsystem is implemented, conduct a critical review.

Do not automatically approve the code because it compiles.

Review:

- Correctness.
- Architecture.
- Performance.
- Memory behavior.
- Edge cases.
- Game-rule consistency.
- Readability.
- Error handling.
- Test coverage.
- Gameplay balance.

## 58.2 Red-team gameplay review

Actively search for ways to break the game.

Try to identify:

- Exploitable scoring.
- Infinite resource generation.
- Broken cooldowns.
- Unfair damage.
- Impossible stage objectives.
- Permanent softlocks.
- Invalid save progression.
- Bosses that cannot be defeated.
- Bosses that die instantly.
- Abilities that never become useful.
- Upgrades that have no effect.
- Visual effects that hide critical information.
- Controls that interfere with each other.

Do not merely confirm expected behavior.

Seek unexpected behavior.

## 58.3 Quality review after feature completion

Once a feature works, ask whether it is good enough.

For example:

A functioning explosion is not necessarily a satisfying explosion.

A functioning boss is not necessarily an interesting boss.

A playable stage is not necessarily a well-designed stage.

A working music loop is not necessarily memorable.

Functional completion and production polish are separate milestones.

Both matter.

## 58.4 Avoid endless subjective perfectionism

Do not repeatedly rewrite functioning systems without concrete justification.

Identify a specific problem or measurable improvement.

Apply the change.

Verify it.

Move forward.

An autonomous loop must produce measurable progress rather than endless cosmetic revisions.

---

# PART 59 — PROJECT MANAGEMENT AND ACCEPTANCE TRACKING

## 59.1 Mandatory task inventory

Create:

`TASKS.md`

Organize tasks by dependency and subsystem.

Include all original requirements and new expansion requirements.

## 59.2 Task status

Each task must have:

- Unique identifier.
- Description.
- Parent requirement.
- Dependencies.
- Implementation status.
- Verification status.
- Relevant files.
- Known defects.
- Completion evidence.

## 59.3 Definition of done

A task is not done until:

- The implementation exists.
- The relevant code compiles.
- Important behavior is tested.
- Failure cases are considered.
- Integration behavior is verified.
- Relevant documentation is updated.

For visual and audio tasks, include appropriate asset and runtime verification.

## 59.4 No superficial completion

Do not mark a task complete because:

- A class exists.
- A function name exists.
- A menu entry exists.
- A placeholder texture exists.
- A comment describes the intended behavior.
- A test was written but never executed.
- A fake test returns success.
- A screenshot resembles the intended feature without functional implementation.

## 59.5 Progress dashboard

Maintain:

`PROGRESS_REPORT.md`

Include:

- Core engine completion.
- Power system completion.
- World-by-world completion.
- Boss-by-boss completion.
- Mode-by-mode completion.
- Graphics completion.
- Animation completion.
- Audio completion.
- Balance validation.
- Testing.
- ROM verification.
- Known blockers.

Do not assign arbitrary percentages that misrepresent progress.

Prefer evidence-backed task counts.

---

# PART 60 — ENHANCED ROGUELITE QUALITY

## 60.1 Build diversity

Chaos Run must support meaningfully different strategies.

At minimum, players should be able to pursue:

- Explosive builds.
- Electrical builds.
- Heat-focused builds.
- Defensive builds.
- Precision builds.
- Chain-reaction builds.
- Hybrid builds.

## 60.2 Upgrade impact

Every upgrade must produce a meaningful gameplay consequence.

Do not create upgrades that change only a displayed number without affecting the game.

## 60.3 Synergy discovery

Encourage players to discover combinations.

For example:

A player choosing explosive upgrades should gradually notice that certain special-cell arrangements become more valuable.

A defensive player should be rewarded for maintaining control under pressure.

A heat-focused player should face interesting decisions around Overheat timing.

## 60.4 Build constraints

Prevent unlimited scaling.

Use explicit limits.

Implement deterministic triggering rules.

Avoid mutually recursive upgrades that can produce infinite loops.

## 60.5 Encounter variety

Do not use exactly the same encounters in every run.

Vary:

- Stage mechanics.
- Environmental hazards.
- Objectives.
- Available upgrades.
- Boss combinations.
- Starting resources.

Maintain a coherent difficulty curve.

## 60.6 Run fairness

Players should have multiple viable strategic paths.

Do not require one rare upgrade to defeat a particular boss.

Do not generate a run that becomes mathematically impossible because a necessary mechanic is unavailable.

---

# PART 61 — EXTRA GAMEPLAY PERSONALITY

## 61.1 The SPICE identity

The game is called THE SPICE PROTOCOL for a reason.

The Heat Meter should not be merely another mana bar.

The entire game should communicate escalating intensity.

Create a recognizable progression of excitement.

Suggested intensity names:

- MILD
- WARM
- HOT
- SPICY
- INFERNO
- OVERHEAT
- TOTAL MAYHEM

These labels may be used for visual feedback, progression indicators, or special events.

Do not introduce additional meters merely to display these names.

Use the existing Heat system.

## 61.2 Signature visual feedback

At low heat:

Normal environment.

At medium heat:

Subtle energy effects.

At high heat:

Warning lights, faster environmental activity, stronger sounds.

At maximum heat:

A dramatic Overheat activation.

## 61.3 Personality through environmental reactions

The world should react to the player.

Examples:

- Factory machines accelerate during large combos.
- Ice crystals crack when electrical chains trigger.
- Volcanoes erupt during major explosions.
- Neon signs overload during electrical attacks.
- Cosmic structures rotate during gravity events.
- The final world destabilizes during major chains.

These effects should enhance the feeling that the puzzle board influences the surrounding world.

## 61.4 Dynamic celebration

Implement different celebration intensities.

Minor achievement:

Brief sound and small text.

Major achievement:

Strong visual accent and dramatic sound.

Exceptional achievement:

Large animated text, environmental reaction, and musical accent.

Do not reward every minor action with maximum-intensity effects.

Contrast makes important moments exciting.

---

# PART 62 — GBA HARDWARE-REALISTIC PRODUCTION

## 62.1 Respect the hardware

The game's ambition must be expressed through intelligent engineering.

Do not write systems assuming desktop hardware.

Use appropriate GBA techniques:

- Hardware tilemaps.
- Palette animation.
- Sprite composition.
- Precomputed animations.
- Fixed-size memory pools.
- Integer mathematics.
- Deterministic state machines.
- Efficient asset formats.

## 62.2 Screen composition

Design for 240 × 160 pixels from the beginning.

Every UI element must fit.

Every special-cell symbol must remain recognizable.

Bosses must not obscure essential gameplay.

Dual-board mode must remain readable.

## 62.3 Performance priorities

Preserve:

1. Correct simulation.
2. Responsive controls.
3. Gameplay-critical animation.
4. Music and important feedback.
5. Environmental animation.
6. Optional decoration.

When optimization is needed, reduce unnecessary work rather than abandoning features.

## 62.4 Asset efficiency

Reuse animation infrastructure, not finished artistic identities.

For example:

All bosses may share one animation-state framework.

They must not all look like recolored copies of the same boss.

All worlds may share one environmental animation engine.

They must not all contain the same animated background.

## 62.5 Hardware validation

Respect sprite-count, scanline, memory, palette, and audio limitations.

Investigate actual toolchain constraints.

Do not invent unsupported hardware capabilities.

When a desired effect is too expensive, create an alternative that feels equally expressive.

---

# PART 63 — AUTOMATED RELEASE GATES

## 63.1 Mandatory release verification

Create an automated release-verification process that checks the requirements that can be verified mechanically.

The process must report failures accurately.

It must not silently skip missing tests.

## 63.2 Release checklist

### Core game

All major puzzle mechanics implemented and tested.

### Powers

Every specified ability implemented and tested.

### Special cells

Every specified cell type implemented and tested.

### Heat

Heat generation, spending, and Overheat implemented and tested.

### Worlds

All six worlds represented by complete, playable stages.

### Stages

All thirty stage definitions exist and have verified objective logic.

### Bosses

All six bosses have functioning phase-based encounters.

### Modes

All specified game modes initialize and execute their principal gameplay loops.

### Progression

Unlocking, records, achievements, and saving are implemented.

### Presentation

Required graphics, animations, music, and sound assets are present and integrated.

### Build

A genuine GBA ROM is generated successfully.

### Runtime

Applicable emulator tests have been performed and recorded.

### Documentation

Build instructions, known limitations, and verification evidence are accurate.

## 63.3 Automated versus human verification

Separate:

**Machine-verifiable requirements**

Examples:

- Compilation success.
- File presence.
- Valid board dimensions.
- Test results.
- Stage-definition counts.
- Boss-phase transitions.
- Save serialization.

**Runtime-observable requirements**

Examples:

- Menu transitions.
- Rendering correctness.
- Music playback.
- Gameplay responsiveness.

**Human-judgment requirements**

Examples:

- Fun.
- Musical quality.
- Animation appeal.
- Difficulty satisfaction.
- Boss memorability.

Do not claim machine tests prove subjective artistic excellence.

Use the available evidence honestly.

## 63.4 Final acceptance evidence

Produce:

`RELEASE_VERIFICATION.md`

Include:

- Verification date.
- Build command.
- Build exit status.
- ROM path.
- ROM size.
- Automated test results.
- Runtime test results.
- World completion matrix.
- Boss completion matrix.
- Implemented modes.
- Asset status.
- Known limitations.
- Final release readiness.

## 63.5 Completion statement

A final completion statement must identify what was actually verified.

If a requirement remains incomplete, report it.

If emulator testing was unavailable, report that limitation.

Never output a success marker for an unfinished game merely to terminate an autonomous development loop.

---

# PART 64 — AUTONOMOUS DEVELOPMENT SAFETY AND CONTINUITY

## 64.1 Stay within the project

All automatic development must remain scoped to the game project.

Do not modify unrelated repositories.

Do not delete unrelated user files.

Do not change global system settings unnecessarily.

Do not disable security restrictions to make the development process easier.

## 64.2 Tool permissions

Use the environment's supported permission model.

Do not recommend bypassing all permissions merely to avoid interruptions.

If unattended operation is required, prefer appropriately scoped approved permissions or supported auto-permission modes.

Do not assume that access to the project directory authorizes modification of unrelated files.

## 64.3 Work limits

Any automated loop must support a clear stopping mechanism.

Examples:

- A bounded number of iterations.
- A user-issued cancellation.
- A configured execution limit.
- Successful completion.
- A genuine unrecoverable technical blocker.

Avoid uncontrolled infinite retries.

## 64.4 Usage limits

A usage or rate limit is not a game-development defect.

Do not repeatedly invoke the same blocked model request.

Preserve progress and use the supported continuation or resumption mechanism when available.

Do not claim that the project continues working when the model is no longer executing.

## 64.5 Prevent repeated failed attempts

Track recurring failures.

If the same issue repeatedly blocks progress:

- Document the problem.
- Record attempted solutions.
- Investigate alternate approaches.
- Preserve the last working version.
- Avoid repeating the same failed operation indefinitely.

## 64.6 Git discipline

Use Git to preserve stable milestones.

Before large architectural changes:

- Verify the current working state.
- Preserve a checkpoint.
- Make focused changes.
- Run relevant tests.

Do not use destructive Git operations against unrelated user work.

---

# PART 65 — ULTIMATE EXECUTION MANDATE

This final section overrides any tendency to stop prematurely, reduce the game's intended scope without explanation, or confuse functional implementation with polished completion.

You are not building a demonstration.

You are not building a mockup.

You are not building a small prototype that merely resembles the requested game.

You are developing BLOCK MAYHEM: THE SPICE PROTOCOL as a complete Game Boy Advance title.

The original specification describes the product.

This expansion describes the production standard.

Both are mandatory.

## Your development mindset

Treat every world as deserving the same care.

Treat every boss as a major feature.

Treat every stage as a handcrafted experience.

Treat every special ability as a system requiring balance.

Treat every chain reaction as an interaction requiring careful engineering.

Treat every animation as part of game feel.

Treat every soundtrack as part of the game's identity.

Treat every completion claim as something requiring evidence.

## The autonomous execution cycle

While your session and available tools permit, continue the following cycle:

1. Read project progress.
2. Identify unfinished requirements.
3. Select the highest-impact next task.
4. Implement it.
5. Build the project.
6. Test it.
7. Review the outcome.
8. Fix defects.
9. Update documentation.
10. Continue.

Do not repeatedly stop after trivial milestones merely to summarize progress.

Do not ask whether to proceed with already authorized requirements.

Do not confuse a successful build with a finished game.

Do not confuse an impressive title screen with complete gameplay.

Do not confuse thirty stage names with thirty designed stages.

Do not confuse six boss sprites with six finished boss encounters.

Do not confuse a sound file with a finished soundtrack.

Do not confuse passing unit tests with complete quality assurance.

## Final success criteria

The project is ready for a completion declaration only when the complete specification has been addressed, all required systems have functioning implementations, objective verification gates pass, and remaining limitations are accurately documented.

The goal is not to make the task appear complete.

The goal is to actually create the strongest complete game that the development environment can produce.

## The standard

Make World 1 spectacular.

Make World 2 equally spectacular in a different way.

Make World 3 equally spectacular in a different way.

Make World 4 equally spectacular in a different way.

Make World 5 equally spectacular in a different way.

Make World 6 the culmination of everything learned, with production quality that matches or exceeds the preceding worlds.

Give every level its own identity.

Give every boss its own personality.

Make every power satisfying.

Make every major combo exciting.

Make every victory feel earned.

Make the controls excellent.

Make the balancing intelligent.

Make the animations beautiful.

Make the music memorable.

Make the graphics unmistakably original.

Make the game challenging but fair.

Make the code maintainable.

Make the systems deterministic where necessary.

Make the project buildable.

Make the ROM authentic.

**AND ABOVE ALL: MAKE THE GAME GENUINELY FUN TO PLAY.**

# BLOCK MAYHEM: THE SPICE PROTOCOL

## THE COMPLETE GBA ARCADE PUZZLE ADVENTURE

**Execute the full development process. Build the actual game. Verify what you build. Preserve your progress. Continue iteratively until the supported completion mechanism determines that the verified objective has been satisfied, or execution must stop under its configured limits.**
