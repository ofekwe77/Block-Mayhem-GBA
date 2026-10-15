"""Small indexed-sprite art toolkit: shapes, outlines and shading for 4bpp sprites."""
import math


class Canvas:
    def __init__(self, w, h):
        self.w = w
        self.h = h
        self.px = [[0] * w for _ in range(h)]

    def copy(self):
        c = Canvas(self.w, self.h)
        c.px = [list(r) for r in self.px]
        return c

    def set(self, x, y, c):
        x = int(x)
        y = int(y)
        if 0 <= x < self.w and 0 <= y < self.h:
            self.px[y][x] = c

    def get(self, x, y):
        if 0 <= x < self.w and 0 <= y < self.h:
            return self.px[y][x]
        return 0

    def rect(self, x0, y0, x1, y1, c):
        for y in range(int(y0), int(y1)):
            for x in range(int(x0), int(x1)):
                self.set(x, y, c)

    def ellipse(self, cx, cy, rx, ry, c):
        for y in range(int(cy - ry - 1), int(cy + ry + 2)):
            for x in range(int(cx - rx - 1), int(cx + rx + 2)):
                if rx > 0 and ry > 0 and ((x + 0.5 - cx) / rx) ** 2 + ((y + 0.5 - cy) / ry) ** 2 <= 1.0:
                    self.set(x, y, c)

    def disc(self, cx, cy, r, c):
        self.ellipse(cx, cy, r, r, c)

    def ring(self, cx, cy, r, width, c):
        for y in range(int(cy - r - 1), int(cy + r + 2)):
            for x in range(int(cx - r - 1), int(cx + r + 2)):
                d = math.hypot(x + 0.5 - cx, y + 0.5 - cy)
                if r - width <= d <= r:
                    self.set(x, y, c)

    def line(self, x0, y0, x1, y1, c, width=1):
        steps = int(max(abs(x1 - x0), abs(y1 - y0)) * 2) + 1
        for i in range(steps + 1):
            t = i / steps
            x = x0 + (x1 - x0) * t
            y = y0 + (y1 - y0) * t
            for dy in range(width):
                for dx in range(width):
                    self.set(x + dx - width // 2, y + dy - width // 2, c)

    def poly(self, pts, c):
        ys = [p[1] for p in pts]
        for y in range(int(min(ys)), int(max(ys)) + 1):
            xs = []
            n = len(pts)
            for i in range(n):
                (x0, y0), (x1, y1) = pts[i], pts[(i + 1) % n]
                if (y0 <= y + 0.5 < y1) or (y1 <= y + 0.5 < y0):
                    xs.append(x0 + (y + 0.5 - y0) * (x1 - x0) / (y1 - y0))
            xs.sort()
            for i in range(0, len(xs) - 1, 2):
                for x in range(int(math.ceil(xs[i] - 0.5)), int(math.floor(xs[i + 1] - 0.5)) + 1):
                    self.set(x, y, c)

    def block(self, x, y, size, light, base, dark):
        """A bevelled cube (the Block Monster is built of these)."""
        self.rect(x, y, x + size, y + size, base)
        self.rect(x, y, x + size, y + 1, light)
        self.rect(x, y, x + 1, y + size, light)
        self.rect(x, y + size - 1, x + size, y + size, dark)
        self.rect(x + size - 1, y, x + size, y + size, dark)

    def outline(self, color, diagonal=False):
        """Adds a 1px outline around every opaque pixel."""
        src = [list(r) for r in self.px]
        offs = [(1, 0), (-1, 0), (0, 1), (0, -1)]
        if diagonal:
            offs += [(1, 1), (-1, -1), (1, -1), (-1, 1)]
        for y in range(self.h):
            for x in range(self.w):
                if src[y][x]:
                    continue
                for dx, dy in offs:
                    nx, ny = x + dx, y + dy
                    if 0 <= nx < self.w and 0 <= ny < self.h and src[ny][nx] and src[ny][nx] != color:
                        self.px[y][x] = color
                        break

    def shade(self, ramp, light_from=(-1, -1)):
        """Recolours pixels of a material: edges facing the light get the next lighter shade,
        edges away get the darker shade. ramp = [darkest .. lightest] colour indices."""
        src = [list(r) for r in self.px]
        lx, ly = light_from
        for y in range(self.h):
            for x in range(self.w):
                c = src[y][x]
                if c not in ramp:
                    continue
                i = ramp.index(c)
                toward = src[y + ly][x + lx] if 0 <= y + ly < self.h and 0 <= x + lx < self.w else 0
                away = src[y - ly][x - lx] if 0 <= y - ly < self.h and 0 <= x - lx < self.w else 0
                if toward not in ramp and i + 1 < len(ramp):
                    self.px[y][x] = ramp[i + 1]
                elif away not in ramp and i > 0:
                    self.px[y][x] = ramp[i - 1]

    def mirror(self):
        c = Canvas(self.w, self.h)
        c.px = [list(reversed(r)) for r in self.px]
        return c

    def blit(self, other, ox, oy):
        for y in range(other.h):
            for x in range(other.w):
                v = other.px[y][x]
                if v:
                    self.set(ox + x, oy + y, v)


def sheet(frames):
    rows = []
    for f in frames:
        rows.extend(f.px)
    return rows
