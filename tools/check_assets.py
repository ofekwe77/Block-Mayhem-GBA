"""Asset verification (PART 56.3): maps every required presentation element to its real asset files and
checks that each file holds usable data in the format the build expects, and that the game code uses it.

  graphics/*.bmp + .json : BMP header / bit depth / palette / pixel data, the JSON type the build tools
                           expect, valid GBA sprite / background dimensions, frame count
  audio/*.xm             : FastTracker II module header, channels, patterns, instruments
  audio/*.wav            : RIFF PCM header, mono 8/16-bit, sample data present and audible (peak level)
  integration            : the asset's item is referenced by the ROM source (src/, include/)
  board tiles            : the named tiles a requirement needs exist in include/bm_tile_ids.h

Every graphics / audio file must belong to a requirement (no orphan or missing assets).

Usage: python3 tools/check_assets.py [--write-manifest ASSET_MANIFEST.md]   (exit 0 = all PASS)
"""
import glob
import json
import os
import re
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

WORLDS = ["CLASSIC CHAOS factory", "FROZEN FACTORY", "VOLCANO MADNESS", "ELECTRIC CITY", "GRAVITY DIMENSION",
          "THE FINAL MELTDOWN"]
BOSSES = [
    ("BOSS-1", "The Block Monster (World 1)", ["boss1_body", "boss1_eye", "boss1_mouth", "boss1_arm"]),
    ("BOSS-2", "The Cryo Titan (World 2)", ["boss2_body", "boss2_head", "boss2_crystal", "boss2_breath"]),
    ("BOSS-3", "The Magma Hydra (World 3)", ["boss3_base", "boss3_neck", "boss3_head", "boss3_fire"]),
    ("BOSS-4", "The Electric Emperor (World 4)", ["boss4_body", "boss4_face", "boss4_orb", "boss4_bolt"]),
    ("BOSS-5", "The Gravity King (World 5)", ["boss5_body", "boss5_head", "boss5_ring", "boss5_cube"]),
    ("BOSS-6", "The Core of Mayhem (World 6)", ["boss6_eye", "boss6_iris", "boss6_heart", "boss6_ring",
                                                 "boss6_tentacle", "boss6_wing"]),
]


def sfx(*names):
    return ["audio/sfx_%s.wav" % n for n in names]


REQUIREMENTS = [
    ("BG-TITLE", "Title screen background: logo, subtitle band, gears, falling-block silhouettes", "26.1, 46",
     ["graphics/bg_title.bmp"], []),
    ("BG-MENU", "Menu / options backdrop", "26", ["graphics/bg_menu.bmp"], []),
    ("BG-MAP", "World map", "47", ["graphics/bg_worldmap.bmp"], []),
] + [
    ("BG-W%d" % w, "World %d background, far + near parallax layers: %s" % (w, WORLDS[w - 1]), "15-20, 44",
     ["graphics/bg_w%d_far.bmp" % w, "graphics/bg_w%d_near.bmp" % w], []) for w in range(1, 7)
] + [
    ("BOARD", "Board tiles and palette: well, blocks, garbage, stone, frames, panels, heat meter, boss HP bar",
     "23, 24, 43", ["graphics/board_tiles.bmp", "graphics/board_palette.bmp"],
     ["WELL", "BLOCK", "GHOST", "GARBAGE", "STONE", "FRAME_L", "FRAME_R", "FRAME_B", "PANEL", "METER_0", "METER_8",
      "HP_0", "HP_8", "HEATH_0", "HEATH_8"]),
    ("SPECIAL-CELLS", "Special-cell tiles (bomb, fire, spark, magnet, rainbow) with ice, armour, charged, disabled and "
     "frost / armour mark variants; animated through palette indices 7/8/10/13 (bm_board_view.cpp _animate_palette)",
     "5-10, 24", ["graphics/board_tiles.bmp"],
     ["BOMB", "FIRE", "SPARK", "MAGNET", "RAINBOW", "ICE", "ICE_BOMB", "ICE_FIRE", "ICE_SPARK", "ICE_MAGNET",
      "ICE_RAINBOW", "ARMOR1", "ARMOR2", "CHARGED", "OFF_BOMB", "OFF_FIRE", "OFF_SPARK", "OFF_MAGNET", "OFF_RAINBOW",
      "FROST_MARK", "ARMOR_MARK"]),
    ("BOARD-FX", "Line-clear flash, hard-drop trail, dual-board energy bridge, SHADOW STACK outline, warnings",
     "12.8, 24.3, 27.3", ["graphics/board_tiles.bmp"],
     ["FLASH1", "FLASH2", "FLASH3", "TRAIL1", "TRAIL2", "TRAIL3", "BRIDGE_0", "BRIDGE_3", "BRIDGE_L", "BRIDGE_R",
      "BRIDGE_HOT", "BRIDGE_GLOW", "SHADOW", "WARN_ROW", "TARGET", "GAP_MARK", "GARBAGE_GHOST"]),
] + [
    (bid, "%s: animated parts (idle, telegraph, attack, hit, enraged frames)" % name, "19, 51",
     ["graphics/%s.bmp" % p for p in parts], []) for bid, name, parts in BOSSES
] + [
    ("FX-PARTICLES", "Particles: sparks, smoke, embers, snow, crystals, debris, stars, electric, shards, dust, hearts",
     "24, 34", ["graphics/spr_particles.bmp"], []),
    ("FX-EXPLOSION", "Bomb / boss explosions", "24", ["graphics/spr_explosion.bmp"], []),
    ("FX-BEAM", "Laser beam", "11", ["graphics/spr_beam.bmp"], []),
    ("UI-ICONS", "Ability icons, objective icons, cursor, arrows", "25, 26", [
        "graphics/spr_icons.bmp", "graphics/spr_goals.bmp", "graphics/spr_cursor.bmp", "graphics/spr_arrows.bmp"], []),
    ("UI-UPGRADES", "Chaos Run upgrade icons (31)", "21", ["graphics/spr_upgrades.bmp"], []),
    ("SECRETS", "Secret events and easter egg: maintenance robot, dizzy boss face, lightning bolt, chili pepper",
     "35", ["graphics/spr_robot.bmp", "graphics/spr_dizzy.bmp", "graphics/spr_bolt.bmp", "graphics/spr_chili.bmp"],
     []),
    ("FONTS", "Fonts (small, big, tiny) and their colour palettes", "26",
     ["graphics/font_small.bmp", "graphics/font_big.bmp", "graphics/font_tiny.bmp"] +
     ["graphics/pal_text_%s.bmp" % c for c in ("white", "yellow", "red", "cyan", "green", "orange", "gray", "violet")] +
     ["graphics/pal_big_%s.bmp" % c for c in ("fire", "ice", "gold", "toxic", "neon", "steel", "blood")], []),
    ("MUS-TITLE", "Title theme", "45", ["audio/mus_title.xm"], []),
    ("MUS-MENU", "Menu music", "45", ["audio/mus_menu.xm"], []),
] + [
    ("MUS-W%d" % w, "World %d music and its Overheat arrangement" % w, "45", [
        "audio/mus_w%d.xm" % w, "audio/mus_w%d_hot.xm" % w], []) for w in range(1, 7)
] + [
    ("MUS-BOSS", "Boss battle music and its Overheat arrangement", "45", ["audio/mus_boss.xm", "audio/mus_boss_hot.xm"],
     []),
    ("MUS-FINAL", "Final boss music and its Overheat arrangement", "45", ["audio/mus_final.xm",
                                                                          "audio/mus_final_hot.xm"], []),
    ("MUS-CLASSIC", "Classic / arcade music", "45", ["audio/mus_classic.xm"], []),
    ("MUS-JINGLES", "Victory, game over and ending themes", "45", [
        "audio/mus_victory.xm", "audio/mus_gameover.xm", "audio/mus_ending.xm"], []),
    ("SFX-UI", "Menu and interface sounds", "45", sfx("menu_move", "menu_ok", "menu_back", "denied", "pause", "unlock"),
     []),
    ("SFX-PIECE", "Piece control sounds", "45", sfx("move", "rotate", "hold", "hard_drop", "lock"), []),
    ("SFX-CLEAR", "Clears, chains, level and result sounds", "45", sfx(
        "clear1", "clear2", "clear3", "clear4", "chain", "chain_big", "level_up", "stage_clear", "game_over"), []),
    ("SFX-SPECIAL", "Special-cell sounds", "45", sfx("bomb", "fire", "spark", "ice", "armor", "magnet", "rainbow"), []),
    ("SFX-ABILITY", "Ability sounds", "45", sfx("laser", "freeze", "gravity", "nuke", "shield", "vent", "shift"), []),
    ("SFX-HEAT", "Heat, Overheat, danger and garbage sounds", "45", sfx(
        "overheat_start", "overheat_success", "overheat_fail", "danger", "garbage"), []),
    ("SFX-BOSS", "Boss sounds", "45", sfx("boss_hit", "boss_attack", "boss_charge", "boss_defeat"), []),
    ("SFX-DUAL", "Dual-board switch", "45", sfx("switch_board"), []),
]

SPRITE_SIZES = {8, 16, 32, 64}
SPRITE_SHAPES = {(8, 8), (16, 16), (32, 32), (64, 64), (16, 8), (32, 8), (32, 16), (64, 32), (8, 16), (8, 32),
                 (16, 32), (32, 64)}


def read_bmp(path):
    d = open(path, "rb").read()
    if d[:2] != b"BM":
        raise ValueError("not a BMP")
    off = struct.unpack("<I", d[10:14])[0]
    w, h = struct.unpack("<ii", d[18:26])
    bpp = struct.unpack("<H", d[28:30])[0]
    comp = struct.unpack("<I", d[30:34])[0]
    ncol = struct.unpack("<I", d[46:50])[0] or (1 << bpp if bpp <= 8 else 0)
    if comp != 0:
        raise ValueError("compressed BMP")
    if bpp not in (4, 8):
        raise ValueError("%d bpp (expected 4 or 8)" % bpp)
    stride = ((w * bpp + 7) // 8 + 3) & ~3
    if len(d) < off + stride * abs(h):
        raise ValueError("pixel data truncated")
    return {"w": w, "h": abs(h), "bpp": bpp, "colors": ncol}


def check_graphic(path):
    bmp = read_bmp(path)
    meta_path = path[:-4] + ".json"
    if not os.path.exists(meta_path):
        raise ValueError("missing %s" % os.path.basename(meta_path))
    meta = json.load(open(meta_path, encoding="utf-8"))
    t = meta.get("type")
    w, h = bmp["w"], bmp["h"]
    if t == "sprite":
        fh = int(meta.get("height", h))
        if (w, fh) not in SPRITE_SHAPES:
            raise ValueError("sprite %dx%d is not a GBA sprite shape" % (w, fh))
        if h % fh:
            raise ValueError("height %d is not a multiple of the frame height %d" % (h, fh))
        return "sprite %dx%d, %d frame%s, %d bpp" % (w, fh, h // fh, "s" if h // fh != 1 else "", bmp["bpp"])
    if t == "regular_bg":
        if w % 256 or h % 256 or w > 512 or h > 512:
            raise ValueError("background %dx%d is not 256/512 square-tile sized" % (w, h))
        return "background %dx%d, %d bpp, %d colours" % (w, h, bmp["bpp"], bmp["colors"])
    if t == "regular_bg_tiles":
        if w % 8 or h % 8:
            raise ValueError("tiles not 8-pixel aligned")
        return "%d tiles, %d bpp" % ((w // 8) * (h // 8), bmp["bpp"])
    if t in ("bg_palette", "sprite_palette"):
        count = int(meta.get("colors_count", 16))
        return "%s, %d colours" % (t.replace("_", " "), count)
    raise ValueError("unknown asset type %r" % t)


def check_xm(path):
    d = open(path, "rb").read()
    if d[:17] != b"Extended Module: ":
        raise ValueError("not an XM module")
    if d[37] != 0x1A:
        raise ValueError("bad XM header")
    version = struct.unpack("<H", d[58:60])[0]
    if version != 0x0104:
        raise ValueError("XM version 0x%04X" % version)
    header_size = struct.unpack("<I", d[60:64])[0]
    length, restart, channels, patterns, instruments = struct.unpack("<HHHHH", d[64:74])
    if not (1 <= channels <= 32 and patterns >= 1 and instruments >= 1 and length >= 1):
        raise ValueError("empty or invalid module (%d ch, %d patterns, %d instruments)" % (channels, patterns,
                                                                                         instruments))
    if restart >= length:
        raise ValueError("restart position outside the order list")
    if len(d) < 60 + header_size:
        raise ValueError("truncated module")
    return "XM, %d channels, %d orders, %d patterns, %d instruments" % (channels, length, patterns, instruments)


def check_wav(path):
    d = open(path, "rb").read()
    if d[:4] != b"RIFF" or d[8:12] != b"WAVE":
        raise ValueError("not a RIFF WAVE file")
    pos = 12
    fmt = None
    data = None
    while pos + 8 <= len(d):
        cid = d[pos:pos + 4]
        size = struct.unpack("<I", d[pos + 4:pos + 8])[0]
        body = d[pos + 8:pos + 8 + size]
        if cid == b"fmt ":
            fmt = struct.unpack("<HHIIHH", body[:16])
        elif cid == b"data":
            data = body
        pos += 8 + size + (size & 1)
    if not fmt or data is None:
        raise ValueError("missing fmt or data chunk")
    tag, channels, rate, _, _, bits = fmt
    if tag != 1 or channels != 1 or bits not in (8, 16):
        raise ValueError("not mono 8/16-bit PCM")
    if bits == 8:
        peak = max(abs(b - 128) for b in data) if data else 0
        full = 127
    else:
        samples = struct.unpack("<%dh" % (len(data) // 2), data[:len(data) // 2 * 2])
        peak = max(abs(s) for s in samples) if samples else 0
        full = 32767
    n = len(data) // (bits // 8)
    if n == 0 or peak < full // 20:
        raise ValueError("silent or empty sound")
    return "PCM %d Hz %d-bit, %d ms, peak %d%%" % (rate, bits, n * 1000 // rate, peak * 100 // full)


def source_text():
    parts = []
    for pattern in ("src/*.cpp", "src/*.h", "include/*.h"):
        for p in glob.glob(os.path.join(ROOT, pattern)):
            parts.append(open(p, encoding="utf-8", errors="replace").read())
    return "\n".join(parts)


def integration(rel, src):
    name = os.path.splitext(os.path.basename(rel))[0]
    if rel.startswith("audio/"):
        kind = "music_items" if name.startswith("mus_") else "sound_items"
        return re.search(r"\b%s::%s\b" % (kind, re.escape(name)), src) is not None
    return re.search(r"\b%s\b" % re.escape(name), src) is not None


def main():
    write_md = None
    if "--write-manifest" in sys.argv:
        write_md = sys.argv[sys.argv.index("--write-manifest") + 1]
    src = source_text()
    tile_names = set(re.findall(r"constexpr int (\w+) =", open(os.path.join(ROOT, "include/bm_tile_ids.h")).read()))
    rows = []
    failures = 0
    covered = set()
    for rid, what, spec, files, tiles in REQUIREMENTS:
        details = []
        problems = []
        for rel in files:
            covered.add(rel)
            path = os.path.join(ROOT, rel)
            if not os.path.exists(path):
                problems.append("%s missing" % rel)
                continue
            try:
                if rel.endswith(".bmp"):
                    desc = check_graphic(path)
                elif rel.endswith(".xm"):
                    desc = check_xm(path)
                else:
                    desc = check_wav(path)
            except (ValueError, struct.error, IndexError) as e:
                problems.append("%s: %s" % (rel, e))
                continue
            if not integration(rel, src):
                problems.append("%s is not used by the ROM source" % rel)
            details.append((rel, desc))
        for t in tiles:
            if t.lower() not in tile_names:
                problems.append("tile %s missing from bm_tile_ids.h" % t)
        ok = not problems
        failures += 0 if ok else 1
        rows.append((rid, what, spec, details, tiles, problems))
        print("%-13s %s  %s" % (rid, "PASS" if ok else "FAIL", "; ".join(problems)))

    # Every asset file the build packs must belong to a requirement.
    orphans = []
    for pattern in ("graphics/*.bmp", "audio/*.xm", "audio/*.wav"):
        for p in sorted(glob.glob(os.path.join(ROOT, pattern))):
            rel = os.path.relpath(p, ROOT).replace(os.sep, "/")
            if rel not in covered:
                orphans.append(rel)
    if orphans:
        failures += 1
        print("ORPHANS       FAIL  " + ", ".join(orphans))

    if write_md:
        out = []
        out.append("# BLOCK MAYHEM — Asset Manifest (PART 56.3)\n")
        out.append("Generated by `tools/check_assets.py --write-manifest`. Every requirement maps to real files that were")
        out.append("parsed and validated (format, GBA dimensions, audible sound data) and whose items are referenced by")
        out.append("the ROM source; named board tiles are checked against `include/bm_tile_ids.h`. All art and audio is")
        out.append("original and generated by the scripts in `tools/gfx/` and `tools/audio/` (see CREDITS.md).\n")
        out.append("| Requirement | What | Spec | Files (validated format) | Status |")
        out.append("|---|---|---|---|---|")
        for rid, what, spec, details, tiles, problems in rows:
            files = "<br>".join("`%s` — %s" % (rel, desc) for rel, desc in details)
            if tiles:
                files += "<br>tiles: " + ", ".join(t.lower() for t in tiles)
            status = "PASS" if not problems else "FAIL: " + "; ".join(problems)
            out.append("| %s | %s | %s | %s | %s |" % (rid, what, spec, files, status))
        out.append("")
        out.append("Orphan assets (packed but not mapped to a requirement): %s" % (", ".join(orphans) if orphans else "none"))
        out.append("")
        with open(os.path.join(ROOT, write_md), "w", newline="\n", encoding="utf-8") as f:
            f.write("\n".join(out))
    print("asset check: %d requirements, %s" % (len(rows), "PASS" if not failures else "%d FAILED" % failures))
    return 0 if not failures else 1


if __name__ == "__main__":
    sys.exit(main())
