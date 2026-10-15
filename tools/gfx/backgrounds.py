"""World / title backgrounds (original procedural pixel art).

Every world has a FAR layer and a NEAR layer (256x256, 4bpp, 2 palette banks
each) for parallax. Animation comes from hardware scrolling and palette
cycling (gear teeth, conveyor stripes, lamps, neon, lava, snow light...).
Cycle specs are exported to include/bm_bg_cycles.h for the runtime.
"""
import math
import os
import random

from painter import Layer, Palette

CYCLES = {}         # item name -> list of (bank, first, count, period_frames, mode)
ROTATE, BLINK, FLICKER, PULSE = 0, 1, 2, 3


def bank(*colors):
    assert len(colors) == 16
    return list(colors)


def save_layer(name, layer, gfx_dir, write_bmp, write_json, cycles=None):
    px = layer.finalize()
    pal = layer.pal.flat()
    write_bmp(os.path.join(gfx_dir, name + ".bmp"), layer.w, layer.h, px, pal, bpp=8)
    write_json(os.path.join(gfx_dir, name + ".json"),
               {"type": "regular_bg", "bpp_mode": "bpp_4_manual", "colors_count": len(pal),
                "flipped_tiles_reduction": False})
    CYCLES[name] = cycles or []


# ====================================================================== WORLD 1 - CLASSIC CHAOS (factory)

# Neon pictograms for signs and billboards. Backdrops never carry letters: partially hidden words
# scrolling behind the HUD read as stray interface text.
PICTOGRAMS = {
    "bolt": ["...##", "..##.", ".##..", "#####", "..##.", ".##..", "##...", "#...."],
    "chili": ["....#", "...#.", ".###.", "####.", "###..", "##...", "#....", ""],
    "tblock": ["", "#####", "#####", ".###.", ".###.", "", "", ""],
    "heart": [".#.#.", "#####", "#####", ".###.", "..#..", "", "", ""],
    "star": ["..#..", "..#..", "#####", ".###.", ".#.#.", "#...#", "", ""],
    "lblock": ["#....", "#....", "#....", "###..", "###..", "", "", ""],
    "arrow": ["..#..", "...#.", "#####", "...#.", "..#..", "", "", ""],
    "skull": [".###.", "#####", "#.#.#", "#####", ".#.#.", "", "", ""],
}


def pictogram(lay, x, y, kind, col, scale=1):
    for ry, row in enumerate(PICTOGRAMS[kind]):
        for rx, v in enumerate(row):
            if v == "#":
                lay.rect(x + rx * scale, y + ry * scale, x + rx * scale + scale, y + ry * scale + scale, col)


def pictogram_row(lay, x, y, kinds, col, gap=3, scale=1):
    for k in kinds:
        pictogram(lay, x, y, k, col, scale)
        x += 5 * scale + gap


def world1(gfx_dir, write_bmp, write_json):
    far_pal = Palette([
        bank((0, 0, 0), (8, 10, 28), (14, 18, 44), (22, 28, 62), (34, 42, 84), (48, 56, 96), (70, 80, 120),
             (100, 112, 150), (150, 160, 196), (60, 36, 40), (96, 56, 48), (40, 64, 120),
             (88, 96, 140), (120, 128, 170), (160, 170, 210), (210, 216, 240)),
        bank((0, 0, 0), (8, 10, 28), (20, 22, 40), (40, 48, 70), (70, 80, 110), (120, 130, 160),
             (232, 192, 40), (24, 20, 20), (90, 20, 24), (255, 64, 48), (255, 80, 200), (64, 232, 255),
             (120, 40, 110), (30, 100, 130), (255, 140, 32), (255, 255, 255)),
    ])
    for i, n in enumerate(["_", "navy0", "navy1", "navy2", "blue", "steel0", "steel1", "steel2", "steel3",
                           "rust0", "rust1", "window", "t0", "t1", "t2", "t3"]):
        far_pal.name(n, 0, i)
    for i, n in enumerate(["_", "dk0", "dk1", "pipe0", "pipe1", "pipe2", "warn", "hazblk", "lampoff", "lamp",
                           "pink", "cyan", "pinkdim", "cyandim", "orange", "white"]):
        far_pal.name(n, 1, i)

    far = Layer(far_pal)
    far.vgradient(0, 0, 256, 256, ["navy0", "navy1", "navy2", "blue", "navy2"])
    # Wall panels with seams and rivets.
    for px0 in range(0, 256, 64):
        far.rect(px0, 0, px0 + 1, 256, "navy0")
        far.rect(px0 + 1, 0, px0 + 2, 256, "steel0")
        for y in range(6, 256, 12):
            far.set(px0 + 4, y, "steel2")
            far.set(px0 + 60, y, "steel1")
    for py0 in (48, 96, 176):
        far.rect(0, py0, 256, py0 + 1, "navy0")
        far.rect(0, py0 + 1, 256, py0 + 2, "steel0")
    # Lit windows (control rooms).
    rnd = random.Random(11)
    for wx in range(8, 256, 24):
        for wy in (56, 70):
            if rnd.random() < 0.7:
                far.rect(wx, wy, wx + 10, wy + 7, "navy0")
                far.rect(wx + 1, wy + 1, wx + 9, wy + 6, "window")
                far.rect(wx + 1, wy + 1, wx + 4, wy + 2, "steel2")
    # Big gears (teeth use the cycling colours t0..t3 => they spin).
    far.gear(36, 112, 22, 30, 16, "steel0", ["t0", "t1", "t2", "t3"], "steel2", "navy0")
    far.gear(78, 140, 13, 18, 10, "steel0", ["t3", "t2", "t1", "t0"], "steel2", "navy0")
    far.gear(212, 118, 18, 25, 14, "steel0", ["t1", "t2", "t3", "t0"], "steel2", "navy0")
    # Pipes.
    far.rect(0, 14, 256, 22, "pipe1")
    far.rect(0, 14, 256, 15, "pipe2")
    far.rect(0, 21, 256, 22, "pipe0")
    for jx in range(0, 256, 40):
        far.rect(jx, 12, jx + 6, 24, "pipe2")
        far.rect(jx, 23, jx + 6, 24, "pipe0")
    for vx in (118, 172):
        far.rect(vx, 22, vx + 8, 150, "pipe1")
        far.rect(vx, 22, vx + 2, 150, "pipe2")
        far.rect(vx + 7, 22, vx + 8, 150, "pipe0")
        for jy in range(40, 150, 36):
            far.rect(vx - 1, jy, vx + 9, jy + 4, "pipe2")
    # Neon arcade sign (flickering).
    far.rect(150, 30, 238, 50, "dk0")
    far.rect(151, 31, 237, 49, "pinkdim")
    far.rect(152, 32, 236, 48, "dk0")
    pictogram_row(far, 157, 35, ["tblock", "chili", "bolt", "chili", "tblock", "heart"], "pink", gap=9)
    pictogram_row(far, 178, 52, ["arrow", "arrow", "arrow", "arrow", "arrow"], "cyan", gap=4)
    # Catwalk railing.
    for x in range(0, 256):
        far.set(x, 156, "steel2")
        far.set(x, 157, "steel0")
        if x % 12 == 0:
            far.rect(x, 157, x + 2, 168, "steel1")
    far.rect(0, 166, 256, 170, "steel1")
    # Hazard stripe band with red warning lamps.
    for x in range(256):
        for y in range(184, 192):
            far.set(x, y, "warn" if ((x + y) // 4) % 2 == 0 else "hazblk")
    for lx in range(12, 256, 32):
        far.disc(lx, 180, 3, "lampoff")
        far.disc(lx, 180, 2, "lamp")
    far_cycles = [(0, 12, 4, 6, ROTATE), (1, 9, 1, 40, BLINK), (1, 10, 1, 0, FLICKER), (1, 11, 1, 0, FLICKER)]
    save_layer("bg_w1_far", far, gfx_dir, write_bmp, write_json, far_cycles)

    near_pal = Palette([
        bank((0, 0, 0), (6, 6, 18), (30, 34, 52), (60, 66, 92), (100, 108, 136), (160, 168, 196),
             (20, 20, 26), (44, 44, 52), (232, 192, 40), (40, 36, 30), (232, 192, 40), (40, 36, 30),
             (64, 232, 96), (24, 80, 40), (232, 48, 40), (255, 255, 255)),
        bank((0, 0, 0), (6, 6, 18), (40, 200, 232), (24, 120, 150), (248, 208, 40), (150, 120, 20),
             (176, 72, 232), (100, 40, 140), (72, 216, 80), (40, 120, 44), (240, 64, 72), (140, 32, 40),
             (248, 144, 32), (150, 80, 16), (200, 210, 230), (255, 255, 255)),
    ])
    for i, n in enumerate(["_", "blk", "m0", "m1", "m2", "m3", "belt0", "belt1", "s0", "s1", "s2", "s3",
                           "green", "greendim", "red", "white"]):
        near_pal.name(n, 0, i)
    near = Layer(near_pal)
    # Conveyor belt across the bottom.
    for x in range(256):
        near.set(x, 140, "blk")
        for y in range(141, 145):
            near.set(x, y, ["s0", "s1", "s2", "s3"][(x // 4) % 4])
        for y in range(145, 156):
            near.set(x, y, "belt1" if (y - 145) % 5 else "belt0")
        near.set(x, 156, "blk")
    for rx in range(6, 256, 16):
        near.disc(rx, 152, 4, "m1")
        near.disc(rx, 152, 2, "m3")
        near.set(rx, 152, "blk")
    for x in range(256):
        for y in range(157, 168):
            near.set(x, y, "m0" if (x // 8 + y // 4) % 2 else "m1")
    # Crates (coloured blocks) riding the belt.
    colors = [(2, 3), (4, 5), (6, 7), (8, 9), (10, 11), (12, 13)]
    for k, cx in enumerate(range(18, 256, 46)):
        hi, lo = colors[k % len(colors)]
        for (ox, oy) in ((0, 0), (8, 0), (4, -8)) if k % 2 else ((0, 0),):
            x0, y0 = cx + ox, 132 + oy
            near.rect(x0, y0, x0 + 8, y0 + 8, 16 + lo)
            near.rect(x0, y0, x0 + 7, y0 + 7, 16 + hi)
            near.rect(x0 + 1, y0 + 1, x0 + 3, y0 + 3, 16 + 15)
            near.rect(x0 + 7, y0, x0 + 8, y0 + 8, 16 + 1)
            near.rect(x0, y0 + 7, x0 + 8, y0 + 8, 16 + 1)
    # Press machines with status lights.
    for mx in (60, 188):
        near.rect(mx, 60, mx + 28, 66, "m2")
        near.rect(mx + 10, 66, mx + 18, 112, "m1")
        near.rect(mx + 11, 66, mx + 13, 112, "m3")
        near.rect(mx + 4, 112, mx + 24, 124, "m2")
        near.rect(mx + 4, 112, mx + 24, 114, "m3")
        near.rect(mx + 4, 122, mx + 24, 124, "m0")
        near.disc(mx + 8, 63, 2, "green")
        near.disc(mx + 20, 63, 2, "red")
    near_cycles = [(0, 8, 4, 4, ROTATE), (0, 12, 1, 30, BLINK), (0, 14, 1, 45, BLINK)]
    save_layer("bg_w1_near", near, gfx_dir, write_bmp, write_json, near_cycles)


# ====================================================================== TITLE

def title(gfx_dir, write_bmp, write_json):
    pal = Palette([
        bank((0, 0, 0), (6, 4, 18), (14, 8, 34), (26, 14, 56), (44, 22, 84), (70, 34, 110), (100, 52, 140),
             (140, 80, 170), (190, 120, 200), (30, 10, 20), (80, 16, 30), (160, 24, 40), (230, 50, 50),
             (255, 120, 60), (255, 200, 90), (255, 255, 220)),
        bank((0, 0, 0), (6, 4, 18), (20, 22, 40), (40, 48, 70), (70, 80, 110), (110, 120, 150),
             (160, 170, 200), (220, 228, 248), (88, 96, 140), (120, 128, 170), (160, 170, 210), (210, 216, 240),
             (255, 80, 200), (64, 232, 255), (255, 140, 32), (255, 255, 255)),
        bank((0, 0, 0), (12, 2, 6), (60, 8, 12), (120, 16, 16), (190, 36, 20), (232, 72, 24), (255, 120, 32),
             (255, 168, 48), (255, 208, 80), (255, 236, 140), (255, 252, 220), (40, 10, 30), (90, 24, 50),
             (255, 255, 255), (232, 48, 32), (255, 240, 120)),
        bank((0, 0, 0), (4, 12, 24), (8, 30, 50), (16, 60, 90), (30, 100, 140), (60, 160, 200),
             (120, 220, 255), (200, 248, 255), (255, 255, 255), (40, 200, 232), (248, 208, 40),
             (176, 72, 232), (72, 216, 80), (240, 64, 72), (56, 104, 240), (248, 144, 32)),
    ])
    for i in range(16):
        pal.name("a%d" % i, 0, i)
        pal.name("m%d" % i, 1, i)
        pal.name("f%d" % i, 2, i)
        pal.name("c%d" % i, 3, i)
    lay = Layer(pal)
    lay.vgradient(0, 0, 256, 176, ["a1", "a2", "a3", "a4", "a5", "a3"])
    lay.rect(0, 176, 256, 256, "a1")
    # Distant stars.
    rnd = random.Random(5)
    for _ in range(70):
        x, y = rnd.randrange(256), rnd.randrange(160)
        lay.set(x, y, rnd.choice(["a7", "a8", "a15"]))
    # Giant mechanical eye behind the logo (iris colours cycle -> it pulses).
    cx, cy = 120, 74
    lay.disc(cx, cy, 52, "m2")
    lay.ring(cx, cy, 52, 3, "m5")
    lay.ring(cx, cy, 48, 2, "m3")
    for k in range(28):
        a = k * math.pi / 14
        lay.line(cx + math.cos(a) * 43, cy + math.sin(a) * 43, cx + math.cos(a) * 51, cy + math.sin(a) * 51, "m4")
    lay.disc(cx, cy, 40, "a10")
    for r, c in ((38, "a11"), (32, "a12"), (25, "a13"), (18, "a14"), (11, "a15")):
        lay.disc(cx, cy, r, c)
    lay.disc(cx, cy, 7, "m1")
    lay.disc(cx - 10, cy - 12, 5, "a15")
    # Gears in the corners (teeth cycle -> they spin).
    lay.gear(14, 22, 18, 25, 12, "m3", ["m8", "m9", "m10", "m11"], "m5", "m1")
    lay.gear(228, 140, 22, 30, 16, "m3", ["m11", "m10", "m9", "m8"], "m5", "m1")
    lay.gear(230, 20, 12, 17, 9, "m3", ["m9", "m10", "m11", "m8"], "m5", "m1")
    lay.gear(16, 146, 12, 17, 9, "m3", ["m10", "m11", "m8", "m9"], "m5", "m1")

    # Logo: BLOCK MAYHEM in an extruded fire gradient with a dark outline (always inside bank 2).
    import font as fnt

    def text_width(text, scale):
        return sum((fnt.glyph_width(fnt.glyph(ch)) + 1) * scale for ch in text) - scale

    def logo(text, y, scale):
        x0 = 120 - text_width(text, scale) // 2
        mask = set()
        cxp = x0
        for ch in text:
            rows = fnt.glyph(ch)
            for ry, row in enumerate(rows):
                for rx, v in enumerate(row):
                    if v == "#":
                        for dy in range(scale):
                            for dx in range(scale):
                                mask.add((cxp + rx * scale + dx, y + ry * scale + dy))
            cxp += (fnt.glyph_width(rows) + 1) * scale
        # outline + extrusion
        for (px, py) in mask:
            for e in range(1, 4):
                lay.set(px + e, py + e, "f2")
            for (dx, dy) in ((-1, 0), (1, 0), (0, -1), (0, 1), (-1, -1), (1, 1), (-1, 1), (1, -1)):
                if (px + dx, py + dy) not in mask:
                    lay.set(px + dx, py + dy, "f1")
        for (px, py) in mask:
            t = (py - y) / (7 * scale)
            lay.set(px, py, ["f10", "f9", "f8", "f7", "f6", "f5", "f4"][min(6, int(t * 7))])
            if (px, py - 1) not in mask:
                lay.set(px, py, "f13")

    logo("BLOCK", 12, 4)
    logo("MAYHEM", 46, 4)
    # Subtitle band.
    lay.rect(0, 92, 256, 106, "c1")
    lay.rect(0, 92, 256, 93, "c5")
    lay.rect(0, 105, 256, 106, "c3")
    sub = "THE SPICE PROTOCOL"
    lay.text(120 - text_width(sub, 1) // 2, 95, sub, "c7")
    # Silhouettes of falling coloured blocks along the bottom.
    for i in range(14):
        x = 20 + i * 15 + rnd.randrange(6)
        y = 112 + rnd.randrange(40)
        col = "c%d" % (9 + i % 7)
        lay.rect(x, y, x + 7, y + 7, col)
        lay.rect(x, y, x + 7, y + 1, "c8")
        lay.rect(x + 7, y, x + 8, y + 8, "c1")
        lay.rect(x, y + 7, x + 8, y + 8, "c1")
    cycles = [(1, 8, 4, 5, ROTATE), (0, 11, 5, 6, ROTATE), (2, 4, 6, 4, ROTATE)]
    save_layer("bg_title", lay, gfx_dir, write_bmp, write_json, cycles)


def menu(gfx_dir, write_bmp, write_json):
    """Calm control-room backdrop for menus (readable behind text)."""
    pal = Palette([
        bank((0, 0, 0), (4, 6, 16), (8, 12, 28), (12, 18, 40), (18, 26, 54), (26, 36, 70), (36, 50, 92),
             (50, 70, 120), (20, 60, 90), (30, 100, 140), (60, 170, 220), (140, 230, 255), (90, 40, 120),
             (150, 70, 190), (220, 120, 255), (255, 255, 255)),
        bank((0, 0, 0), (4, 6, 16), (24, 28, 44), (40, 46, 70), (60, 68, 100), (90, 100, 136), (232, 192, 40),
             (30, 26, 20), (88, 96, 140), (120, 128, 170), (160, 170, 210), (210, 216, 240), (255, 80, 60),
             (60, 230, 120), (255, 160, 40), (255, 255, 255)),
    ])
    for i in range(16):
        pal.name("a%d" % i, 0, i)
        pal.name("m%d" % i, 1, i)
    lay = Layer(pal)
    lay.vgradient(0, 0, 256, 256, ["a1", "a2", "a3", "a2", "a1"])
    # Circuit traces whose highlights travel (palette rotation over a8..a11).
    rnd = random.Random(21)
    for k in range(18):
        x = rnd.randrange(256)
        y = rnd.randrange(20, 150)
        length = rnd.randrange(20, 70)
        horizontal = rnd.random() < 0.6
        for i in range(length):
            px, py = (x + i, y) if horizontal else (x, y + i)
            lay.set(px, py, "a%d" % (8 + (i // 3) % 4))
        lay.disc(x, y, 1.6, "a11")
    # Big faint gears.
    lay.gear(40, 120, 30, 38, 16, "a3", ["m8", "m9", "m10", "m9"], "a5", "a2")
    lay.gear(214, 52, 24, 31, 14, "a3", ["m9", "m10", "m9", "m8"], "a5", "a2")
    # Hazard stripes top and bottom.
    for x in range(256):
        for y in list(range(0, 5)) + list(range(155, 160)):
            lay.set(x, y, "m6" if ((x + y) // 4) % 2 == 0 else "m7")
    cycles = [(0, 8, 4, 6, ROTATE), (1, 8, 3, 8, ROTATE)]
    save_layer("bg_menu", lay, gfx_dir, write_bmp, write_json, cycles)


# ====================================================================== shared scenery helpers

def name_banks(pal, prefixes):
    for b, prefix in enumerate(prefixes):
        for i in range(16):
            pal.name("%s%d" % (prefix, i), b, i)


def cloud(lay, cx, cy, w, h, colors, rnd):
    """Puffy cloud built from overlapping ellipses, shaded top (colors: dark..light)."""
    for k in range(int(w / 6) + 3):
        x = cx - w / 2 + rnd.random() * w
        y = cy + rnd.random() * h * 0.4
        r = h * (0.4 + rnd.random() * 0.5)
        lay.disc(x, y, r, colors[0])
        lay.disc(x - r * 0.15, y - r * 0.2, r * 0.75, colors[1])
        if len(colors) > 2:
            lay.disc(x - r * 0.3, y - r * 0.35, r * 0.4, colors[2])


def building(lay, x, w, top, bottom, body, edge, window_colors, rnd, density=0.55, step=(3, 4)):
    lay.rect(x, top, x + w, bottom, body)
    lay.rect(x, top, x + 1, bottom, edge)
    for wy in range(top + 3, bottom - 2, step[1]):
        for wx in range(x + 2, x + w - 2, step[0]):
            if rnd.random() < density:
                lay.set(wx, wy, rnd.choice(window_colors))


def icicle(lay, x, y, length, body, light):
    lay.poly([(x - 2, y), (x + 2, y), (x, y + length)], body)
    lay.line(x - 1, y, x, y + length - 2, light)


def mountain(lay, x, base, width, height, body, light, snow=None):
    lay.poly([(x - width / 2, base), (x - width * 0.08, base - height), (x + width * 0.1, base - height * 0.92),
              (x + width / 2, base)], body)
    lay.poly([(x - width * 0.08, base - height), (x + width * 0.1, base - height * 0.92), (x - width * 0.02, base - height * 0.5),
              (x - width * 0.3, base - height * 0.3)], light)
    if snow:
        lay.poly([(x - width * 0.12, base - height * 0.9), (x - width * 0.08, base - height), (x + width * 0.1, base - height * 0.92),
                  (x + width * 0.06, base - height * 0.8), (x - width * 0.02, base - height * 0.84)], snow)


# ====================================================================== WORLD 2 - FROZEN FACTORY

def world2(gfx_dir, write_bmp, write_json):
    far_pal = Palette([
        bank((0, 0, 0), (6, 10, 30), (10, 18, 48), (16, 30, 70), (24, 46, 96), (36, 66, 124), (56, 92, 156),
             (90, 130, 190), (140, 180, 230), (200, 228, 255), (255, 255, 255),
             (40, 220, 170), (60, 170, 240), (120, 110, 230), (190, 120, 240), (20, 40, 60)),
        bank((0, 0, 0), (8, 14, 34), (30, 44, 70), (54, 72, 104), (84, 106, 140), (130, 150, 186), (190, 206, 232),
             (230, 240, 255), (80, 160, 220), (140, 210, 250), (210, 240, 255), (255, 214, 120), (255, 250, 220),
             (60, 120, 170), (24, 40, 64), (255, 255, 255)),
    ])
    name_banks(far_pal, ["a", "m"])
    rnd = random.Random(202)
    far = Layer(far_pal)
    far.vgradient(0, 0, 256, 256, ["a1", "a2", "a3", "a4", "a5", "a6", "a5"])
    # Aurora curtain: vertical streaks banded by height (a11..a14 rotate -> the light flows down the curtain).
    for x in range(256):
        top = 12 + math.sin(x * 0.045 * 2 * math.pi / (0.045 * 256 / 2) / (2 * math.pi) * 2 * math.pi / 256 * 256) * 0
        top = 12 + 7 * math.sin(2 * math.pi * x / 128.0) + 3 * math.sin(2 * math.pi * x / 42.6)
        length = 16 + 6 * math.sin(2 * math.pi * x / 64.0 + 1.0)
        streak = 0.6 + 0.4 * math.sin(x * 0.9)
        for k in range(int(length)):
            fade = 1.0 - k / length
            if fade * streak * 16 > (k * 7 + x * 3) % 16 * 0.55:
                far.set(x, top + k, "a%d" % (11 + int(k / length * 4) % 4))
    for _ in range(60):
        far.set(rnd.randrange(256), rnd.randrange(60), rnd.choice(["a9", "a10", "a8"]))
    # Distant ice mountains.
    for (x, w, h) in ((30, 120, 70), (120, 150, 90), (210, 130, 75), (290, 120, 70)):
        mountain(far, x % 256, 150, w, h, "a5", "a7", "a9")
    # Frozen factory: silos, tanks and gantries with frost caps.
    for (x, w, top) in ((8, 26, 96), (44, 18, 110), (150, 30, 90), (196, 20, 104), (228, 24, 116)):
        far.rect(x, top, x + w, 170, "m3")
        far.rect(x, top, x + 2, 170, "m4")
        far.rect(x + w - 2, top, x + w, 170, "m2")
        far.ellipse(x + w / 2, top, w / 2 + 1, 4, "m6")
        far.ellipse(x + w / 2, top - 1, w / 2 - 1, 2.5, "m7")
        for wy in range(top + 8, 165, 12):
            far.rect(x + 4, wy, x + w - 4, wy + 2, "m2")
            if rnd.random() < 0.6:
                far.rect(x + w // 2 - 2, wy + 4, x + w // 2 + 2, wy + 7, "m11")
        for k in range(w // 4):
            icicle(far, x + 2 + k * 4 + rnd.randrange(2), top + 3, 3 + rnd.randrange(6), "m9", "m10")
    # Frozen pipes with icicles.
    for y in (124, 150):
        far.rect(0, y, 256, y + 5, "m4")
        far.rect(0, y, 256, y + 1, "m6")
        for x in range(2, 256, 7):
            icicle(far, x, y + 5, 3 + rnd.randrange(8), "m9", "m10")
    # Snow ground.
    far.vgradient(0, 168, 256, 200, ["m6", "m7", "m10"])
    far.rect(0, 200, 256, 256, "m7")
    far_cycles = [(0, 11, 4, 8, ROTATE), (1, 11, 2, 50, BLINK), (0, 9, 1, 0, FLICKER)]
    save_layer("bg_w2_far", far, gfx_dir, write_bmp, write_json, far_cycles)

    near_pal = Palette([
        bank((0, 0, 0), (8, 14, 34), (40, 70, 110), (80, 140, 200), (140, 210, 250), (210, 240, 255), (255, 255, 255),
             (30, 44, 70), (54, 72, 104), (90, 110, 150), (150, 170, 200), (60, 240, 255), (180, 255, 255),
             (64, 160, 240), (120, 200, 255), (24, 36, 60)),
        bank((0, 0, 0), (8, 14, 34), (60, 100, 150), (110, 170, 220), (200, 230, 255), (255, 255, 255),
             (80, 90, 120), (120, 130, 160), (180, 190, 210), (240, 244, 255), (64, 200, 255), (20, 30, 50),
             (100, 80, 160), (160, 140, 220), (255, 220, 140), (40, 50, 80)),
    ])
    name_banks(near_pal, ["n", "c"])
    near = Layer(near_pal)
    # Big icicles hanging from the top edge.
    for x in range(0, 256, 9):
        length = 6 + rnd.randrange(16)
        near.poly([(x - 4, 0), (x + 4, 0), (x + rnd.randrange(3) - 1, length)], "n4")
        near.line(x - 2, 0, x, length - 3, "n6")
        near.line(x + 3, 0, x + 1, length - 2, "n3")
    near.rect(0, 0, 256, 3, "n5")
    # Crystal formations at the bottom (glints blink).
    for x in range(4, 256, 22):
        h = 10 + rnd.randrange(16)
        base = 160
        near.poly([(x, base), (x + 4, base - h), (x + 8, base)], "n3")
        near.poly([(x + 4, base), (x + 4, base - h), (x + 8, base)], "n2")
        near.line(x + 3, base - h + 2, x + 2, base - 2, "n5")
        near.set(x + 4, base - h, "n12")
        h2 = 6 + rnd.randrange(10)
        near.poly([(x + 9, base), (x + 12, base - h2), (x + 15, base)], "n4")
    # Snow drift band.
    for x in range(256):
        top = 152 + int(math.sin(x * 0.08) * 3 + math.sin(x * 0.21) * 2)
        near.rect(x, top, x + 1, 168, "n5")
        near.set(x, top, "n6")
    # Frozen machinery box with a cold control panel.
    for mx in (40, 182):
        near.rect(mx, 118, mx + 30, 150, "n8")
        near.rect(mx, 118, mx + 30, 120, "n10")
        near.rect(mx + 4, 124, mx + 26, 134, "n7")
        for k in range(4):
            near.rect(mx + 6 + k * 5, 126, mx + 9 + k * 5, 129, "n11" if k % 2 else "n13")
        near.ellipse(mx + 15, 118, 17, 3, "n5")
        for k in range(6):
            icicle(near, mx + 3 + k * 5, 121, 4 + rnd.randrange(5), "n4", "n6")
    near_cycles = [(0, 11, 3, 10, ROTATE), (0, 12, 1, 0, FLICKER)]
    save_layer("bg_w2_near", near, gfx_dir, write_bmp, write_json, near_cycles)


# ====================================================================== WORLD 3 - VOLCANO MADNESS

def world3(gfx_dir, write_bmp, write_json):
    far_pal = Palette([
        bank((0, 0, 0), (16, 4, 6), (32, 8, 10), (54, 12, 12), (80, 18, 14), (110, 30, 18), (150, 44, 20),
             (60, 40, 40), (90, 60, 56), (130, 90, 80), (40, 26, 28), (200, 60, 20), (240, 110, 24),
             (255, 170, 40), (255, 230, 120), (24, 12, 14)),
        bank((0, 0, 0), (20, 8, 10), (46, 20, 18), (76, 34, 28), (110, 52, 40), (150, 76, 54), (200, 40, 16),
             (240, 96, 24), (255, 168, 48), (255, 232, 120), (255, 255, 220), (70, 60, 60), (110, 100, 96),
             (160, 150, 140), (40, 30, 34), (255, 120, 80)),
    ])
    name_banks(far_pal, ["a", "v"])
    rnd = random.Random(303)
    far = Layer(far_pal)
    far.vgradient(0, 0, 256, 256, ["a2", "a3", "a4", "a5", "a6", "a5", "a3"])
    # Smoke clouds.
    for (cx, cy, w, h) in ((40, 30, 90, 22), (150, 20, 110, 26), (230, 44, 80, 20), (100, 60, 70, 16)):
        cloud(far, cx, cy, w, h, ["a7", "a8", "a9"], rnd)
    # Volcanoes with lava streams (stream colours a11..a14 rotate -> flowing lava).
    for (x, w, h) in ((54, 140, 100), (190, 120, 84)):
        mountain(far, x, 176, w, h, "v2", "v4")
        crater_y = 176 - h
        far.ellipse(x - w * 0.0, crater_y + 3, 10, 3, "a13")
        for k in range(3):
            sx = x - 4 + k * 4
            prev = (sx, crater_y + 4)
            for i in range(1, 30):
                t = i / 30
                px = sx + math.sin(i * 0.4 + k) * 4 + (k - 1) * t * 30
                py = crater_y + 4 + t * (h - 6)
                far.line(prev[0], prev[1], px, py, "a%d" % (11 + (i // 2) % 4), 2)
                prev = (px, py)
        # Eruption glow column.
        for i in range(20):
            far.disc(x + rnd.randrange(7) - 3, crater_y - i * 2, 2 + rnd.random() * 2, "a13" if i < 8 else "a12")
    # Burning machinery silhouettes.
    for (x, w, top) in ((112, 22, 120), (140, 16, 132), (226, 26, 126), (0, 20, 134)):
        far.rect(x, top, x + w, 176, "a10")
        far.rect(x + 3, top - 10, x + 7, top, "a10")
        for k in range(3):
            far.disc(x + 5 + k * 2, top - 12 - k * 3, 2.5 - k * 0.5, "a12")
    # Lava sea with rocks.
    far.vgradient(0, 176, 256, 256, ["v6", "v7", "v6", "v2"])
    for _ in range(30):
        x, y = rnd.randrange(256), 180 + rnd.randrange(50)
        far.ellipse(x, y, 4 + rnd.randrange(6), 2, "v3")
    far_cycles = [(0, 11, 4, 4, ROTATE), (1, 6, 4, 5, ROTATE), (0, 13, 1, 0, FLICKER)]
    save_layer("bg_w3_far", far, gfx_dir, write_bmp, write_json, far_cycles)

    near_pal = Palette([
        bank((0, 0, 0), (20, 8, 10), (50, 22, 20), (84, 38, 30), (120, 56, 42), (160, 80, 56), (200, 40, 16),
             (240, 96, 24), (255, 168, 48), (255, 232, 120), (255, 255, 220), (40, 30, 30), (70, 56, 52),
             (110, 90, 80), (255, 140, 40), (30, 16, 16)),
        bank((0, 0, 0), (20, 8, 10), (60, 40, 40), (100, 70, 64), (150, 110, 96), (200, 60, 20), (240, 110, 24),
             (255, 180, 50), (255, 240, 140), (90, 90, 100), (130, 130, 140), (180, 180, 190), (40, 40, 50),
             (255, 90, 40), (255, 210, 90), (12, 6, 6)),
    ])
    name_banks(near_pal, ["n", "b"])
    near = Layer(near_pal)
    # Lava river along the bottom (n6..n9 rotate).
    for x in range(256):
        for y in range(148, 168):
            near.set(x, y, "n%d" % (6 + ((x // 5 + y // 3 + (x * y) % 3) % 4)))
    for x in range(256):
        near.set(x, 147, "n2")
        near.set(x, 146, "n11" if x % 3 else "n12")
    # Molten rocks and glowing cracks.
    for x in range(6, 256, 26):
        h = 8 + rnd.randrange(14)
        near.poly([(x, 150), (x + 4, 150 - h), (x + 12, 150 - h + 3), (x + 18, 150)], "n3")
        near.poly([(x + 4, 150 - h), (x + 12, 150 - h + 3), (x + 8, 150 - h + 6)], "n5")
        near.line(x + 6, 150 - h + 6, x + 9, 148, "n8")
    # Burning machine wreck with flames (flame colours flicker).
    for mx in (34, 176):
        near.poly([(mx, 146), (mx + 6, 118), (mx + 30, 112), (mx + 36, 146)], "b3")
        near.poly([(mx + 6, 118), (mx + 30, 112), (mx + 28, 118), (mx + 10, 122)], "b4")
        near.disc(mx + 18, 132, 6, "b9")
        near.disc(mx + 18, 132, 3, "b12")
        for k in range(4):
            fx, fy = mx + 8 + k * 7, 112 - (k % 2) * 3
            near.poly([(fx - 3, fy + 2), (fx, fy - 9), (fx + 3, fy + 2)], "b13")
            near.poly([(fx - 1, fy + 2), (fx, fy - 5), (fx + 1, fy + 2)], "b14")
    near_cycles = [(0, 6, 4, 3, ROTATE), (1, 13, 2, 0, FLICKER), (0, 14, 1, 0, FLICKER)]
    save_layer("bg_w3_near", near, gfx_dir, write_bmp, write_json, near_cycles)


# ====================================================================== WORLD 4 - ELECTRIC CITY

def world4(gfx_dir, write_bmp, write_json):
    far_pal = Palette([
        bank((0, 0, 0), (6, 2, 16), (12, 6, 30), (20, 10, 46), (32, 16, 66), (48, 24, 92), (70, 36, 120),
             (30, 30, 50), (46, 46, 70), (64, 64, 96), (200, 200, 255), (64, 232, 255), (255, 80, 200),
             (180, 120, 255), (255, 220, 120), (14, 10, 26)),
        bank((0, 0, 0), (10, 8, 20), (20, 16, 36), (30, 26, 52), (44, 38, 70), (64, 232, 255), (20, 120, 160),
             (255, 80, 200), (140, 30, 110), (255, 240, 120), (180, 255, 255), (255, 255, 255), (120, 255, 160),
             (255, 120, 60), (90, 80, 140), (6, 4, 14)),
    ])
    name_banks(far_pal, ["a", "c"])
    rnd = random.Random(404)
    far = Layer(far_pal)
    far.vgradient(0, 0, 256, 256, ["a1", "a2", "a3", "a4", "a5", "a6", "a4"])
    # Storm clouds.
    for (cx, cy, w, h) in ((30, 22, 100, 18), (150, 14, 130, 22), (240, 30, 90, 16)):
        cloud(far, cx, cy, w, h, ["a7", "a8", "a9"], rnd)
    # Lightning bolts (a10 blinks -> flashes).
    for (x0, y0) in ((70, 26), (200, 34)):
        x, y = x0, y0
        for i in range(10):
            nx, ny = x + rnd.randrange(9) - 4, y + 6
            far.line(x, y, nx, ny, "a10", 1)
            x, y = nx, ny
    # Skyline of dark skyscrapers with lit windows (window colours flicker / rotate).
    x = 0
    while x < 256:
        w = 10 + rnd.randrange(16)
        top = 60 + rnd.randrange(60)
        building(far, x, w, top, 200, "c2", "c4", ["c5", "c7", "c9", "c3"], rnd, 0.5)
        if rnd.random() < 0.4:
            far.line(x + w // 2, top, x + w // 2, top - 10, "c4")
            far.set(x + w // 2, top - 11, "c13")
        x += w + 1
    # Giant neon billboards.
    for (bx, by, kinds, col) in ((12, 88, ["bolt", "bolt", "bolt"], "c5"), (150, 76, ["heart", "star", "heart"], "c7"),
                                 (206, 112, ["chili", "lblock", "chili"], "c12")):
        far.rect(bx - 2, by - 2, bx + 32, by + 11, "c1")
        far.rect(bx - 2, by - 2, bx + 32, by - 1, col)
        far.rect(bx - 2, by + 10, bx + 32, by + 11, col)
        pictogram_row(far, bx + 2, by + 1, kinds, col, gap=5)
    far.rect(0, 200, 256, 256, "c1")
    far_cycles = [(0, 10, 1, 90, BLINK), (1, 5, 3, 7, ROTATE), (0, 11, 3, 6, ROTATE), (1, 9, 1, 0, FLICKER)]
    save_layer("bg_w4_far", far, gfx_dir, write_bmp, write_json, far_cycles)

    near_pal = Palette([
        bank((0, 0, 0), (6, 4, 14), (16, 12, 30), (28, 22, 48), (44, 36, 72), (64, 54, 100), (64, 232, 255),
             (255, 80, 200), (180, 120, 255), (255, 240, 120), (20, 120, 160), (140, 30, 110), (255, 255, 255),
             (120, 255, 160), (90, 90, 120), (10, 8, 20)),
        bank((0, 0, 0), (8, 6, 18), (40, 34, 60), (70, 60, 96), (110, 100, 140), (64, 232, 255), (255, 80, 200),
             (255, 240, 120), (255, 255, 255), (60, 60, 80), (100, 100, 120), (150, 150, 170), (20, 20, 30),
             (255, 120, 60), (120, 255, 160), (180, 255, 255)),
    ])
    name_banks(near_pal, ["n", "s"])
    near = Layer(near_pal)
    # Foreground rooftops with neon trim (trim colours rotate).
    x = 0
    while x < 256:
        w = 18 + rnd.randrange(22)
        top = 128 + rnd.randrange(18)
        near.rect(x, top, x + w, 168, "n2")
        near.rect(x, top, x + w, top + 1, "n%d" % (6 + (x // 20) % 3))
        for wy in range(top + 4, 166, 5):
            for wx in range(x + 2, x + w - 2, 4):
                if rnd.random() < 0.45:
                    near.set(wx, wy, rnd.choice(["n9", "n6", "n7"]))
        if rnd.random() < 0.5:
            near.rect(x + 3, top - 8, x + 7, top, "n3")
            near.set(x + 5, top - 9, "n7")
        x += w + 2
    # Power cables with sparks between rooftops.
    for k in range(3):
        y0 = 96 + k * 9
        for x in range(256):
            y = y0 + 6 * math.sin(math.pi * ((x + k * 40) % 128) / 128.0)
            near.set(x, y, "n4")
        for x in range(10 + k * 30, 256, 90):
            y = y0 + 6 * math.sin(math.pi * ((x + k * 40) % 128) / 128.0)
            near.disc(x, y, 1.5, "n12")
    near_cycles = [(0, 6, 3, 9, ROTATE), (0, 12, 1, 0, FLICKER)]
    save_layer("bg_w4_near", near, gfx_dir, write_bmp, write_json, near_cycles)


# ====================================================================== WORLD 5 - GRAVITY DIMENSION

def world5(gfx_dir, write_bmp, write_json):
    far_pal = Palette([
        bank((0, 0, 0), (4, 2, 14), (10, 6, 28), (18, 10, 44), (28, 14, 64), (40, 20, 86), (60, 30, 110),
             (90, 50, 140), (130, 80, 180), (20, 60, 50), (30, 110, 80), (60, 230, 140), (180, 255, 220),
             (255, 255, 255), (200, 120, 255), (8, 4, 20)),
        bank((0, 0, 0), (8, 6, 20), (30, 26, 50), (50, 46, 80), (80, 76, 120), (120, 116, 170), (60, 230, 140),
             (20, 120, 80), (176, 72, 232), (100, 40, 140), (240, 190, 60), (255, 255, 255), (120, 255, 200),
             (255, 120, 220), (40, 30, 60), (180, 160, 255)),
    ])
    name_banks(far_pal, ["a", "g"])
    rnd = random.Random(505)
    far = Layer(far_pal)
    far.vgradient(0, 0, 256, 256, ["a1", "a2", "a3", "a4", "a5", "a4", "a2"])
    # Nebula swirls.
    for k in range(4):
        cx, cy = 30 + k * 64, 40 + (k % 2) * 50
        for i in range(160):
            a = i * 0.13
            r = 6 + i * 0.25
            far.set(cx + math.cos(a) * r, cy + math.sin(a) * r * 0.6, rnd.choice(["a6", "a7", "a14"]))
    for _ in range(220):
        far.set(rnd.randrange(256), rnd.randrange(256), rnd.choice(["a12", "a13", "a8", "a13"]))
    # A banded ringed planet.
    pcx, pcy, pr = 58, 46, 22
    for y in range(int(pcy - pr), int(pcy + pr) + 1):
        for x in range(int(pcx - pr), int(pcx + pr) + 1):
            if (x - pcx) ** 2 + (y - pcy) ** 2 <= pr * pr:
                band = int((y - pcy + pr) / 5) % 3
                shade = (x - pcx) + (y - pcy) * 0.5
                far.set(x, y, ["g9", "g8", "g14"][band] if shade > 6 else ["g8", "g15", "g9"][band])
    for k in range(96):
        a = k * math.pi / 48
        rx, ry = pcx + math.cos(a) * 34, pcy + math.sin(a) * 8
        if math.sin(a) < 0 and (rx - pcx) ** 2 + (ry - pcy) ** 2 < pr * pr:
            continue                                   # back half of the ring hidden by the planet
        far.disc(rx, ry, 1.1, "a%d" % (9 + k % 4))
    # Floating islands with glowing crystal tops and hanging roots.
    for (x, y, w) in ((150, 92, 56), (220, 128, 44), (24, 132, 40), (196, 44, 30)):
        far.poly([(x - w / 2, y), (x + w / 2, y), (x + w * 0.25, y + w * 0.5), (x - w * 0.1, y + w * 0.62),
                  (x - w * 0.35, y + w * 0.35)], "g2")
        far.poly([(x - w / 2, y), (x + w / 2, y), (x + w * 0.3, y + w * 0.15), (x - w * 0.4, y + w * 0.12)], "g3")
        far.rect(x - w / 2, y - 2, x + w / 2, y, "g6")
        for k in range(4):
            rx = x - w * 0.2 + k * w * 0.13
            far.line(rx, y + w * 0.3, rx + (k % 2) * 2 - 1, y + w * 0.3 + 6 + k * 2, "g7")
        for k in range(3):
            cx2 = x - w * 0.25 + k * w * 0.25
            h = 6 + (k * 5) % 9
            far.poly([(cx2 - 3, y - 2), (cx2, y - 2 - h), (cx2 + 3, y - 2)], "g8")
            far.line(cx2, y - h, cx2 - 1, y - 3, "g15")
        # Impossible triangle above the biggest island.
        if w > 50:
            p = [(x - 14, y - 16), (x + 14, y - 16), (x, y - 40)]
            for i in range(3):
                far.line(p[i][0], p[i][1], p[(i + 1) % 3][0], p[(i + 1) % 3][1], "g%d" % (8 + i % 2), 3)
    # A staircase to nowhere.
    for k in range(7):
        sx, sy = 92 + k * 7, 150 - k * 6
        far.rect(sx, sy, sx + 8, sy + 3, "g4")
        far.rect(sx, sy, sx + 8, sy + 1, "g5")
    # Small rotating ring around a floating cube.
    for k in range(48):
        a = k * math.pi / 24
        far.disc(214 + math.cos(a) * 16, 82 + math.sin(a) * 6, 1.2, "a%d" % (9 + k % 4))
    far.poly([(214, 76), (220, 82), (214, 88), (208, 82)], "g6")
    far_cycles = [(0, 9, 4, 5, ROTATE), (0, 12, 2, 0, FLICKER), (1, 8, 2, 20, BLINK)]
    save_layer("bg_w5_far", far, gfx_dir, write_bmp, write_json, far_cycles)

    near_pal = Palette([
        bank((0, 0, 0), (8, 6, 20), (40, 26, 30), (70, 46, 50), (100, 70, 72), (60, 230, 140), (20, 120, 80),
             (176, 72, 232), (100, 40, 140), (255, 255, 255), (120, 255, 200), (255, 120, 220), (30, 20, 50),
             (240, 190, 60), (80, 76, 120), (14, 10, 24)),
        bank((0, 0, 0), (8, 6, 20), (60, 230, 140), (120, 255, 200), (176, 72, 232), (255, 120, 220),
             (240, 190, 60), (255, 255, 255), (40, 36, 60), (80, 76, 120), (120, 116, 170), (180, 176, 220),
             (20, 120, 80), (100, 40, 140), (30, 26, 44), (200, 255, 240)),
    ])
    name_banks(near_pal, ["n", "q"])
    near = Layer(near_pal)
    # Floating rock islands drifting in the foreground.
    for (x, y, w) in ((20, 136, 34), (98, 150, 26), (150, 128, 40), (226, 146, 30)):
        near.poly([(x - w / 2, y), (x + w / 2, y), (x + w * 0.25, y + w * 0.5), (x - w * 0.2, y + w * 0.45)], "n3")
        near.poly([(x - w / 2, y), (x + w / 2, y), (x + w * 0.3, y + w * 0.15), (x - w * 0.35, y + w * 0.12)], "n4")
        near.rect(x - w / 2, y - 2, x + w / 2, y, "n5")
        for k in range(3):
            near.set(x - w / 3 + k * w / 3, y - 3, "n10")
    # Spinning geometric cubes.
    for (cx, cy) in ((60, 110), (190, 104), (128, 118)):
        pts = [(cx, cy - 7), (cx + 7, cy), (cx, cy + 7), (cx - 7, cy)]
        near.poly(pts, "q4")
        near.poly([pts[0], pts[1], (cx, cy)], "q5")
        near.line(pts[0][0], pts[0][1], pts[2][0], pts[2][1], "q7")
    # Energy streams (q2,q3 rotate).
    for k in range(2):
        for x in range(256):
            y = 96 + k * 50 + math.sin(x * 0.05 + k * 2) * 6
            near.set(x, y, "q%d" % (2 + (x // 4) % 2))
    near_cycles = [(1, 2, 2, 6, ROTATE), (0, 5, 2, 12, ROTATE), (0, 10, 1, 0, FLICKER)]
    save_layer("bg_w5_near", near, gfx_dir, write_bmp, write_json, near_cycles)


# ====================================================================== WORLD 6 - THE FINAL MELTDOWN

def world6(gfx_dir, write_bmp, write_json):
    far_pal = Palette([
        bank((0, 0, 0), (10, 4, 14), (22, 8, 26), (36, 12, 40), (56, 16, 56), (80, 20, 70), (40, 70, 110),
             (90, 150, 210), (190, 230, 255), (200, 60, 20), (255, 140, 40), (255, 230, 120), (180, 120, 255),
             (255, 80, 200), (64, 232, 255), (255, 255, 255)),
        bank((0, 0, 0), (12, 6, 16), (40, 36, 56), (70, 66, 90), (110, 106, 136), (160, 156, 190),
             (232, 192, 40), (24, 20, 30), (255, 60, 60), (255, 160, 60), (120, 220, 255), (180, 120, 255),
             (60, 230, 140), (220, 40, 40), (255, 210, 90), (255, 255, 255)),
    ])
    name_banks(far_pal, ["a", "m"])
    rnd = random.Random(606)
    far = Layer(far_pal)
    far.vgradient(0, 0, 256, 256, ["a1", "a2", "a3", "a4", "a5", "a3"])
    # Cosmic portal in the middle (portal colours rotate -> swirling).
    cx, cy = 128, 64
    for i in range(420):
        a = i * 0.11
        r = 4 + i * 0.09
        far.disc(cx + math.cos(a) * r, cy + math.sin(a) * r * 0.75, 1.2, "a%d" % (12 + (i // 12) % 3))
    far.disc(cx, cy, 5, "a15")
    # Left: frozen structures; right: volcanic fragments - the worlds colliding.
    for (x, top) in ((10, 90), (30, 104), (50, 84)):
        far.poly([(x, 170), (x + 6, top), (x + 12, 170)], "a7")
        far.line(x + 6, top + 2, x + 4, 168, "a8")
    for (x, y, r) in ((196, 96, 10), (220, 120, 8), (240, 84, 6), (180, 128, 7)):
        far.disc(x, y, r, "a5")
        far.disc(x - 2, y - 2, r * 0.6, "a9")
        far.disc(x - 3, y - 3, r * 0.3, "a10")
    # Broken arcade machinery (gears with missing teeth) and an electrical storm.
    far.gear(84, 126, 14, 19, 12, "m2", ["m3", "m4", "m5", "m4"], "m5", "m1")
    far.gear(162, 140, 10, 14, 9, "m2", ["m5", "m4", "m3", "m4"], "m5", "m1")
    for (x0, y0) in ((60, 10), (176, 6)):
        x, y = x0, y0
        for i in range(8):
            nx, ny = x + rnd.randrange(11) - 5, y + 7
            far.line(x, y, nx, ny, "m10")
            x, y = nx, ny
    far.vgradient(0, 168, 256, 256, ["m2", "m7", "m1"])
    for x in range(0, 256, 2):
        far.set(x, 168, "m13" if (x // 8) % 2 else "m6")
    far_cycles = [(0, 12, 3, 4, ROTATE), (1, 10, 1, 60, BLINK), (0, 9, 3, 6, ROTATE), (1, 13, 1, 0, FLICKER)]
    save_layer("bg_w6_far", far, gfx_dir, write_bmp, write_json, far_cycles)

    near_pal = Palette([
        bank((0, 0, 0), (10, 6, 14), (40, 36, 56), (80, 76, 100), (140, 136, 168), (220, 216, 240), (255, 90, 30),
             (255, 210, 90), (120, 220, 255), (210, 240, 255), (180, 120, 255), (220, 40, 40), (60, 230, 140),
             (255, 255, 255), (232, 192, 40), (20, 16, 26)),
        bank((0, 0, 0), (10, 6, 14), (60, 40, 40), (100, 60, 50), (150, 90, 70), (255, 90, 30), (255, 160, 60),
             (120, 220, 255), (200, 240, 255), (180, 120, 255), (255, 80, 200), (64, 232, 255), (255, 255, 255),
             (40, 30, 50), (80, 70, 100), (220, 40, 40)),
    ])
    name_banks(near_pal, ["n", "f"])
    near = Layer(near_pal)
    # Floating fragments from every world: steel plates, ice shards, lava rocks, neon pieces.
    kinds = [("n3", "n4"), ("n8", "n9"), ("f4", "f6"), ("n10", "n13")]
    for k in range(18):
        x = rnd.randrange(256)
        y = 100 + rnd.randrange(60)
        body, light = kinds[k % 4]
        s = 4 + rnd.randrange(6)
        pts = [(x - s, y), (x, y - s), (x + s, y + s // 2), (x - s // 2, y + s)]
        near.poly(pts, body)
        near.line(pts[0][0], pts[0][1], pts[1][0], pts[1][1], light)
    # Cracked floor with glowing magma seams (seam colours rotate).
    for x in range(256):
        near.rect(x, 152, x + 1, 168, "n2")
        near.set(x, 152, "n4")
    x = 0
    while x < 256:
        w = 8 + rnd.randrange(16)
        for i in range(10):
            near.set(x + i % 3, 153 + i, "n%d" % (6 + i % 2))
        x += w
    near_cycles = [(0, 6, 2, 5, ROTATE), (0, 10, 2, 0, FLICKER), (1, 9, 3, 7, ROTATE)]
    save_layer("bg_w6_near", near, gfx_dir, write_bmp, write_json, near_cycles)


WORLD_POS = [(36, 118), (78, 74), (120, 116), (160, 68), (198, 110), (214, 34)]


def worldmap(gfx_dir, write_bmp, write_json):
    pal = Palette([
        bank((0, 0, 0), (4, 4, 16), (10, 8, 30), (18, 14, 48), (28, 22, 70), (44, 34, 96), (70, 56, 130),
             (110, 96, 170), (40, 30, 34), (70, 54, 52), (104, 82, 76), (140, 116, 104), (180, 160, 150),
             (220, 210, 240), (255, 255, 255), (6, 6, 12)),
        bank((0, 0, 0), (20, 22, 36), (56, 60, 80), (96, 104, 128), (150, 160, 184), (220, 226, 240),
             (232, 192, 40), (150, 110, 20), (80, 160, 220), (140, 210, 250), (210, 240, 255), (255, 255, 255),
             (40, 70, 110), (24, 40, 80), (248, 144, 32), (8, 10, 20)),
        bank((0, 0, 0), (40, 10, 12), (110, 24, 20), (180, 40, 16), (240, 96, 24), (255, 168, 48), (255, 232, 120),
             (60, 40, 40), (40, 24, 72), (72, 44, 120), (120, 80, 180), (64, 232, 255), (255, 80, 200),
             (200, 170, 240), (20, 10, 30), (255, 255, 255)),
        bank((0, 0, 0), (60, 230, 140), (120, 255, 200), (200, 255, 230), (255, 255, 255), (20, 120, 80),
             (176, 72, 232), (255, 60, 60), (255, 160, 60), (120, 220, 255), (80, 76, 100), (40, 36, 56),
             (140, 136, 168), (220, 40, 40), (255, 210, 90), (6, 4, 12)),
    ])
    for i in range(16):
        pal.name("s%d" % i, 0, i)
        pal.name("f%d" % i, 1, i)
        pal.name("v%d" % i, 2, i)
        pal.name("p%d" % i, 3, i)
    lay = Layer(pal)
    lay.vgradient(0, 0, 256, 256, ["s1", "s2", "s3", "s4", "s3", "s2"])
    rnd = random.Random(77)
    for _ in range(110):
        lay.set(rnd.randrange(256), rnd.randrange(256), rnd.choice(["s7", "s13", "s14", "s6"]))
    # Nebula wisps.
    for k in range(5):
        cx, cy = rnd.randrange(256), rnd.randrange(160)
        for i in range(60):
            a = i * 0.21 + k
            lay.set(cx + math.cos(a) * (10 + i * 0.6), cy + math.sin(a * 1.3) * (6 + i * 0.3), "s5")

    # Energy path between worlds (dots cycle p1..p4 -> the path flows).
    for w in range(5):
        (x0, y0), (x1, y1) = WORLD_POS[w], WORLD_POS[w + 1]
        steps = int(math.hypot(x1 - x0, y1 - y0) / 3)
        for i in range(steps):
            t = i / steps
            x = x0 + (x1 - x0) * t
            y = y0 + (y1 - y0) * t - math.sin(t * math.pi) * 10
            lay.disc(x, y, 1.3, "p%d" % (1 + i % 4))

    def island(cx, cy, w=22, h=12):
        lay.poly([(cx - w, cy), (cx + w, cy), (cx + w * 0.6, cy + h * 0.6), (cx + 4, cy + h * 1.4),
                  (cx - 6, cy + h * 1.2), (cx - w * 0.7, cy + h * 0.5)], "s9")
        lay.poly([(cx - w, cy), (cx + w, cy), (cx + w * 0.5, cy + h * 0.35), (cx - w * 0.6, cy + h * 0.3)], "s10")
        lay.rect(cx - w, cy - 1, cx + w, cy + 1, "s11")
        for k in range(4):
            lay.set(cx - w * 0.5 + k * w * 0.35, cy + h * 0.8 + (k % 2) * 2, "s8")

    # W1 factory: platform, chimney, gear.
    x, y = WORLD_POS[0]
    island(x, y)
    lay.rect(x - 14, y - 12, x + 4, y, "f3")
    lay.rect(x - 14, y - 12, x + 4, y - 11, "f4")
    lay.rect(x + 6, y - 20, x + 11, y, "f2")
    lay.rect(x + 6, y - 21, x + 11, y - 20, "f6")
    lay.gear(x - 5, y - 18, 4, 6, 8, "f2", ["f4", "f5", "f4", "f3"], "f6", "f1")
    lay.rect(x - 12, y - 8, x - 9, y - 5, "f6")
    lay.rect(x - 6, y - 8, x - 3, y - 5, "f14")
    # W2 frozen factory: crystal spires.
    x, y = WORLD_POS[1]
    island(x, y)
    for (dx, h) in ((-14, 10), (-7, 16), (0, 22), (7, 14), (14, 9)):
        lay.poly([(x + dx - 3, y), (x + dx, y - h), (x + dx + 3, y)], "f9")
        lay.line(x + dx, y - h + 2, x + dx, y - 1, "f10")
    lay.rect(x - 18, y - 2, x + 18, y, "f11")
    # W3 volcano: cone with a glowing crater (lava colours pulse).
    x, y = WORLD_POS[2]
    island(x, y)
    lay.poly([(x - 18, y), (x - 5, y - 20), (x + 5, y - 20), (x + 18, y)], "v7")
    lay.poly([(x - 5, y - 20), (x + 5, y - 20), (x + 3, y - 17), (x - 3, y - 17)], "v5")
    lay.line(x - 2, y - 17, x - 6, y - 4, "v4")
    lay.line(x + 1, y - 17, x + 5, y - 6, "v3")
    lay.disc(x, y - 24, 2, "v6")
    # W4 electric city: skyscrapers with neon windows (flicker).
    x, y = WORLD_POS[3]
    island(x, y)
    for (dx, h, c) in ((-15, 12, "v9"), (-8, 20, "v8"), (-1, 26, "v9"), (6, 16, "v8"), (13, 10, "v9")):
        lay.rect(x + dx, y - h, x + dx + 6, y, c)
        for wy in range(y - h + 2, y - 1, 3):
            lay.set(x + dx + 2, wy, "v11" if (wy + dx) % 2 else "v12")
    lay.line(x + 1, y - 26, x + 2, y - 32, "v11")
    # W5 gravity dimension: floating fragments and a ring.
    x, y = WORLD_POS[4]
    island(x, y)
    lay.ring(x, y - 14, 10, 1.5, "p1")
    for (dx, dy) in ((-10, -22), (8, -26), (14, -12), (-14, -8)):
        lay.rect(x + dx, y + dy, x + dx + 4, y + dy + 4, "p6")
        lay.set(x + dx, y + dy, "p3")
    lay.disc(x, y - 14, 3, "p2")
    # W6 final meltdown: dark core with a red eye.
    x, y = WORLD_POS[5]
    island(x, y, 20, 12)
    lay.disc(x, y - 14, 12, "p11")
    lay.ring(x, y - 14, 12, 2, "p12")
    lay.disc(x, y - 14, 7, "p13")
    lay.disc(x, y - 14, 4, "p8")
    lay.disc(x, y - 14, 2, "p15")
    for k in range(6):
        a = k * math.pi / 3
        lay.line(x + math.cos(a) * 13, y - 14 + math.sin(a) * 13, x + math.cos(a) * 17, y - 14 + math.sin(a) * 17,
                 "p9" if k % 2 else "p6")
    cycles = [(3, 1, 4, 4, ROTATE), (2, 4, 3, 5, ROTATE), (2, 11, 2, 0, FLICKER), (3, 13, 2, 30, PULSE),
              (1, 4, 2, 8, ROTATE)]
    save_layer("bg_worldmap", lay, gfx_dir, write_bmp, write_json, cycles)


def write_cycles_header(inc_dir):
    lines = ["#ifndef BM_BG_CYCLES_H\n#define BM_BG_CYCLES_H\n\n#include <stdint.h>\n\nnamespace bm::bgfx\n{\n",
             "    struct cycle { uint8_t bank; uint8_t first; uint8_t count; uint8_t period; uint8_t mode; };\n",
             "    enum cycle_mode : uint8_t { rotate = 0, blink = 1, flicker = 2, pulse = 3 };\n\n"]
    for name, cycles in CYCLES.items():
        lines.append("    constexpr cycle %s_cycles[] = {\n" % name)
        for c in cycles:
            lines.append("        { %d, %d, %d, %d, %d },\n" % c)
        if not cycles:
            lines.append("        { 0, 0, 0, 0, 0 },\n")
        lines.append("    };\n    constexpr int %s_cycle_count = %d;\n\n" % (name, len(cycles)))
    lines.append("}\n\n#endif\n")
    with open(os.path.join(inc_dir, "bm_bg_cycles.h"), "w", newline="\n", encoding="utf-8") as f:
        f.write("// Generated by tools/gen_graphics.py - do not edit.\n")
        f.write("".join(lines))


def generate(gfx_dir, inc_dir, write_bmp, write_json):
    world1(gfx_dir, write_bmp, write_json)
    world2(gfx_dir, write_bmp, write_json)
    world3(gfx_dir, write_bmp, write_json)
    world4(gfx_dir, write_bmp, write_json)
    world5(gfx_dir, write_bmp, write_json)
    world6(gfx_dir, write_bmp, write_json)
    title(gfx_dir, write_bmp, write_json)
    menu(gfx_dir, write_bmp, write_json)
    worldmap(gfx_dir, write_bmp, write_json)
    write_cycles_header(inc_dir)
