"""The six bosses: original multi-part sprite art (PART 23.5, 51).

Each boss is a rig of independently animated parts (body, eyes, mouth, arms,
heads, wings, rings...). Each boss has its own 16-colour palette; every part
sheet stacks its animation frames vertically. The runtime rigs live in
src/bm_boss_view.cpp.
"""
import math
import os

from sprite_art import Canvas, sheet


def save(gfx_dir, write_bmp, write_json, name, frames, palette):
    w, h = frames[0].w, frames[0].h
    write_bmp(os.path.join(gfx_dir, name + ".bmp"), w, h * len(frames), sheet(frames), palette, bpp=4)
    write_json(os.path.join(gfx_dir, name + ".json"), {"type": "sprite", "height": h})


def save_palette(gfx_dir, write_bmp, write_json, name, palette):
    write_bmp(os.path.join(gfx_dir, name + ".bmp"), 8, 8, [[0] * 8 for _ in range(8)], palette, bpp=4)
    write_json(os.path.join(gfx_dir, name + ".json"), {"type": "sprite_palette", "bpp_mode": "bpp_4"})


# ============================================================================ 1. THE BLOCK MONSTER

P1 = [(0, 0, 0), (16, 12, 24), (56, 60, 80), (96, 104, 128), (150, 160, 184), (220, 226, 240),
      (224, 56, 48), (130, 24, 32), (248, 200, 48), (170, 120, 24), (48, 200, 232), (24, 110, 150),
      (176, 72, 232), (255, 240, 120), (255, 64, 32), (40, 8, 16)]
# indices
O1, SD, ST, SL, SH = 1, 2, 3, 4, 5
RED, RDK, YEL, YDK, CYA, CDK, VIO, GLOW, HOT, MDK = 6, 7, 8, 9, 10, 11, 12, 13, 14, 15


def monster_block(c, x, y, s, color):
    pairs = {RED: (YEL, RDK), YEL: (SH, YDK), CYA: (SH, CDK), VIO: (SH, O1), ST: (SL, SD)}
    light, dark = pairs[color]
    c.block(x, y, s, light, color, dark)


def monster_body():
    c = Canvas(64, 64)
    # Silhouette built from 8px cubes (symmetric), steel joints between them.
    layout = [
        "...ryyr...",
        "..ycSSyc..",
        ".rvSSSSvr.",
        "ryvSSSSvyr",
        "ccrSSSScc.",
        ".vyrSSryv.",
        "..ryccyr..",
        "..s....s..",
    ]
    colors = {"r": RED, "y": YEL, "c": CYA, "v": VIO, "S": ST, "s": ST}
    for gy, row in enumerate(layout):
        for gx, ch in enumerate(row):
            if ch == ".":
                continue
            x = gx * 7 - 3
            y = gy * 8
            if ch in "Ss":
                c.rect(x, y, x + 7, y + 8, ST)
            else:
                monster_block(c, x, y, 7, colors[ch])
    # Face plate (eyes and mouth sprites sit on it).
    c.rect(18, 10, 46, 36, SD)
    c.rect(19, 11, 45, 35, ST)
    c.rect(19, 11, 45, 12, SL)
    for (rx, ry) in ((20, 12), (43, 12), (20, 33), (43, 33)):
        c.set(rx, ry, SH)
    # Shoulder joints.
    for sx in (6, 57):
        c.disc(sx, 20, 5, SD)
        c.disc(sx, 20, 3, SL)
        c.set(sx - 1, 19, SH)
    # Feet.
    c.rect(14, 56, 26, 64, SD)
    c.rect(38, 56, 50, 64, SD)
    c.rect(14, 56, 26, 58, SL)
    c.rect(38, 56, 50, 58, SL)
    c.outline(O1)
    return c


def monster_eye(state):
    c = Canvas(16, 16)
    c.disc(8, 8, 6.5, SD)
    c.disc(8, 8, 5.5, MDK)
    if state == 0:      # open
        c.disc(8, 8, 4.5, GLOW)
        c.disc(8, 8, 2.5, HOT)
        c.disc(9, 8, 1.2, O1)
        c.set(6, 6, SH)
    elif state == 1:    # half closed
        c.disc(8, 9, 4.5, GLOW)
        c.disc(8, 9, 2.0, HOT)
        c.rect(2, 2, 14, 8, ST)
        c.rect(2, 7, 14, 8, SL)
    elif state == 2:    # closed lid
        c.disc(8, 8, 5.5, ST)
        c.rect(3, 8, 13, 9, SD)
    else:               # angry: red, slanted brow
        c.disc(8, 9, 4.5, HOT)
        c.disc(8, 9, 2.2, GLOW)
        c.poly([(1, 2), (15, 6), (15, 8), (1, 5)], SD)
    c.outline(O1)
    return c


def monster_mouth(state):
    c = Canvas(32, 16)
    c.rect(2, 3, 30, 13, SD)
    if state == 0:
        for x in range(4, 28, 3):
            c.rect(x, 5, x + 2, 11, ST)
    else:
        h = 3 if state == 1 else 5
        c.rect(4, 8 - h, 28, 8 + h, MDK)
        c.rect(6, 9 - h // 2, 26, 7 + h // 2 + 1, HOT if state == 2 else RDK)
        for x in range(5, 27, 4):
            c.poly([(x, 8 - h), (x + 3, 8 - h), (x + 1.5, 8 - h + 3)], SH)
            c.poly([(x + 2, 8 + h), (x + 5, 8 + h), (x + 3.5, 8 + h - 3)], SH)
    c.outline(O1)
    return c


def monster_arm(pose):
    c = Canvas(32, 32)
    if pose == 0:       # resting: hanging down
        c.rect(18, 2, 24, 16, ST)
        monster_block(c, 14, 14, 9, RED)
        monster_block(c, 13, 22, 9, YEL)
    elif pose == 1:     # wind-up: raised high
        c.line(24, 26, 20, 10, ST, 5)
        monster_block(c, 12, 2, 10, RED)
        monster_block(c, 20, 6, 8, YEL)
    else:               # punch: extended to the left (towards the board)
        c.rect(10, 14, 30, 20, ST)
        monster_block(c, 0, 10, 11, RED)
        monster_block(c, 2, 20, 7, YEL)
    c.shade([SD, ST, SL])
    c.outline(O1)
    return c


# ============================================================================ 2. THE CRYO TITAN

P2 = [(0, 0, 0), (8, 16, 40), (24, 40, 80), (60, 90, 140), (120, 150, 200), (200, 220, 248),
      (80, 160, 220), (140, 210, 250), (210, 240, 255), (255, 255, 255), (64, 240, 255), (200, 255, 255),
      (120, 110, 200), (30, 50, 90), (255, 120, 120), (12, 20, 40)]
NV, SB, LS, HL2 = 2, 3, 4, 5
ICD, ICE, ICL, WHT, VIS, VIC, FPU, CRK, WRN, SHD = 6, 7, 8, 9, 10, 11, 12, 13, 14, 15


def crystal(c, x, y, h, w, tilt=0):
    c.poly([(x, y), (x + w / 2 + tilt, y - h), (x + w, y)], ICE)
    c.poly([(x + w / 2, y), (x + w / 2 + tilt, y - h), (x + w, y)], ICD)
    c.line(x + w / 2 + tilt, y - h + 1, x + w / 2 + tilt * 0.5, y - 1, ICL)


def titan_body():
    c = Canvas(64, 64)
    # Massive chest plate.
    c.poly([(6, 20), (58, 20), (54, 56), (10, 56)], SB)
    c.poly([(10, 24), (54, 24), (50, 52), (14, 52)], LS)
    c.rect(28, 24, 36, 52, SB)
    for y in range(28, 50, 7):
        c.rect(16, y, 26, y + 2, NV)
        c.rect(38, y, 48, y + 2, NV)
    # Core reactor (frozen).
    c.disc(32, 36, 6, NV)
    c.disc(32, 36, 4.5, VIS)
    c.disc(31, 35, 1.8, VIC)
    # Ice crusting on the shoulders and belt.
    for (x, h) in ((4, 10), (10, 14), (16, 8), (44, 9), (50, 15), (56, 10)):
        crystal(c, x, 22, h, 6, (x % 3) - 1)
    for x in range(12, 52, 6):
        crystal(c, x, 58, 5 + (x * 7) % 4, 5)
    c.rect(18, 56, 30, 64, NV)
    c.rect(34, 56, 46, 64, NV)
    c.shade([NV, SB, LS])
    c.outline(1)
    return c


def titan_head(state):
    c = Canvas(32, 32)
    c.poly([(6, 6), (26, 6), (29, 24), (3, 24)], SB)
    c.poly([(8, 8), (24, 8), (26, 22), (6, 22)], LS)
    visor = {0: VIS, 1: WHT, 2: VIS, 3: WRN}[state]
    c.rect(7, 12, 25, 17, NV)
    c.rect(8, 13, 24, 16, visor)
    c.rect(9, 13, 14, 14, VIC if state != 3 else WHT)
    if state == 2:          # cracked visor
        c.line(12, 13, 16, 16, CRK)
        c.line(16, 16, 20, 13, CRK)
    if state == 3:          # angry brow plates
        c.poly([(6, 9), (15, 12), (6, 12)], NV)
        c.poly([(26, 9), (17, 12), (26, 12)], NV)
    c.rect(11, 24, 21, 30, NV)     # frost breath port
    c.rect(13, 25, 19, 29, ICD if state != 1 else ICL)
    crystal(c, 6, 7, 6, 5, -1)
    crystal(c, 21, 7, 7, 5, 1)
    c.shade([NV, SB, LS])
    c.outline(1)
    return c


def titan_crystal(size):
    c = Canvas(32, 32)
    heights = [(8, 10), (14, 16), (20, 22)][size]
    crystal(c, 4, 30, heights[0], 8, -2)
    crystal(c, 11, 30, heights[1], 10, 0)
    crystal(c, 20, 30, heights[0] + 2, 8, 2)
    if size == 2:
        crystal(c, 15, 30 - heights[1] + 6, 8, 6, 1)
    c.rect(2, 29, 30, 32, SB)
    c.outline(1)
    return c


def titan_breath(frame):
    c = Canvas(32, 16)
    reach = [12, 22, 31][frame]
    for i in range(reach):
        x = 31 - i
        spread = 1 + i * 0.22
        for y in range(int(8 - spread), int(8 + spread) + 1):
            c.set(x, y, ICL if abs(y - 8) < spread * 0.5 else ICE)
    for k in range(frame * 3 + 2):
        c.set(31 - (k * 7) % reach, 3 + (k * 5) % 10, WHT)
    return c


# ============================================================================ 3. THE MAGMA HYDRA

P3 = [(0, 0, 0), (24, 8, 8), (60, 24, 20), (100, 44, 32), (150, 72, 48), (180, 40, 16), (240, 96, 24),
      (255, 168, 48), (255, 232, 120), (255, 255, 220), (255, 255, 80), (120, 20, 24), (180, 40, 40),
      (230, 80, 64), (80, 70, 70), (240, 230, 210)]
RK0, RK1, RK2, LV0, LV1, LV2, GLW, WHH, EYE, SC0, SC1, SC2, SMK, TTH = range(2, 16)


def hydra_base():
    c = Canvas(64, 32)
    c.ellipse(32, 22, 31, 10, RK1)
    c.ellipse(32, 20, 26, 7, LV0)
    c.ellipse(32, 19, 22, 5, LV1)
    c.ellipse(30, 18, 12, 2.5, LV2)
    for (x, r) in ((8, 6), (56, 7), (18, 4), (46, 5)):
        c.disc(x, 26, r, RK0)
        c.disc(x - 1, 25, r - 2, RK2)
    for (x, y) in ((24, 18), (38, 20), (31, 17)):
        c.disc(x, y, 1.5, GLW)
    c.shade([RK0, RK1, RK2])
    c.outline(1)
    return c


def hydra_neck():
    c = Canvas(16, 16)
    c.disc(8, 8, 6.5, SC0)
    c.disc(8, 8, 5, SC1)
    c.disc(7, 7, 2.5, SC2)
    c.rect(4, 10, 12, 12, LV0)
    c.outline(1)
    return c


def hydra_head(state):
    c = Canvas(32, 32)
    stone = state == 3
    body, dark, light = (RK1, RK0, RK2) if stone else (SC1, SC0, SC2)
    # Snout points left toward the board.
    c.poly([(2, 14), (18, 6), (30, 10), (30, 24), (18, 26), (2, 20)], body)
    c.poly([(18, 6), (30, 10), (26, 4), (20, 2)], dark)
    for x in (20, 25):
        c.poly([(x, 6), (x + 3, 0), (x + 4, 6)], dark)     # horns
    if state == 1:          # roaring
        c.poly([(2, 15), (16, 18), (2, 25)], 1)
        c.poly([(3, 17), (13, 19), (3, 23)], LV2)
        for x in range(3, 14, 3):
            c.set(x, 16, TTH)
            c.set(x, 24, TTH)
    else:
        c.line(2, 18, 15, 20, 1)
        for x in range(4, 14, 3):
            c.set(x, 18, TTH)
    if stone:
        c.line(10, 8, 16, 22, 1)
        c.line(16, 22, 24, 14, 1)
        c.disc(19, 13, 2, RK0)
    elif state == 2:        # hurt: eye shut
        c.line(16, 13, 22, 13, 1)
    else:
        c.disc(19, 13, 2.5, EYE)
        c.disc(19, 13, 1, 1)
    c.shade([dark, body, light])
    c.outline(1)
    return c


def hydra_fire(frame):
    c = Canvas(32, 16)
    reach = [10, 20, 31][frame]
    for i in range(reach):
        x = 31 - i if False else i
        spread = 1 + i * 0.25
        for y in range(int(8 - spread), int(8 + spread) + 1):
            d = abs(y - 8) / max(1.0, spread)
            c.set(31 - x, y, WHH if d < 0.3 else (GLW if d < 0.6 else LV1))
    return c


# ============================================================================ 4. THE ELECTRIC EMPEROR

P4 = [(0, 0, 0), (10, 6, 24), (40, 24, 72), (72, 44, 120), (120, 80, 180), (200, 170, 240), (64, 232, 255),
      (20, 120, 160), (255, 80, 200), (140, 30, 110), (248, 208, 64), (150, 110, 20), (180, 255, 255),
      (255, 255, 255), (255, 60, 60), (6, 10, 20)]
CH0, CH1, CH2, CHH, NC, NCD, NP, NPD, GD, GDD, FG, WH4, RD4, SCR = range(2, 16)


def emperor_body():
    c = Canvas(64, 64)
    # Throne chassis with a crown antenna.
    c.poly([(8, 60), (14, 22), (50, 22), (56, 60)], CH1)
    c.poly([(14, 60), (20, 28), (44, 28), (50, 60)], CH0)
    c.rect(22, 30, 42, 44, SCR)           # face screen recess (face sprite overlays)
    c.poly([(20, 22), (24, 8), (28, 18), (32, 4), (36, 18), (40, 8), (44, 22)], GD)
    c.poly([(30, 6), (32, 2), (34, 6)], NC)
    for (x, y) in ((24, 8), (40, 8), (32, 4)):
        c.disc(x, y, 1.5, NP)
    # Circuit traces.
    for (x0, y0, x1, y1, col) in ((10, 50, 22, 50, NC), (22, 50, 22, 46, NC), (54, 50, 42, 50, NP),
                                  (42, 50, 42, 46, NP), (16, 36, 20, 36, NC), (48, 36, 44, 36, NP),
                                  (12, 56, 30, 56, NC), (52, 56, 34, 56, NP)):
        c.line(x0, y0, x1, y1, col)
    for (x, y) in ((10, 50), (54, 50), (16, 36), (48, 36)):
        c.disc(x, y, 1.6, WH4)
    # Shoulder coils.
    for x in (10, 54):
        c.disc(x, 26, 6, CH2)
        c.ring(x, 26, 6, 1.4, NC if x < 32 else NP)
    c.shade([CH0, CH1, CH2])
    c.outline(1)
    return c


def emperor_face(state):
    c = Canvas(32, 32)
    c.rect(4, 6, 28, 26, SCR)
    c.rect(4, 6, 28, 7, NCD)
    glow = {0: FG, 1: FG, 2: RD4, 3: WH4}[state]
    # Eyes.
    if state == 2:
        c.poly([(7, 10), (14, 13), (7, 14)], glow)
        c.poly([(25, 10), (18, 13), (25, 14)], glow)
    else:
        c.rect(8, 11, 13, 14, glow)
        c.rect(19, 11, 24, 14, glow)
    # Mouth.
    if state == 1:          # smirk
        c.line(10, 20, 22, 19, glow)
        c.line(22, 19, 24, 17, glow)
    elif state == 2:        # angry
        c.line(9, 21, 23, 21, glow)
        c.line(9, 21, 9, 19, glow)
        c.line(23, 21, 23, 19, glow)
    elif state == 3:        # charging
        c.ellipse(16, 20, 5, 3, glow)
    else:
        c.line(10, 20, 22, 20, glow)
    for y in range(7, 26, 3):
        for x in range(5, 28):
            if c.get(x, y) == SCR:
                c.set(x, y, 1)
    c.outline(NC)
    return c


def emperor_orb(frame):
    c = Canvas(16, 16)
    c.disc(8, 8, 6 if frame else 5, NPD if frame else NCD)
    c.disc(8, 8, 4, NP if frame else NC)
    c.disc(7, 7, 2, WH4)
    for k in range(4):
        a = k * math.pi / 2 + frame * 0.6
        c.set(8 + math.cos(a) * 7, 8 + math.sin(a) * 7, NC)
    return c


def emperor_bolt(frame):
    c = Canvas(16, 32)
    pts = [(8, 0), (5, 8), (10, 12), (4, 20), (9, 24), (6, 31)] if frame == 0 else \
          [(7, 0), (11, 7), (5, 13), (10, 19), (5, 25), (9, 31)]
    for i in range(len(pts) - 1):
        c.line(pts[i][0], pts[i][1], pts[i + 1][0], pts[i + 1][1], NC, 3)
        c.line(pts[i][0], pts[i][1], pts[i + 1][0], pts[i + 1][1], WH4, 1)
    return c


# ============================================================================ 5. THE GRAVITY KING

P5 = [(0, 0, 0), (10, 6, 20), (40, 20, 70), (80, 40, 130), (130, 80, 190), (200, 170, 240), (150, 100, 20),
      (240, 190, 60), (255, 240, 150), (60, 230, 140), (20, 120, 80), (255, 255, 255), (200, 180, 230),
      (180, 255, 220), (255, 80, 90), (6, 4, 16)]
RB0, RB1, RB2, RBH, GK0, GK1, GK2, CG, CGD, SW, SKN, EYG, RD5, VOID = range(2, 16)


def king_body():
    c = Canvas(32, 64)
    c.poly([(10, 4), (22, 4), (30, 60), (2, 60)], RB1)
    c.poly([(13, 4), (19, 4), (24, 58), (8, 58)], RB0)
    c.poly([(2, 60), (30, 60), (26, 64), (6, 64)], RB2)
    for y in range(12, 56, 10):
        c.line(16 - y * 0.18, y, 16 + y * 0.18, y, GK1)
    c.disc(16, 20, 4, GK1)
    c.disc(16, 20, 2.5, CG)
    for (x, y) in ((6, 40), (26, 46), (10, 54), (22, 30)):
        c.set(x, y, SW)
    c.shade([RB0, RB1, RB2])
    c.outline(1)
    return c


def king_head(state):
    c = Canvas(32, 32)
    c.ellipse(16, 20, 8, 9, SKN)
    c.poly([(6, 12), (8, 2), (12, 8), (16, 0), (20, 8), (24, 2), (26, 12)], GK1)
    for (x, y) in ((8, 3), (16, 1), (24, 3)):
        c.disc(x, y, 1.6, CG)
    eye = {0: CGD, 1: EYG, 2: RD5}[state]
    if state == 2:
        c.poly([(9, 17), (15, 19), (9, 20)], eye)
        c.poly([(23, 17), (17, 19), (23, 20)], eye)
    else:
        c.ellipse(12, 19, 2, 1.4, eye)
        c.ellipse(20, 19, 2, 1.4, eye)
    c.line(12, 25, 20, 25, RB0 if state != 2 else RD5)
    c.poly([(8, 26), (16, 31), (24, 26), (16, 28)], RB2)     # beard of starlight
    c.shade([GK0, GK1, GK2])
    c.outline(1)
    return c


def king_ring():
    c = Canvas(64, 64)
    c.ring(32, 32, 30, 3, GK1)
    c.ring(32, 32, 27, 1, GK0)
    for k in range(8):
        a = k * math.pi / 4
        x, y = 32 + math.cos(a) * 28.5, 32 + math.sin(a) * 28.5
        c.disc(x, y, 3, CG if k % 2 else RB2)
        c.disc(x, y, 1.4, SW)
    for k in range(16):
        a = k * math.pi / 8 + 0.2
        c.set(32 + math.cos(a) * 24, 32 + math.sin(a) * 24, GK2)
    return c


def king_cube(frame):
    c = Canvas(16, 16)
    a = frame * math.pi / 8
    pts = [(8 + math.cos(a + k * math.pi / 2) * 6, 8 + math.sin(a + k * math.pi / 2) * 6) for k in range(4)]
    c.poly(pts, CG if frame % 2 else CGD)
    c.poly([pts[0], pts[1], (8, 8)], SW if frame == 0 else EYG)
    c.outline(1)
    return c


# ============================================================================ 6. THE CORE OF MAYHEM

P6 = [(0, 0, 0), (8, 4, 12), (40, 36, 56), (80, 76, 100), (140, 136, 168), (220, 216, 240), (240, 230, 220),
      (220, 40, 40), (255, 140, 40), (10, 0, 10), (255, 90, 30), (255, 210, 90), (120, 220, 255),
      (180, 120, 255), (150, 10, 30), (255, 255, 255)]
MT0, MT1, MT2, MTH, EWH, IRR, IRO, PUP, LVC, LVG, CRY, ELC, VEIN, WH6 = range(2, 16)


def core_eye(state):
    c = Canvas(64, 64)
    c.disc(32, 32, 30, MT0)
    c.ring(32, 32, 30, 3, MT1)
    for k in range(12):
        a = k * math.pi / 6
        c.line(32 + math.cos(a) * 26, 32 + math.sin(a) * 26, 32 + math.cos(a) * 30, 32 + math.sin(a) * 30, MT2)
    open_h = {0: 20, 1: 10, 2: 0, 3: 20}[state]
    if open_h:
        c.ellipse(32, 32, 24, open_h, EWH)
        for k in range(6):
            a = 0.5 + k * 1.0
            c.line(32 + math.cos(a) * 12, 32 + math.sin(a) * open_h * 0.5, 32 + math.cos(a) * 22,
                   32 + math.sin(a) * open_h * 0.8, VEIN)
    else:
        c.rect(8, 31, 56, 33, MT2)
    if state == 3:          # cracked shell (final phase)
        for (x0, y0, x1, y1) in ((10, 12, 22, 22), (22, 22, 18, 30), (54, 14, 44, 24), (44, 24, 48, 30),
                                 (30, 56, 34, 46)):
            c.line(x0, y0, x1, y1, LVC, 2)
    c.shade([MT0, MT1, MT2])
    c.outline(1)
    return c


def core_iris(state):
    c = Canvas(32, 32)
    c.disc(16, 16, 11, IRR)
    c.disc(16, 16, 8, IRO)
    c.disc(16, 16, 5 if state == 0 else 3, PUP)
    c.disc(13, 13, 2, WH6)
    for k in range(10):
        a = k * math.pi / 5
        c.set(16 + math.cos(a) * 9.5, 16 + math.sin(a) * 9.5, VEIN)
    return c


def core_wing(frame):
    c = Canvas(64, 32)
    lift = [-6, 0, 6][frame]
    for k in range(5):
        x0 = 58 - k * 11
        c.poly([(62, 16), (x0, 4 + lift * (k / 4) + k), (x0 - 8, 8 + lift * (k / 4) + k * 2), (60, 20)], MT1)
        c.line(62, 16, x0 - 4, 6 + lift * (k / 4) + k, MTH)
        c.disc(x0 - 6, 8 + lift * (k / 4) + k * 2, 1.5, ELC if k % 2 else CRY)
    c.shade([MT0, MT1, MT2])
    c.outline(1)
    return c


def core_ring():
    c = Canvas(64, 64)
    for k in range(12):
        a0 = k * math.pi / 6
        a1 = a0 + math.pi / 9
        pts = [(32 + math.cos(a0) * 30, 32 + math.sin(a0) * 30), (32 + math.cos(a1) * 30, 32 + math.sin(a1) * 30),
               (32 + math.cos(a1) * 24, 32 + math.sin(a1) * 24), (32 + math.cos(a0) * 26, 32 + math.sin(a0) * 26)]
        c.poly(pts, CRY if k % 3 == 0 else (ELC if k % 3 == 1 else LVC))
    c.ring(32, 32, 23, 1, MT2)
    return c


def core_tentacle(frame):
    c = Canvas(16, 32)
    for y in range(32):
        x = 8 + math.sin(y * 0.35 + frame * 1.6) * 4
        w = 3 - y / 16
        c.line(x - w, y, x + w, y, ELC if (y + frame) % 6 < 3 else MT2)
    for y in range(2, 32, 7):
        c.set(8 + math.sin(y * 0.35 + frame * 1.6) * 4, y, WH6)
    return c


def core_heart():
    c = Canvas(32, 32)
    c.disc(16, 16, 12, MT0)
    c.disc(16, 16, 10, LVC)
    c.disc(16, 16, 6, LVG)
    c.disc(14, 14, 2.5, WH6)
    for k in range(8):
        a = k * math.pi / 4
        c.line(16 + math.cos(a) * 10, 16 + math.sin(a) * 10, 16 + math.cos(a) * 14, 16 + math.sin(a) * 14, CRY)
    c.outline(1)
    return c


def generate(gfx_dir, write_bmp, write_json):
    s = lambda name, frames, pal: save(gfx_dir, write_bmp, write_json, name, frames, pal)
    s("boss1_body", [monster_body()], P1)
    s("boss1_eye", [monster_eye(i) for i in range(4)], P1)
    s("boss1_mouth", [monster_mouth(i) for i in range(3)], P1)
    s("boss1_arm", [monster_arm(i) for i in range(3)], P1)
    s("boss2_body", [titan_body()], P2)
    s("boss2_head", [titan_head(i) for i in range(4)], P2)
    s("boss2_crystal", [titan_crystal(i) for i in range(3)], P2)
    s("boss2_breath", [titan_breath(i) for i in range(3)], P2)
    s("boss3_base", [hydra_base()], P3)
    s("boss3_neck", [hydra_neck()], P3)
    s("boss3_head", [hydra_head(i) for i in range(4)], P3)
    s("boss3_fire", [hydra_fire(i) for i in range(3)], P3)
    s("boss4_body", [emperor_body()], P4)
    s("boss4_face", [emperor_face(i) for i in range(4)], P4)
    s("boss4_orb", [emperor_orb(i) for i in range(2)], P4)
    s("boss4_bolt", [emperor_bolt(i) for i in range(2)], P4)
    s("boss5_body", [king_body()], P5)
    s("boss5_head", [king_head(i) for i in range(3)], P5)
    s("boss5_ring", [king_ring()], P5)
    s("boss5_cube", [king_cube(i) for i in range(4)], P5)
    s("boss6_eye", [core_eye(i) for i in range(4)], P6)
    s("boss6_iris", [core_iris(i) for i in range(2)], P6)
    s("boss6_wing", [core_wing(i) for i in range(3)], P6)
    s("boss6_ring", [core_ring()], P6)
    s("boss6_tentacle", [core_tentacle(i) for i in range(4)], P6)
    s("boss6_heart", [core_heart()], P6)
