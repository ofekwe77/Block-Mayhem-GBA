"""Secret-event and easter-egg sprites (PART 35): a tiny maintenance robot, the dizzy face of a
defeated boss, lightning bolts for the chain storm and a chili pepper for the title secret.
Original pixel art drawn with the shared effect palette (sprites.FX_PAL)."""
import random

from sprite_art import Canvas, sheet
from sprites import FX_PAL

# FX_PAL indices
WHITE, YELLOW, ORANGE, RED, DARK_RED, CYAN, BLUE, VIOLET, GREEN = 1, 2, 3, 4, 5, 6, 7, 8, 9
LIGHT, GREY, DARK, INK, PINK, ICE = 10, 11, 12, 13, 14, 15


def robot_frame(legs=0, wave=0, hop=0):
    """16x16 robot. legs: 0 stand, 1/2 walking steps. wave: 0 arms down, 1 right arm up, 2 right arm high.
    hop: pixels lifted (cheer)."""
    c = Canvas(16, 16)
    y0 = 1 - hop
    # Antenna with its signal light.
    c.set(8, y0, YELLOW)
    c.set(8, y0 + 1, DARK)
    # Head and visor eyes.
    c.rect(4, y0 + 2, 12, y0 + 7, LIGHT)
    c.rect(4, y0 + 6, 12, y0 + 7, GREY)
    c.rect(5, y0 + 3, 7, y0 + 5, CYAN)
    c.rect(9, y0 + 3, 11, y0 + 5, CYAN)
    c.set(5, y0 + 3, WHITE)
    c.set(9, y0 + 3, WHITE)
    for x in (6, 8, 10):
        c.set(x, y0 + 6, DARK)      # mouth grille
    # Body with a chest light.
    c.rect(5, y0 + 8, 11, y0 + 12, GREY)
    c.rect(5, y0 + 8, 11, y0 + 9, LIGHT)
    c.set(7, y0 + 10, ORANGE)
    c.set(8, y0 + 10, RED)
    # Arms.
    c.rect(3, y0 + 8, 4, y0 + 11, LIGHT)
    if wave == 0:
        c.rect(12, y0 + 8, 13, y0 + 11, LIGHT)
    elif wave == 1:
        c.rect(12, y0 + 6, 13, y0 + 9, LIGHT)
        c.set(13, y0 + 5, LIGHT)
    else:
        c.rect(12, y0 + 3, 13, y0 + 8, LIGHT)
        c.set(13, y0 + 2, LIGHT)
    # Legs.
    if legs == 0:
        c.rect(6, y0 + 12, 7, y0 + 14, DARK)
        c.rect(9, y0 + 12, 10, y0 + 14, DARK)
    elif legs == 1:
        c.rect(5, y0 + 12, 6, y0 + 14, DARK)
        c.rect(9, y0 + 12, 10, y0 + 13, DARK)
    else:
        c.rect(6, y0 + 12, 7, y0 + 13, DARK)
        c.rect(10, y0 + 12, 11, y0 + 14, DARK)
    c.outline(INK)
    return c


def robot():
    return [
        robot_frame(0, 0),      # 0 stand
        robot_frame(1, 0),      # 1 walk A
        robot_frame(2, 0),      # 2 walk B
        robot_frame(0, 1),      # 3 wave low
        robot_frame(0, 2),      # 4 wave high
        robot_frame(0, 2, 1),   # 5 cheer (hop)
    ]


def spiral_eye(c, cx, cy, turn):
    c.ring(cx, cy, 4.2, 1.2, INK)
    c.ring(cx, cy, 2.2, 1.0, INK)
    # A gap in each ring makes it read as a spiral; 'turn' rotates the gaps.
    gaps = [(1, -4), (4, 1), (-1, 4), (-4, -1)]
    gx, gy = gaps[turn % 4]
    c.set(cx + gx, cy + gy, YELLOW)
    gx2, gy2 = gaps[(turn + 2) % 4]
    c.set(cx + gx2 // 2, cy + gy2 // 2, YELLOW)
    c.set(cx, cy, INK)


def dizzy_frame(kind):
    """32x32 'defeated boss' face: YELLOW / ORANGE are the face colours (recoloured per boss at run time
    when a sprite palette is free; the shared effect palette shows a yellow face otherwise)."""
    c = Canvas(32, 32)
    c.disc(16, 17, 13, YELLOW)
    c.ellipse(16, 21, 11, 7, YELLOW)
    # Cheeks.
    c.ellipse(8, 21, 2.5, 1.6, PINK)
    c.ellipse(24, 21, 2.5, 1.6, PINK)
    if kind in (0, 1):
        spiral_eye(c, 11, 14, kind)
        spiral_eye(c, 21, 14, kind + 1)
    else:
        for (ex, ey) in ((11, 14), (21, 14)):
            c.line(ex - 3, ey - 3, ex + 3, ey + 3, INK, 2)
            c.line(ex - 3, ey + 3, ex + 3, ey - 3, INK, 2)
    # Wobbly open mouth with the tongue hanging out.
    mouth = [(11, 22), (14, 21), (18, 22), (21, 21), (21, 24), (11, 24)]
    c.poly(mouth, INK)
    tx = 17 if kind != 1 else 14
    c.ellipse(tx, 26, 2.6, 2.6, PINK)
    c.rect(tx - 2, 24, tx + 3, 26, PINK)
    c.set(tx, 25, RED)
    c.set(tx, 26, RED)
    if kind == 2:
        # Sweat drop.
        c.ellipse(27, 8, 1.8, 2.4, CYAN)
        c.set(27, 5, CYAN)
        c.set(26, 7, ICE)
    c.outline(INK)
    c.shade([ORANGE, YELLOW], light_from=(-1, -1))
    return c


def dizzy():
    return [dizzy_frame(0), dizzy_frame(1), dizzy_frame(2)]


def bolt_frame(seed):
    """32x64 lightning bolt: violet glow, cyan body, white core, one fork."""
    rnd = random.Random(seed)
    pts = [(16, 0)]
    y = 0
    x = 16
    while y < 52:
        y = min(52, y + rnd.randint(6, 11))
        x = max(5, min(26, x + rnd.choice((-7, -5, -4, 4, 5, 7))))
        pts.append((x, y))
    pts.append((16, 63))     # every frame starts and ends at the centre column, so bolts stack seamlessly
    fork_at = pts[len(pts) // 2]
    fork = [fork_at, (fork_at[0] + (6 if fork_at[0] < 16 else -6), fork_at[1] + 9),
            (fork_at[0] + (4 if fork_at[0] < 16 else -4), fork_at[1] + 17)]
    c = Canvas(32, 64)
    for path, widths in ((pts, (4, 2, 1)), (fork, (3, 1, 1))):
        for width, color in zip(widths, (VIOLET, CYAN, WHITE)):
            for a, b in zip(path, path[1:]):
                c.line(a[0], a[1], b[0], b[1], color, width)
    return c


def bolts():
    return [bolt_frame(7), bolt_frame(29)]


def chili_frame(flip):
    c = Canvas(16, 16)
    body = [(4, 5), (7, 3), (10, 5), (12, 9), (12, 13), (10, 14), (8, 11), (5, 8)]
    c.poly(body, RED)
    c.line(9, 12, 11, 13, DARK_RED, 1)
    c.line(11, 9, 11, 12, DARK_RED, 1)
    c.set(6, 5, ORANGE)
    c.set(7, 5, YELLOW)
    c.set(6, 6, ORANGE)
    # Stem and cap.
    c.rect(4, 3, 7, 4, GREEN)
    c.set(5, 2, GREEN)
    c.set(4, 1, GREEN)
    c.outline(INK)
    return c.mirror() if flip else c


def chilies():
    return [chili_frame(False), chili_frame(True)]


def generate(gfx_dir, write_bmp, write_json):
    import os

    def save(name, frames, w, h):
        px = sheet(frames)
        write_bmp(os.path.join(gfx_dir, name + ".bmp"), w, len(px), px, FX_PAL, bpp=4)
        write_json(os.path.join(gfx_dir, name + ".json"), {"type": "sprite", "height": h})

    save("spr_robot", robot(), 16, 16)
    save("spr_dizzy", dizzy(), 32, 32)
    save("spr_bolt", bolts(), 32, 64)
    save("spr_chili", chilies(), 16, 16)
