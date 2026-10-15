"""Bank-aware procedural pixel painter for GBA 4bpp backgrounds.

A Layer holds global palette indices (bank * 16 + index). Drawing wraps
horizontally so layers scroll seamlessly. finalize() enforces the hardware
rule "one 16-colour bank per 8x8 tile" by remapping minority pixels to the
nearest colour of the tile's majority bank.
"""
import math

BAYER4 = [
    [0, 8, 2, 10],
    [12, 4, 14, 6],
    [3, 11, 1, 9],
    [15, 7, 13, 5],
]


class Palette:
    def __init__(self, banks):
        self.banks = banks          # list of lists of 16 (r, g, b)
        self.names = {}

    def name(self, label, bank, index):
        self.names[label] = bank * 16 + index

    def __getitem__(self, label):
        if isinstance(label, int):
            return label
        return self.names[label]

    def rgb(self, gi):
        return self.banks[gi // 16][gi % 16]

    def flat(self):
        out = []
        for b in self.banks:
            out.extend(b)
        return out


class Layer:
    def __init__(self, pal, w=256, h=256):
        self.pal = pal
        self.w = w
        self.h = h
        self.px = [[0] * w for _ in range(h)]

    def set(self, x, y, c):
        if 0 <= y < self.h:
            self.px[y][int(x) % self.w] = self.pal[c]

    def get(self, x, y):
        return self.px[y][int(x) % self.w]

    def fill(self, c):
        ci = self.pal[c]
        for row in self.px:
            for x in range(self.w):
                row[x] = ci

    def rect(self, x0, y0, x1, y1, c):
        ci = self.pal[c]
        for y in range(max(0, y0), min(self.h, y1)):
            row = self.px[y]
            for x in range(x0, x1):
                row[x % self.w] = ci

    def vgradient(self, x0, y0, x1, y1, colors):
        """Dithered vertical gradient through the given list of colours."""
        n = len(colors) - 1
        for y in range(max(0, y0), min(self.h, y1)):
            t = (y - y0) / max(1, (y1 - y0 - 1)) * n
            for x in range(x0, x1):
                k = int(t)
                frac = t - k
                c = colors[min(n, k + (1 if frac * 16 > BAYER4[y % 4][x % 4] else 0))]
                self.set(x, y, c)

    def hgradient(self, x0, y0, x1, y1, colors):
        n = len(colors) - 1
        for x in range(x0, x1):
            t = (x - x0) / max(1, (x1 - x0 - 1)) * n
            for y in range(max(0, y0), min(self.h, y1)):
                k = int(t)
                frac = t - k
                c = colors[min(n, k + (1 if frac * 16 > BAYER4[y % 4][x % 4] else 0))]
                self.set(x, y, c)

    def disc(self, cx, cy, r, c):
        for y in range(int(cy - r - 1), int(cy + r + 2)):
            for x in range(int(cx - r - 1), int(cx + r + 2)):
                if (x - cx) ** 2 + (y - cy) ** 2 <= r * r:
                    self.set(x, y, c)

    def ring(self, cx, cy, r, width, c):
        for y in range(int(cy - r - 1), int(cy + r + 2)):
            for x in range(int(cx - r - 1), int(cx + r + 2)):
                d = math.hypot(x - cx, y - cy)
                if r - width <= d <= r:
                    self.set(x, y, c)

    def line(self, x0, y0, x1, y1, c, width=1):
        steps = int(max(abs(x1 - x0), abs(y1 - y0))) + 1
        for i in range(steps + 1):
            t = i / max(1, steps)
            x = x0 + (x1 - x0) * t
            y = y0 + (y1 - y0) * t
            for dy in range(width):
                for dx in range(width):
                    self.set(int(round(x)) + dx, int(round(y)) + dy, c)

    def poly(self, pts, c):
        ys = [p[1] for p in pts]
        for y in range(int(min(ys)), int(max(ys)) + 1):
            xs = []
            n = len(pts)
            for i in range(n):
                (x0, y0), (x1, y1) = pts[i], pts[(i + 1) % n]
                if (y0 <= y < y1) or (y1 <= y < y0):
                    xs.append(x0 + (y - y0) * (x1 - x0) / (y1 - y0))
            xs.sort()
            for i in range(0, len(xs) - 1, 2):
                for x in range(int(math.ceil(xs[i])), int(math.floor(xs[i + 1])) + 1):
                    self.set(x, y, c)

    def gear(self, cx, cy, r_in, r_out, teeth, body, tooth_colors, hub, hole):
        """Gear whose teeth use a cycle of colours (palette cycling makes it spin)."""
        for y in range(int(cy - r_out - 1), int(cy + r_out + 2)):
            for x in range(int(cx - r_out - 1), int(cx + r_out + 2)):
                d = math.hypot(x - cx, y - cy)
                a = (math.atan2(y - cy, x - cx) + math.pi) / (2 * math.pi)
                seg = a * teeth
                tooth = (seg % 1.0) < 0.5
                if d <= r_in:
                    self.set(x, y, body)
                elif d <= r_out and tooth:
                    k = int(seg) % len(tooth_colors)
                    self.set(x, y, tooth_colors[k])
        self.ring(cx, cy, r_in, 1.2, tooth_colors[0])
        self.disc(cx, cy, r_in * 0.45, hub)
        self.disc(cx, cy, r_in * 0.18, hole)

    def text(self, x, y, s, c, scale=1):
        import font as fnt
        cx = x
        for ch in s:
            rows = fnt.glyph(ch)
            for ry, r in enumerate(rows):
                for rx, v in enumerate(r):
                    if v == "#":
                        for dy in range(scale):
                            for dx in range(scale):
                                self.set(cx + rx * scale + dx, y + ry * scale + dy, c)
            cx += (fnt.glyph_width(rows) + 1) * scale

    def noise(self, x0, y0, x1, y1, colors, seed, density):
        import random
        rnd = random.Random(seed)
        for y in range(y0, y1):
            for x in range(x0, x1):
                if rnd.random() < density:
                    self.set(x, y, colors[rnd.randrange(len(colors))])

    def finalize(self):
        """Enforce one bank per 8x8 tile. Returns the pixel rows."""
        pal = self.pal
        for ty in range(0, self.h, 8):
            for tx in range(0, self.w, 8):
                counts = {}
                for y in range(ty, ty + 8):
                    for x in range(tx, tx + 8):
                        gi = self.px[y][x]
                        if gi % 16:
                            counts[gi // 16] = counts.get(gi // 16, 0) + 1
                if not counts:
                    continue
                bank = max(counts, key=lambda b: counts[b])
                for y in range(ty, ty + 8):
                    for x in range(tx, tx + 8):
                        gi = self.px[y][x]
                        if gi % 16 == 0 or gi // 16 == bank:
                            continue
                        r, g, b = pal.rgb(gi)
                        best = None
                        best_d = None
                        for i in range(1, 16):
                            rr, gg, bb = pal.banks[bank][i]
                            d = (rr - r) ** 2 + (gg - g) ** 2 + (bb - b) ** 2
                            if best_d is None or d < best_d:
                                best_d = d
                                best = i
                        self.px[y][x] = bank * 16 + best
        return self.px
