"""CHAOS RUN upgrade icons: 16x16 badges (frame shape/colour = rarity) with a
hand-placed 10x10 symbol per upgrade. Frame index = upgrade_id for the 23
non-ARMORY upgrades (ARMORY upgrades reuse the ability icons).
"""

PAL = [
    (0, 0, 0),          # 0 transparent
    (16, 16, 32),       # 1 k outline
    (255, 255, 255),    # 2 W white
    (176, 176, 196),    # 3 L light grey
    (96, 96, 120),      # 4 g grey
    (255, 240, 120),    # 5 Y yellow
    (255, 160, 40),     # 6 O orange
    (232, 56, 32),      # 7 R red
    (120, 24, 24),      # 8 r dark red
    (96, 224, 255),     # 9 C cyan
    (48, 96, 240),      # 10 B blue
    (200, 120, 255),    # 11 V violet
    (255, 128, 200),    # 12 P pink
    (120, 240, 120),    # 13 G green
    (232, 184, 40),     # 14 D gold
    (24, 28, 56),       # 15 n navy (badge fill)
]

CHARS = {".": None, "k": 1, "W": 2, "L": 3, "g": 4, "Y": 5, "O": 6, "R": 7, "r": 8, "C": 9, "B": 10, "V": 11,
         "P": 12, "G": 13, "D": 14, "d": 4, "n": 15}

# rarity per upgrade id (0 common, 1 rare, 2 legendary) - must match core/bm_run.cpp
RARITY = [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 2, 2, 0, 0, 0]

SYMBOLS = [
    # 0 EXPLOSIVE MASTER: bomb with a lit fuse
    [".......Y..", "......O.Y.", ".....O....", "...kkkk...", "..kddddk..", ".kdLdddk..", ".kddddddk.",
     ".kddddddk.", "..kddddk..", "...kkkk..."],
    # 1 FIRE STARTER: flame
    ["....R.....", "...RR.....", "..ROR..R..", "..ROORRR..", ".ROYOORR..", ".ROYYOOR..", ".RRYYYOR..",
     "..ROYYOR..", "..RROORR..", "...RRRR..."],
    # 2 CHAIN KING: crowned chain links
    [".D..D..D..", ".DDDDDDD..", "..........", ".OOOO.....", "O....O....", "O..OOOOO..", "O..O.O..O.",
     ".OOOOO..O.", "...O....O.", "....OOOO.."],
    # 3 HEAT ENGINE: thermometer with a rising arrow
    ["...WW.....", "...WR..G..", "...WR.GGG.", "...WR..G..", "...WR..G..", "...WR.....", "..RRRR....",
     "..RRRR....", "...RR.....", ".........."],
    # 4 SECOND CHANCE: heart with a plus
    ["..RR..RR..", ".RRRRRRRR.", ".RRRWRRRR.", ".RRWWWRRR.", ".RRRWRRRR.", "..RRRRRR..", "...RRRR...",
     "....RR....", "..........", ".........."],
    # 5 LASER FOCUS: crosshair on a beam
    ["....C.....", "....C.....", "..kkCkk...", ".k..C..k..", "CCCCRCCCC.", ".k..C..k..", "..kkCkk...",
     "....C.....", "....C.....", ".........."],
    # 6 CRYSTAL BREAKER: cracked crystal
    ["....C.....", "...CWC....", "..CCWCC...", "..CkCCC...", "..CCkCC...", "..CCCkC...", "..CCkCC...",
     "...CkC....", "....C.....", ".........."],
    # 7 ELECTRIC SURGE: lightning bolt
    [".....YY...", "....YY....", "...YW.....", "..YYYYYY..", ".....WY...", "....YY....", "...YY.....",
     "..YY......", "..Y.......", ".........."],
    # 8 MAGNETIC FIELD: magnet with field lines
    [".LL...LL..", ".RR...RR..", ".RR...RR..", ".RR...RR..", ".RRR.RRR..", "..RRRRR...", "...RRR....",
     "..........", "C.......C.", ".C.....C.."],
    # 9 TIME ENGINEER: clock
    ["..kkkkk...", ".kWWWWWk..", "kWWWkWWWk.", "kWWWkWWWk.", "kWWWkkkWk.", "kWWWWWWWk.", ".kWWWWWk..",
     "..kkkkk...", "..........", ".........."],
    # 10 COMBO BATTERY: charged battery
    ["...kk.....", ".kkkkkk...", ".kGGGGk...", ".kGGGGk...", ".kkkkkk...", ".kGGGGk...", ".kGGGGk...",
     ".kGGGGk...", ".kkkkkk...", ".........."],
    # 11 OVERHEAT EXPERT: crowned flame
    [".D..D..D..", ".DDDDDDD..", "..........", "....R.....", "...ROR....", "..ROYOR...", "..RYYYR...",
     "..ROYOR...", "...RRR....", ".........."],
    # 12 SAFE VENT: steam arrow leaving a grate
    ["....W.....", "...WWW....", "..W.W.W...", "....W.....", "....W.....", ".LLLLLLL..", ".LgLgLgL..",
     ".LLLLLLL..", "..........", ".........."],
    # 13 IRON DEFENSE: shield
    [".BBBBBBB..", "BWWBBBBBB.", "BWBBBBBBB.", "BBBBBBBBB.", "BBBBBBBBB.", ".BBBBBBB..", ".BBBBBBB..",
     "..BBBBB...", "...BBB....", "....B....."],
    # 14 RAINBOW MASTER: rainbow arc
    ["..........", "..RRRRR...", ".ROOOOOR..", "ROYYYYYOR.", "OYGGGGGYO.", "YGCCCCCGY.", "GC.....CG.",
     "C.......C.", "..........", ".........."],
    # 15 PRECISION ENGINE: target
    ["..RRRRR...", ".RWWWWWR..", "RWRRRRRWR.", "RWRWWWRWR.", "RWRWRWRWR.", "RWRWWWRWR.", "RWRRRRRWR.",
     ".RWWWWWR..", "..RRRRR...", ".........."],
    # 16 FAST CHARGER: bolt in a power ring
    ["..kkkkk...", ".kCCCCCk..", "kCCCYCCCk.", "kCCYYCCCk.", "kCYYYYYCk.", "kCCCYYCCk.", "kCCCYCCCk.",
     ".kCCCCCk..", "..kkkkk...", ".........."],
    # 17 REACTIVE ARMOR: armor plate
    [".LLLLLLL..", "LWWLLLLLg.", "LWLLgLLLg.", "LLLgLLLLg.", "LLgLLLLLg.", "LLLLLLLLg.", ".LLLLLLg..",
     "..LLLLg...", "...ggg....", ".........."],
    # 18 DOUBLE SPARK: twin bolts
    ["...YY..YY.", "..YY..YY..", ".YW..YW...", "YYYYYYYYY.", "..WY..WY..", ".YY..YY...", "YY..YY....",
     "Y...Y.....", "..........", ".........."],
    # 19 FINAL GAMBIT: skull over danger stripes
    ["..WWWWW...", ".WWWWWWW..", "WkkWWWkkW.", "WkkWWWkkW.", "WWWWkWWWW.", ".WWWWWWW..", "..WkWkW...",
     "..........", "R.R.R.R.R.", ".R.R.R.R.."],
    # 20 BOMB SUPPLY: bomb plus
    [".......G..", "...Y..GGG.", "..O....G..", ".kkkk.....", "kddddk....", "kdLddk....", "kddddk....",
     ".kkkk.....", "..........", ".........."],
    # 21 STORM SUPPLY: storm cloud plus
    [".......G..", "..LLL.GGG.", ".LLLLL.G..", "LLLLLLL...", ".LLLLL....", "...YY.....", "..YW......",
     ".YYYY.....", "...Y......", ".........."],
    # 22 HOT START: flame under a rising arrow
    ["....G.....", "...GGG....", "..G.G.G...", "....G.....", "...R......", "..ROR..R..", "..ROORRR..",
     ".ROYYOOR..", ".RRYYYRR..", "..RRRRR..."],
]


def badge(rarity):
    img = [[0] * 16 for _ in range(16)]
    border, fill = {0: (10, 15), 1: (11, 15), 2: (14, 8)}[rarity]
    for y in range(16):
        for x in range(16):
            inside = 0 <= x <= 15 and 0 <= y <= 15
            corner = (x in (0, 15) and y in (0, 1, 14, 15)) or (y in (0, 15) and x in (0, 1, 14, 15))
            if not inside or corner:
                continue
            edge = x in (0, 15) or y in (0, 15) or (x in (1, 14) and y in (1, 14))
            img[y][x] = border if edge else fill
    if rarity == 1:
        # rare: inner diamond accents
        for (x, y) in ((7, 1), (8, 1), (7, 14), (8, 14)):
            img[y][x] = 12
    if rarity == 2:
        # legendary: sparkle corners and a double border
        for (x, y) in ((1, 2), (2, 1), (13, 1), (14, 2), (1, 13), (2, 14), (13, 14), (14, 13)):
            img[y][x] = 5
        for x in range(2, 14):
            img[1][x] = 6
            img[14][x] = 6
    return img


def icons():
    frames = []
    for i, art in enumerate(SYMBOLS):
        img = badge(RARITY[i])
        for y, row in enumerate(art):
            for x, ch in enumerate(row):
                c = CHARS[ch]
                if c is not None:
                    img[3 + y][3 + x] = c
        frames.append(img)
    return frames


def generate(gfx_dir, write_bmp, write_json):
    import os
    px = []
    for f in icons():
        px.extend(f)
    write_bmp(os.path.join(gfx_dir, "spr_upgrades.bmp"), 16, len(px), px, PAL, bpp=4)
    write_json(os.path.join(gfx_dir, "spr_upgrades.json"), {"type": "sprite", "height": 16})
