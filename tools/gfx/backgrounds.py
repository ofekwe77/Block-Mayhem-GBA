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
    far.text(156, 34, "MAYHEM", "pink")
    far.text(180, 52, "ARCADE", "cyan")
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
    title(gfx_dir, write_bmp, write_json)
    menu(gfx_dir, write_bmp, write_json)
    write_cycles_header(inc_dir)
