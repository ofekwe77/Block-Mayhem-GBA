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
| stages.* | all 30 Adventure stages x 3 bots x 4 seeds: every stage won, 0 invalid states, 0 timeouts, hazards and boss attacks fire (`tests/build/stage_report.md`) |
| run.* | 31-upgrade pool well formed; maxed build respects every cap through the real session rules; same seed = same run; 300 seeded runs: offers always eligible and distinct, no repeated encounters or bosses, post-boss rare card; every encounter template at every act and every run boss won by bots; 6 full bot runs (`tests/build/run_report.md`) |

Latest run (2026-10-15): **68 passed, 0 failed.**

Every simulated frame also calls `session::validate()` (board/heat/cooldown/boss invariants).

## 2. Emulator tests (real ROM in the mGBA core)

`tools/harness/bm_harness.c` links the static libmgba built by `tools/harness/build_libmgba.sh`.
Scenario scripts (`tests/emu/*.txt`) press buttons, wait for RAM test-interface fields, capture
PNG screenshots and measure audio RMS. Diagnostic mailbox commands (start a mode with autoplay, add
heat, set boss HP...) are a test path only and cannot be reached from the game UI.

| Scenario | Result (2026-10-15) |
|----------|---------------------|
| smoke.txt — boot, title music, menu, Classic input, pause, Arcade autoplay | PASS (error_code 0) |
| adventure.txt — world map, stage card, stage 1-1, all six bosses, phase changes, defeat sequence | PASS |
| worlds.txt — first stage of every world + dual stage 4-3, music RMS per world | PASS (RMS 4244..6041) |
| chaos_run.txt — run map, encounters under autoplay, results, upgrade choice, act 1 boss, act 2 map | PASS (4 cleared, 4 upgrades, error_code 0) |

Screenshots from the latest runs: `screenshots/tmp/`.

## Not yet tested
See TASKS.md entries without `TESTED` status (save persistence across restarts, stress/performance
runs, Boss Rush, options / records / achievements screens).
