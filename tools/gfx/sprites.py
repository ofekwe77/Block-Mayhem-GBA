"""Effect / UI sprites: particles, explosions, beams, cursors, ability icons."""
import math

# Shared effect palette (16 colours, index 0 transparent).
FX_PAL = [
    (0, 0, 0),
    (255, 255, 255),    # 1 white
    (255, 240, 120),    # 2 yellow
    (255, 160, 40),     # 3 orange
    (232, 56, 32),      # 4 red
    (120, 24, 24),      # 5 dark red
    (96, 224, 255),     # 6 cyan
    (48, 96, 240),      # 7 blue
    (200, 120, 255),    # 8 violet
    (120, 240, 120),    # 9 green
    (176, 176, 196),    # 10 light grey
    (96, 96, 120),      # 11 grey
    (48, 48, 64),       # 12 dark grey
    (16, 16, 32),       # 13 near black
    (255, 200, 220),    # 14 pink
    (208, 248, 255),    # 15 ice white
]


def blank(w, h):
    return [[0] * w for _ in range(h)]


def put(img, x, y, c):
    if 0 <= y < len(img) and 0 <= x < len(img[0]):
        img[y][x] = c


def disc(img, cx, cy, r, c):
    for y in range(len(img)):
        for x in range(len(img[0])):
            if (x - cx + 0.5) ** 2 + (y - cy + 0.5) ** 2 <= r * r:
                img[y][x] = c


def ring(img, cx, cy, r, width, c):
    for y in range(len(img)):
        for x in range(len(img[0])):
            d = math.hypot(x - cx + 0.5, y - cy + 0.5)
            if r - width <= d <= r:
                img[y][x] = c


def stack(frames):
    out = []
    for f in frames:
        out.extend(f)
    return out


def particles():
    """8x8 particle frames."""
    frames = []
    # 0-3 spark burst
    for k in range(4):
        f = blank(8, 8)
        s = [2, 3, 2, 1][k]
        for d in range(s):
            put(f, 3 - d, 3, 2 if d else 1)
            put(f, 4 + d, 4, 2 if d else 1)
            put(f, 3, 3 - d, 2)
            put(f, 4, 4 + d, 2)
        put(f, 3, 3, 1)
        put(f, 4, 4, 1)
        frames.append(f)
    # 4-7 smoke puff
    for k in range(4):
        f = blank(8, 8)
        r = 1.5 + k * 0.8
        disc(f, 4, 4, r, 11 if k < 2 else 12)
        disc(f, 3.5, 3.5, r * 0.5, 10 if k < 3 else 11)
        frames.append(f)
    # 8-11 ember
    for k in range(4):
        f = blank(8, 8)
        disc(f, 4, 4, 2.2 - k * 0.4, [2, 3, 3, 4][k])
        put(f, 4, 4, 1 if k < 2 else 2)
        frames.append(f)
    # 12-13 snowflake
    for k in range(2):
        f = blank(8, 8)
        for d in range(-2, 3):
            put(f, 4 + d, 4, 15)
            put(f, 4, 4 + d, 15)
            if k:
                put(f, 4 + d, 4 + d, 6)
        put(f, 4, 4, 1)
        frames.append(f)
    # 14-15 crystal shard
    for k in range(2):
        f = blank(8, 8)
        pts = [(3, 1), (4, 1), (3, 2), (4, 2), (2, 3), (5, 3), (3, 3), (4, 3), (3, 4), (4, 4), (3, 5), (4, 5), (4, 6)]
        for (x, y) in pts:
            put(f, x if not k else 7 - x, y, 6 if (x + y) % 2 else 15)
        frames.append(f)
    # 16-19 debris chunk (rotating square)
    for k in range(4):
        f = blank(8, 8)
        for y in range(2, 6):
            for x in range(2, 6):
                if k % 2 == 0 or abs(x - 3.5) + abs(y - 3.5) <= 2.5:
                    put(f, x, y, 10 if (x + y + k) % 3 else 12)
        frames.append(f)
    # 20-21 star twinkle
    for k in range(2):
        f = blank(8, 8)
        put(f, 4, 4, 1)
        for d in (1, 2):
            c = 2 if d == 1 else (8 if k else 0)
            put(f, 4 + d, 4, c)
            put(f, 4 - d, 4, c)
            put(f, 4, 4 + d, c)
            put(f, 4, 4 - d, c)
        frames.append(f)
    # 22-23 electric bit
    for k in range(2):
        f = blank(8, 8)
        path = [(1, 4), (2, 3), (3, 4), (4, 2), (5, 5), (6, 3)] if k == 0 else [(1, 3), (2, 5), (3, 3), (4, 5), (5, 2), (6, 4)]
        for (x, y) in path:
            put(f, x, y, 6)
            put(f, x, y + 1, 1)
        frames.append(f)
    # 24-27 coloured block shards (white/yellow/cyan/violet)
    for c in (1, 2, 6, 8):
        f = blank(8, 8)
        for (x, y) in [(3, 2), (4, 2), (2, 3), (3, 3), (4, 3), (5, 3), (3, 4), (4, 4)]:
            put(f, x, y, c)
        put(f, 3, 2, 1)
        frames.append(f)
    # 28-29 cosmic dust
    for k in range(2):
        f = blank(8, 8)
        put(f, 3 + k, 3, 8)
        put(f, 4, 4 - k, 14)
        put(f, 2, 5, 7)
        frames.append(f)
    # 30-31 heart / plus pickup glints
    f = blank(8, 8)
    for (x, y) in [(2, 2), (3, 2), (5, 2), (6, 2), (1, 3), (2, 3), (3, 3), (4, 3), (5, 3), (6, 3), (7, 3),
                   (2, 4), (3, 4), (4, 4), (5, 4), (6, 4), (3, 5), (4, 5), (5, 5), (4, 6)]:
        put(f, x, y, 4)
    frames.append(f)
    f = blank(8, 8)
    for d in range(-2, 3):
        put(f, 4 + d, 4, 9)
        put(f, 4, 4 + d, 9)
    frames.append(f)
    return frames


def explosion():
    """32x32 explosion frames: flash, fireball, ring, smoke."""
    frames = []
    specs = [
        (6, 0, 1, 2),
        (9, 0, 2, 3),
        (12, 10, 3, 4),
        (14, 12, 4, 5),
        (15, 13.5, 11, 12),
        (15.5, 14.5, 12, 13),
    ]
    for k, (r_out, r_ring, c_in, c_ring) in enumerate(specs):
        f = blank(32, 32)
        if r_ring:
            ring(f, 16, 16, r_out, 2.0, c_ring)
            if k < 4:
                disc(f, 16, 16, r_out - 3, c_in)
                disc(f, 15, 15, (r_out - 3) * 0.55, 2)
        else:
            disc(f, 16, 16, r_out, c_ring)
            disc(f, 16, 16, r_out - 2, c_in)
            disc(f, 15, 15, r_out * 0.4, 1)
        # Debris specks.
        for i in range(8):
            a = i * math.pi / 4 + k * 0.3
            d = r_out + 1
            put(f, int(16 + math.cos(a) * d), int(16 + math.sin(a) * d), 2 if k < 3 else 11)
        frames.append(f)
    return frames


def beam():
    """16x32 laser beam segments (2 frames) + 16x32 beam head."""
    frames = []
    for k in range(2):
        f = blank(16, 32)
        for y in range(32):
            wob = 1 if ((y // 4 + k) % 2) else 0
            for x in range(16):
                d = abs(x - 7.5)
                if d < 1.5:
                    f[y][x] = 1
                elif d < 3 + wob:
                    f[y][x] = 6
                elif d < 5 + wob:
                    f[y][x] = 7
                elif d < 6 and (y + k) % 3 == 0:
                    f[y][x] = 8
        frames.append(f)
    f = blank(16, 32)
    disc(f, 8, 26, 7, 7)
    disc(f, 8, 26, 5, 6)
    disc(f, 8, 26, 3, 1)
    for y in range(26):
        for x in range(6, 10):
            f[y][x] = 1 if 7 <= x <= 8 else 6
    frames.append(f)
    return frames


def cursor():
    """16x16 targeting cursor (2 frames) and column marker."""
    frames = []
    for k in range(2):
        f = blank(16, 16)
        o = k
        for i in range(5):
            put(f, o + i, o, 4)
            put(f, o, o + i, 4)
            put(f, 15 - o - i, o, 4)
            put(f, 15 - o, o + i, 4)
            put(f, o + i, 15 - o, 4)
            put(f, o, 15 - o - i, 4)
            put(f, 15 - o - i, 15 - o, 4)
            put(f, 15 - o, 15 - o - i, 4)
        put(f, 7, 7, 1)
        put(f, 8, 8, 1)
        put(f, 7, 8, 2)
        put(f, 8, 7, 2)
        frames.append(f)
    # Column marker (down arrow) frames
    for k in range(2):
        f = blank(16, 16)
        for y in range(8):
            for x in range(16):
                if abs(x - 7.5) <= 7 - y and y >= 2 + k:
                    f[y + 6][x] = 4 if abs(x - 7.5) < 7 - y - 1 else 13
        frames.append(f)
    return frames


def icons():
    """16x16 ability icons: laser, freeze, gravity, shift, nuke, shield, chain spark, vent, locked."""
    frames = []

    def frame():
        f = blank(16, 16)
        for y in range(16):
            for x in range(16):
                edge = x in (0, 15) or y in (0, 15)
                corner = (x in (0, 15) and y in (0, 15))
                if corner:
                    continue
                f[y][x] = 13 if edge else (12 if (x + y) % 2 else 13)
        for x in range(1, 15):
            f[1][x] = 11
        return f

    # Laser: vertical beam with star.
    f = frame()
    for y in range(3, 14):
        put(f, 7, y, 1)
        put(f, 8, y, 6)
        put(f, 6, y, 7)
        put(f, 9, y, 7)
    for d in range(-3, 4):
        put(f, 7 + d, 4, 2)
    frames.append(f)
    # Time freeze: clock.
    f = frame()
    ring(f, 8, 8, 6, 1.3, 15)
    disc(f, 8, 8, 4.6, 7)
    for y in range(4, 9):
        put(f, 7, y, 1)
    for x in range(7, 11):
        put(f, x, 8, 1)
    frames.append(f)
    # Gravity pulse: four arrows.
    f = frame()
    for d in range(4):
        put(f, 7, 3 + d, 8)
        put(f, 8, 3 + d, 8)
        put(f, 7, 12 - d, 8)
        put(f, 8, 12 - d, 8)
    for i in range(3):
        put(f, 6 - i, 4 + i, 8)
        put(f, 9 + i, 4 + i, 8)
        put(f, 6 - i, 11 - i, 8)
        put(f, 9 + i, 11 - i, 8)
    disc(f, 8, 8, 1.6, 1)
    frames.append(f)
    # Shape shift: two blocks with swap arrows.
    f = frame()
    for y in range(4, 8):
        for x in range(3, 7):
            put(f, x, y, 9)
    for y in range(8, 12):
        for x in range(9, 13):
            put(f, x, y, 6)
    for i in range(4):
        put(f, 9 + i, 4, 2)
        put(f, 3 + i, 11, 2)
    put(f, 12, 5, 2)
    put(f, 3, 10, 2)
    frames.append(f)
    # Nuclear burst: radiation trefoil.
    f = frame()
    disc(f, 8, 8, 6, 2)
    for y in range(16):
        for x in range(16):
            a = math.atan2(y - 7.5, x - 7.5)
            d = math.hypot(x - 7.5, y - 7.5)
            if 2 < d < 6 and int((a + math.pi) / (math.pi / 3)) % 2 == 0:
                f[y][x] = 13
    disc(f, 8, 8, 1.5, 13)
    frames.append(f)
    # Shield matrix: hexagon shield.
    f = frame()
    for y in range(3, 14):
        w = 5 if y < 9 else 5 - (y - 9)
        for x in range(8 - w, 8 + w):
            put(f, x, y, 6 if (x + y) % 3 else 15)
        put(f, 8 - w, y, 7)
        put(f, 7 + w, y, 7)
    frames.append(f)
    # Chain spark: charged cell.
    f = frame()
    for y in range(5, 11):
        for x in range(5, 11):
            put(f, x, y, 11)
    for (x, y) in [(4, 4), (11, 4), (4, 11), (11, 11), (8, 2), (8, 13), (2, 8), (13, 8)]:
        put(f, x, y, 6)
    put(f, 7, 7, 2)
    put(f, 8, 8, 2)
    put(f, 8, 7, 1)
    frames.append(f)
    # Emergency vent: upward vent arrows.
    f = frame()
    for i in range(3):
        for d in range(4):
            put(f, 4 + i * 4, 12 - d, 3)
        put(f, 3 + i * 4, 10, 3)
        put(f, 5 + i * 4, 10, 3)
        put(f, 4 + i * 4, 7, 2)
    for x in range(2, 14):
        put(f, x, 13, 11)
    frames.append(f)
    # Locked.
    f = frame()
    ring(f, 8, 6, 3.5, 1.2, 10)
    for y in range(7, 13):
        for x in range(4, 12):
            put(f, x, y, 11)
    put(f, 8, 9, 13)
    put(f, 8, 10, 13)
    frames.append(f)
    return frames


def arrows():
    """16x16 gravity arrows: down, left, right, up."""
    frames = []
    base = blank(16, 16)
    for y in range(16):
        for x in range(16):
            if 5 <= x <= 10 and y <= 8:
                base[y][x] = 8
            if y > 8 and abs(x - 7.5) <= 15 - y:
                base[y][x] = 8
    for y in range(16):
        for x in range(16):
            if base[y][x] and (x in (5, 10) or y == 0):
                base[y][x] = 14
    down = base
    up = list(reversed(down))
    left = [[down[x][15 - y] for x in range(16)] for y in range(16)]
    right = [list(reversed(r)) for r in left]
    return [down, left, right, up]


def generate(gfx_dir, write_bmp, write_json):
    import os

    def out(n):
        return os.path.join(gfx_dir, n)

    def save(name, frames, w, h):
        px = stack(frames)
        write_bmp(out(name + ".bmp"), w, len(px), px, FX_PAL, bpp=4)
        write_json(out(name + ".json"), {"type": "sprite", "height": h})

    save("spr_particles", particles(), 8, 8)
    save("spr_explosion", explosion(), 32, 32)
    save("spr_beam", beam(), 16, 32)
    save("spr_cursor", cursor(), 16, 16)
    save("spr_icons", icons(), 16, 16)
    save("spr_arrows", arrows(), 16, 16)
