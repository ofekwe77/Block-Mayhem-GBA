# Testing

Two layers of automated verification. Results below are real runs (not projections).

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
| abilities.* | denial without heat, exact deduction, cooldown in locked pieces, timeout without cost, vent |
| bots.* | 5 strategies x 3 seeds: no invalid state in 2-minute games |

Latest run (2026-10-15): **61 passed, 0 failed.**

Every simulated frame also calls `session::validate()` (board/heat/cooldown/boss invariants).

## 2. Emulator tests (real ROM in the mGBA core)

`tools/harness/bm_harness.c` links the static libmgba built by `tools/harness/build_libmgba.sh`.
Scenario scripts (`tests/emu/*.txt`) press buttons, wait for RAM test-interface fields, capture
PNG screenshots and measure audio RMS. Diagnostic mailbox commands (start a mode with autoplay, add
heat, set boss HP...) are a test path only and cannot be reached from the game UI.

| Scenario | Result (2026-10-15) |
|----------|---------------------|
| smoke.txt — boot, title music, menu, Classic input, pause, Arcade autoplay 1200 frames | PASS (20 rows, chain 3, error_code 0, title audio RMS 3956) |

Screenshots from the latest run: `screenshots/tmp/` (title, menu, classic, pause, arcade).

## Not yet tested
See TASKS.md entries without `TESTED` status (stages, bosses, Chaos Run, save persistence across restarts,
stress/performance runs).
