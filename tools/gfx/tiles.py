"""Board-layer tiles (4bpp, multi-bank palette) for BLOCK MAYHEM.

Palette bank layout (board layer, 12 banks):
  0      frame / well / panels (world-tinted at runtime)
  1..7   shape colours I O T S Z J L
  8      garbage grey
  9      stone
  10     armour steel
  11     heat meter / HUD accents

Indices inside block banks (1..10):
  0 transparent  1 outline  2 shadow  3 base  4 light  5 highlight
  6 symbol dark  7 hot core (animated)  8 hot bright (animated)  9 flame orange
  10 electric (animated)  11 magnet red  12 magnet blue  13 rainbow (animated)
  14 ice pale  15 white
"""


def T(*rows):
    """Tile from 8 strings of hex digits ('.' = 0)."""
    out = []
    for r in rows:
        out.append([0 if ch == "." else int(ch, 16) for ch in r])
    assert len(out) == 8 and all(len(r) == 8 for r in out), rows
    return out


def overlay(base, top):
    return [[t if t else b for b, t in zip(rb, rt)] for rb, rt in zip(base, top)]


def recolor(tile, mapping):
    return [[mapping.get(v, v) for v in r] for r in tile]


BLOCK = T(
    "55555551",
    "54444431",
    "54333321",
    "54333321",
    "54333321",
    "53333321",
    "53222221",
    "11111111",
)

GARBAGE = T(
    "55555551",
    "54444431",
    "54322321",
    "54211321",
    "54211321",
    "53322321",
    "53222221",
    "11111111",
)

STONE = T(
    "45545541",
    "53344321",
    "43233241",
    "54322321",
    "43342231",
    "52323321",
    "43222321",
    "11111111",
)

SYM_BOMB = T(
    "........",
    ".....8..",
    "...66...",
    "..6776..",
    "..6787..",
    "..6776..",
    "...66...",
    "........",
)

SYM_FIRE = T(
    "........",
    "....9...",
    "...99...",
    "..9899..",
    "..9889..",
    "..9889..",
    "...99...",
    "........",
)

SYM_SPARK = T(
    "........",
    "....aa..",
    "...aa...",
    "..afffa.",
    "....aa..",
    "...aa...",
    "..a.....",
    "........",
)

SYM_MAGNET = T(
    "........",
    "..fb.cf.",
    "..b...c.",
    "..b...c.",
    "..bb.cc.",
    "...b6c..",
    "........",
    "........",
)

SYM_RAINBOW = T(
    "........",
    "....f...",
    "...ddd..",
    "..ddfdd.",
    "...ddd..",
    "....d...",
    "........",
    "........",
)

# Disabled specials: same silhouettes in shadow tones.
def dim(sym):
    return [[(2 if v in (7, 8, 9, 10, 11, 12, 13, 15) else (1 if v == 6 else v)) for v in r] for r in sym]


ICE_OVER = T(
    "ffeeeee.",
    "fe....e.",
    "e..e..e.",
    "e...e.e.",
    "e....ee.",
    "e.e...e.",
    "eeeeeee.",
    "........",
)

ARMOR2 = T(
    "55555551",
    "5f4444f1",
    "54333321",
    "54322321",
    "54322321",
    "53333321",
    "5f2222f1",
    "11111111",
)

ARMOR1 = T(
    "55555551",
    "5f444431",
    "54313321",
    "54331321",
    "54333121",
    "53333311",
    "5322221f",
    "11111111",
)

GHOST = T(
    "54444445",
    "4......4",
    "4......4",
    "4......4",
    "4......4",
    "4......4",
    "4......4",
    "54444445",
)

CHARGED_RING = T(
    "aa.aa.aa",
    "a......a",
    "........",
    "a......a",
    "a......a",
    "........",
    "a......a",
    "aa.aa.aa",
)

FROST_MARK = T(
    "ee....ee",
    "e......e",
    "........",
    "........",
    "........",
    "........",
    "e......e",
    "ee....ee",
)

ARMOR_MARK = T(
    "99....99",
    "9......9",
    "........",
    "........",
    "........",
    "........",
    "9......9",
    "99....99",
)

FLASH1 = T(
    "ffffffff",
    "ffffffff",
    "ff5555ff",
    "ff5555ff",
    "ff5555ff",
    "ff5555ff",
    "ffffffff",
    "ffffffff",
)

FLASH2 = T(
    "........",
    ".5....5.",
    "..5ff5..",
    "..ffff..",
    "..ffff..",
    "..5ff5..",
    ".5....5.",
    "........",
)

FLASH3 = T(
    "f......f",
    "........",
    "...ff...",
    "..f..f..",
    "..f..f..",
    "...ff...",
    "........",
    "f......f",
)

# Bank 0 tiles.
WELL = T(
    "7.......",
    "........",
    "........",
    "........",
    "........",
    "........",
    "........",
    "........",
)
WELL = [[6 if v == 0 else v for v in r] for r in WELL]

WELL_DANGER = [[(12 if (x + y) % 4 == 0 and y == 7 else v) for x, v in enumerate(r)] for y, r in enumerate(WELL)]

FRAME_L = T(
    "12345421",
    "12345421",
    "12345a21",
    "12345421",
    "12345421",
    "12345421",
    "1234b421",
    "12345421",
)

FRAME_R = [list(reversed(r)) for r in FRAME_L]

FRAME_B = T(
    "11111111",
    "22222222",
    "44444444",
    "55555555",
    "4a4b4a4b",
    "33333333",
    "22222222",
    "11111111",
)

FRAME_BL = T(
    "12345421",
    "12345422",
    "12345444",
    "12345555",
    "12345a4b",
    "12333333",
    "12222222",
    "11111111",
)

FRAME_BR = [list(reversed(r)) for r in FRAME_BL]

FRAME_T = T(
    "11111111",
    "22222222",
    "33333333",
    "4b4a4b4a",
    "55555555",
    "44444444",
    "22222222",
    "11111111",
)

FRAME_TL = T(
    "11111111",
    "12222222",
    "12333333",
    "1234b4a4",
    "12345555",
    "12345444",
    "12345422",
    "12345421",
)

FRAME_TR = [list(reversed(r)) for r in FRAME_TL]

CHUTE = T(
    "8.8.8.8.",
    "........",
    "........",
    "........",
    "........",
    "........",
    "........",
    "........",
)

PANEL = [[8] * 8 for _ in range(8)]
PANEL_TL = T(
    "99999999",
    "9fffffff",
    "9f888888",
    "9f888888",
    "9f888888",
    "9f888888",
    "9f888888",
    "9f888888",
)
PANEL_T = T(
    "99999999",
    "ffffffff",
    "88888888",
    "88888888",
    "88888888",
    "88888888",
    "88888888",
    "88888888",
)
PANEL_TR = T(
    "99999999",
    "ffffff19",
    "88888819",
    "88888819",
    "88888819",
    "88888819",
    "88888819",
    "88888819",
)
PANEL_L = T(
    "9f888888",
    "9f888888",
    "9f888888",
    "9f888888",
    "9f888888",
    "9f888888",
    "9f888888",
    "9f888888",
)
PANEL_R = T(
    "88888819",
    "88888819",
    "88888819",
    "88888819",
    "88888819",
    "88888819",
    "88888819",
    "88888819",
)
PANEL_BL = T(
    "9f888888",
    "9f888888",
    "9f888888",
    "9f888888",
    "9f888888",
    "9f888888",
    "91111111",
    "99999999",
)
PANEL_B = T(
    "88888888",
    "88888888",
    "88888888",
    "88888888",
    "88888888",
    "88888888",
    "11111111",
    "99999999",
)
PANEL_BR = T(
    "88888819",
    "88888819",
    "88888819",
    "88888819",
    "88888819",
    "88888819",
    "11111119",
    "99999999",
)


def meter_fill(level):
    """Bank 11 heat meter cell, 8 wide: frame columns + fill from the bottom up to 'level' pixels."""
    rows = []
    for y in range(8):
        fill = (7 - y) < level
        mid = [5 if fill else 2] * 4
        if fill:
            mid = [6, 5, 5, 4]
        rows.append([1, 3] + mid + [3, 1])
    return rows


def meter_cap(top):
    rows = []
    for y in range(8):
        if (top and y == 0) or (not top and y == 7):
            rows.append([1] * 8)
        elif (top and y == 1) or (not top and y == 6):
            rows.append([1, 3, 3, 3, 3, 3, 3, 1])
        else:
            rows.append([1, 3, 2, 2, 2, 2, 3, 1])
    return rows


def build():
    """Returns (tiles list, names list) - index 0 is the transparent tile."""
    tiles = []
    names = []

    def add(name, tile):
        names.append(name)
        tiles.append(tile)

    add("EMPTY", [[0] * 8 for _ in range(8)])
    add("WELL", WELL)
    add("WELL_DANGER", WELL_DANGER)
    add("BLOCK", BLOCK)
    add("BOMB", overlay(BLOCK, SYM_BOMB))
    add("FIRE", overlay(BLOCK, SYM_FIRE))
    add("SPARK", overlay(BLOCK, SYM_SPARK))
    add("MAGNET", overlay(BLOCK, SYM_MAGNET))
    add("RAINBOW", overlay(BLOCK, SYM_RAINBOW))
    iced = overlay(BLOCK, ICE_OVER)
    add("ICE", iced)
    add("ICE_BOMB", overlay(iced, SYM_BOMB))
    add("ICE_FIRE", overlay(iced, SYM_FIRE))
    add("ICE_SPARK", overlay(iced, SYM_SPARK))
    add("ICE_MAGNET", overlay(iced, SYM_MAGNET))
    add("ICE_RAINBOW", overlay(iced, SYM_RAINBOW))
    add("ARMOR2", ARMOR2)
    add("ARMOR1", ARMOR1)
    add("GHOST", GHOST)
    add("CHARGED", overlay(BLOCK, CHARGED_RING))
    add("OFF_BOMB", overlay(BLOCK, dim(SYM_BOMB)))
    add("OFF_FIRE", overlay(BLOCK, dim(SYM_FIRE)))
    add("OFF_SPARK", overlay(BLOCK, dim(SYM_SPARK)))
    add("OFF_MAGNET", overlay(BLOCK, dim(SYM_MAGNET)))
    add("OFF_RAINBOW", overlay(BLOCK, dim(SYM_RAINBOW)))
    add("FROST_MARK", overlay(BLOCK, FROST_MARK))
    add("ARMOR_MARK", overlay(BLOCK, ARMOR_MARK))
    add("STONE", STONE)
    add("GARBAGE", GARBAGE)
    add("FLASH1", FLASH1)
    add("FLASH2", FLASH2)
    add("FLASH3", FLASH3)
    add("FRAME_L", FRAME_L)
    add("FRAME_R", FRAME_R)
    add("FRAME_B", FRAME_B)
    add("FRAME_BL", FRAME_BL)
    add("FRAME_BR", FRAME_BR)
    add("FRAME_T", FRAME_T)
    add("FRAME_TL", FRAME_TL)
    add("FRAME_TR", FRAME_TR)
    add("CHUTE", [[6 if v == 0 else v for v in r] for r in CHUTE])
    add("PANEL", PANEL)
    add("PANEL_TL", PANEL_TL)
    add("PANEL_T", PANEL_T)
    add("PANEL_TR", PANEL_TR)
    add("PANEL_L", PANEL_L)
    add("PANEL_R", PANEL_R)
    add("PANEL_BL", PANEL_BL)
    add("PANEL_B", PANEL_B)
    add("PANEL_BR", PANEL_BR)
    add("METER_TOP", meter_cap(True))
    add("METER_BOTTOM", meter_cap(False))
    for level in range(9):
        add("METER_%d" % level, meter_fill(level))
    # Charged variant on a garbage-looking cell and a ghost for telegraphed incoming garbage.
    add("GARBAGE_GHOST", [[(4 if v in (3, 4, 5) else (0 if v == 2 else v)) for v in r] for r in GARBAGE])
    # Telegraph tiles (bank 0): incoming-garbage warning inside the frame column, column target in the chute,
    # gap marker under the board.
    add("WARN_ROW", T(
        "1cccccc1",
        "1cddddc1",
        "1cdeedc1",
        "1cdeedc1",
        "1cddddc1",
        "1cccccc1",
        "1cccccc1",
        "11111111",
    ))
    add("TARGET", T(
        "........",
        ".cccccc.",
        ".cddddc.",
        "..cddc..",
        "..ceec..",
        "...cc...",
        "...ee...",
        "........",
    ))
    add("GAP_MARK", T(
        "11111111",
        "222ee222",
        "44eeee44",
        "5eeffee5",
        "4a4ee4b4",
        "33333333",
        "22222222",
        "11111111",
    ))
    # Boss HP bar (bank 11: 12 frame, 13 fill light, 14 fill, 1 outline, 2 empty).
    add("HP_LEFT", T(
        "11111111",
        "1ccccccc",
        "1c222222",
        "1c222222",
        "1c222222",
        "1ccccccc",
        "11111111",
        "........",
    ))
    add("HP_RIGHT", T(
        "11111111",
        "ccccccc1",
        "222222c1",
        "222222c1",
        "222222c1",
        "ccccccc1",
        "11111111",
        "........",
    ))
    for level in range(9):
        rows = []
        for y in range(8):
            if y in (0, 6):
                rows.append([1] * 8)
            elif y in (1, 5):
                rows.append([12] * 8)
            elif y == 7:
                rows.append([0] * 8)
            else:
                rows.append([(13 if y == 2 else 14) if x < level else 2 for x in range(8)])
        add("HP_%d" % level, rows)
    # Horizontal heat bar (bank 11, same animated fill colours as the vertical meter).
    add("HEATH_LEFT", [[1] * 8 if y in (0, 7) else ([1] + [3] * 7 if y in (1, 6) else [1, 3] + [2] * 6)
                       for y in range(8)])
    add("HEATH_RIGHT", [[1] * 8 if y in (0, 7) else ([3] * 7 + [1] if y in (1, 6) else [2] * 6 + [3, 1])
                        for y in range(8)])
    for level in range(9):
        rows = []
        for y in range(8):
            if y in (0, 7):
                rows.append([1] * 8)
            elif y in (1, 6):
                rows.append([3] * 8)
            else:
                fill = {2: 4, 3: 5, 4: 5, 5: 6}[y]
                rows.append([fill if x < level else 2 for x in range(8)])
        add("HEATH_%d" % level, rows)
    return tiles, names


# ---------------------------------------------------------------- palettes

def shade_bank(base, outline_tint=None):
    """Builds the 16 colours of a block bank from a base colour."""
    r, g, b = base

    def mix(c, t, k):
        return tuple(int(c[i] + (t[i] - c[i]) * k) for i in range(3))
    black = (8, 8, 24) if outline_tint is None else outline_tint
    shadow = mix(base, (16, 16, 40), 0.45)
    light = mix(base, (255, 255, 255), 0.35)
    high = mix(base, (255, 255, 255), 0.7)
    outline = mix(base, black, 0.8)
    return [
        (0, 0, 0), outline, shadow, base, light, high,
        (20, 16, 36),       # 6 symbol dark
        (232, 48, 32),      # 7 hot core (animated)
        (255, 240, 120),    # 8 hot bright (animated)
        (255, 136, 24),     # 9 flame orange
        (96, 216, 255),     # 10 electric (animated)
        (232, 40, 56),      # 11 magnet red
        (48, 96, 240),      # 12 magnet blue
        (200, 96, 255),     # 13 rainbow (animated)
        (176, 232, 255),    # 14 ice pale
        (255, 255, 255),    # 15 white
    ]


SHAPE_COLORS = [
    (40, 200, 232),     # I cyan
    (248, 208, 40),     # O yellow
    (176, 72, 232),     # T violet
    (72, 216, 80),      # S green
    (240, 64, 72),      # Z red
    (56, 104, 240),     # J blue
    (248, 144, 32),     # L orange
]


def frame_bank(metal=(112, 120, 144), accent=(248, 200, 40), well=(14, 16, 34), panel=(22, 26, 52)):
    def mix(c, t, k):
        return tuple(int(c[i] + (t[i] - c[i]) * k) for i in range(3))
    return [
        (0, 0, 0),
        (6, 6, 16),                         # 1 outline
        mix(metal, (0, 0, 0), 0.55),        # 2 frame dark
        mix(metal, (0, 0, 0), 0.25),        # 3 frame mid
        metal,                              # 4 frame light
        mix(metal, (255, 255, 255), 0.45),  # 5 frame highlight
        well,                               # 6 well background
        mix(well, (255, 255, 255), 0.12),   # 7 grid dot
        panel,                              # 8 panel bg
        mix(panel, (255, 255, 255), 0.35),  # 9 panel border
        accent,                             # 10 accent (bolts)
        mix(accent, (255, 64, 0), 0.5),     # 11 accent 2
        (232, 40, 32),                      # 12 danger red
        (255, 136, 24),                     # 13 orange
        (255, 224, 72),                     # 14 yellow
        (255, 255, 255),                    # 15 white
    ]


def heat_bank():
    return [
        (0, 0, 0),
        (6, 6, 16),         # 1 outline
        (30, 20, 40),       # 2 empty meter
        (110, 110, 130),    # 3 meter frame
        (255, 240, 160),    # 4 fill highlight
        (255, 120, 24),     # 5 fill (animated by heat level)
        (200, 40, 24),      # 6 fill dark
        (255, 255, 255),
        (40, 40, 60), (60, 60, 90), (90, 90, 120), (140, 140, 170),
        (200, 200, 220), (255, 200, 80), (255, 80, 40), (255, 255, 255),
    ]


def palette():
    banks = [frame_bank()]
    for c in SHAPE_COLORS:
        banks.append(shade_bank(c))
    banks.append(shade_bank((128, 132, 152)))      # 8 garbage
    banks.append(shade_bank((104, 88, 80)))        # 9 stone
    steel = shade_bank((150, 160, 184))
    banks.append(steel)                            # 10 armour
    banks.append(heat_bank())                      # 11 heat / HUD
    colors = []
    for b in banks:
        colors.extend(b)
    return colors
