# BLOCK MAYHEM: THE SPICE PROTOCOL

An original falling-block action puzzle game for the Nintendo Game Boy Advance.
Output: `BlockMayhem.gba` (a genuine GBA ROM built with devkitARM + Butano).

Status: feature complete — six worlds, thirty stages, six multi-phase bosses, six game modes. What is verified
and how is in `RELEASE_VERIFICATION.md` (summary) and `docs/verify_report.md` (latest automated run); the task
inventory is `TASKS.md`. Design: `GAME_DESIGN.md`, rules table `INTERACTIONS.md`, balance `BALANCE.md`,
per-stage records `STAGE_PASSPORTS.md`, world / boss status `WORLD_PRODUCTION_MATRIX.md` /
`BOSS_PRODUCTION_MATRIX.md`, assets `ASSET_MANIFEST.md`, tests `TESTING.md`.

## Installing dependencies (Windows)

1. **MSYS2** (no admin rights needed): download `msys2-base-x86_64-latest.sfx.exe` from
   https://github.com/msys2/msys2-installer/releases and extract it to `C:\msys64`
   (`msys2-base-x86_64-latest.sfx.exe -y -oC:\`). Start `C:\msys64\usr\bin\bash.exe -l` once.
2. **devkitPro packages** inside MSYS2 (adds the devkitPro pacman repositories; see
   https://devkitpro.org/wiki/devkitPro_pacman):
   ```sh
   pacman-key --recv-keys BC26F752D25B92CE272E0F44F7FD5492264BB9D0 --keyserver keyserver.ubuntu.com
   pacman-key --lsign-key BC26F752D25B92CE272E0F44F7FD5492264BB9D0
   # install https://pkg.devkitpro.org/devkitpro-keyring.pkg.tar.zst with pacman -U, then add to /etc/pacman.conf:
   #   [dkp-libs]    Server = https://pkg.devkitpro.org/packages
   #   [dkp-windows] Server = https://pkg.devkitpro.org/packages/windows/$arch/
   pacman -Sy gba-dev make python mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja
   ```
3. **Butano 21.9.0**: `git clone --depth 1 --branch 21.9.0 https://github.com/GValiente/butano.git C:\msys64\opt\butano`
   (or set `LIBBUTANO=/path/to/butano/butano`).
4. **Python 3** is used by the asset generators (the MSYS2 `python` package is enough).

## Building

```powershell
.\build.ps1           # regenerate graphics + audio, then build BlockMayhem.gba
.\build.ps1 rom       # build the ROM only
.\build.ps1 tests     # run the host-side unit tests and simulations
.\build.ps1 verify    # full release verification: assets, ROM + header, unit tests, every emulator scenario
.\build.ps1 docs      # regenerate STAGE_PASSPORTS.md from the stage data
```
The project folder contains spaces / non-ASCII characters, which GNU make cannot handle, so
`build.ps1` maps it to drive `M:` with `subst` (per session, no admin rights; override with `BM_DRIVE`).
From an MSYS2 shell already inside an ASCII path you can run `tools/bm.sh rom` (or plain `make -j8`).

## Running in mGBA

Open `BlockMayhem.gba` in mGBA (https://mgba.io). The save file `BlockMayhem.sav` is created next to the ROM.

Automated emulator tests use a headless harness built on the real mGBA core:
```powershell
.\build.ps1 harness
.\build.ps1 emu --rom BlockMayhem.gba --script tests/emu/smoke.txt --save tmp.sav --out screenshots/tmp
```

## Controls

| Button | Action |
|--------|--------|
| D-Pad Left / Right | Move |
| D-Pad Down | Soft drop |
| D-Pad Up | Hard drop |
| A / B | Rotate clockwise / counter-clockwise |
| L | Hold |
| R | Activate equipped ability (while aiming: A fire, B cancel) |
| SELECT | Cycle abilities (Dual Board: switch board) |
| START | Pause |

## Game modes

- **Adventure** — six worlds of five stages, each world ending with a multi-phase boss; stage cards, ranks
  C..S+, world introductions and an ending.
- **Classic** — traditional falling-block game: seven-bag, hold, 3 previews, ghost, speed-up, high scores.
- **Chaos Arcade** — endless score attack with special blocks, the Heat meter, Overheat, three abilities
  chosen on the loadout screen and telegraphed environmental events.
- **Chaos Run** — a roguelite of three acts (twelve fights) with 31 upgrades and synergies.
- **Dual Board** — two boards, one heat meter; switch with SELECT, forced switch every 4 locks, CROSS OVER combos
  across the energy bridge.
- **Boss Rush** — every defeated boss in a row (unlocked by the World 3 boss).

Classic, Chaos Arcade and Dual Board accept challenge modifiers unlocked by the Adventure (CHALLENGES page of
the setup screen). OPTIONS holds difficulty, ghost piece, screen shake, flashes, volumes, CUSTOMIZE (block,
frame, title and particle styles — CONTRAST is a colour-blind friendly block theme), sound test and credits.

## Saving

Progress, records and settings are saved to cartridge SRAM (32 KB) automatically after each game.
Two slots with a magic header, version, size and CRC-32 are written alternately; on boot the newest valid
slot is used, and unreadable data falls back to defaults instead of crashing.

## Troubleshooting

- `M: is mapped to another folder` — set `$env:BM_DRIVE="N:"` and rerun.
- `MSYS2 not found` — install as above or set `$env:BM_MSYS2`.
- Graphics look wrong after editing generators — run `.\build.ps1` (not `rom`) to regenerate assets.
- Build artefacts live in `build/`; `.\build.ps1 clean` removes them.
