"""Original sound-effect designs (PART 25.5). Each function returns a Sound."""
from synth import Sound, tone, osc, mul, env_adsr, lowpass, arpeggio, note_hz


def s(seconds):
    return Sound(seconds)


def move():
    x = s(0.04)
    x.add(tone("square", 880, 820, 0.04, d=0.02, s=0.2, r=0.01, duty=0.25), gain=0.5)
    return x.normalize(0.45)


def rotate():
    x = s(0.06)
    x.add(tone("square", 660, 990, 0.06, d=0.03, s=0.3, r=0.02, duty=0.125), gain=0.6)
    return x.normalize(0.5)


def hold():
    x = s(0.12)
    x.add(arpeggio("tri", [note_hz("E5"), note_hz("B5"), note_hz("E6")], 0.03, 0.12, s=0.4), gain=0.7)
    return x.normalize(0.55)


def hard_drop():
    x = s(0.18)
    x.add(tone("noise", 3000, 300, 0.18, d=0.12, s=0.1, r=0.05, seed=3), gain=0.6)
    x.add(tone("sine", 160, 50, 0.16, d=0.1, s=0.2, r=0.05), gain=0.9)
    return x.normalize(0.85)


def lock():
    x = s(0.07)
    x.add(tone("square", 220, 180, 0.07, d=0.04, s=0.2, r=0.02, duty=0.5), gain=0.5)
    x.add(tone("noise", 6000, 2000, 0.05, d=0.03, s=0.1, r=0.02, seed=4), gain=0.3)
    return x.normalize(0.55)


def clear(n):
    notes = [["C5", "E5", "G5"], ["C5", "E5", "G5", "C6"], ["D5", "F#5", "A5", "D6", "F#6"],
             ["C5", "E5", "G5", "C6", "E6", "G6", "C7"]][n - 1]
    dur = 0.18 + 0.08 * n
    x = s(dur + 0.1)
    x.add(arpeggio("square", [note_hz(v) for v in notes], 0.035, dur, duty=0.25, s=0.55), gain=0.6)
    x.add(tone("noise", 9000, 4000, dur * 0.6, d=0.1, s=0.2, r=0.1, seed=7), gain=0.15 * n)
    if n == 4:
        x.add(tone("saw", 110, 220, dur, d=0.2, s=0.6, r=0.1), gain=0.4)
    return x.normalize(0.85)


def bomb():
    x = s(0.55)
    x.add(lowpass(tone("noise", 4000, 400, 0.55, d=0.35, s=0.1, r=0.15, seed=11), 0.35), gain=1.0)
    x.add(tone("sine", 90, 30, 0.45, d=0.3, s=0.2, r=0.1), gain=1.0)
    return x.normalize(0.95)


def fire():
    x = s(0.35)
    x.add(lowpass(tone("noise", 2500, 1200, 0.35, a=0.02, d=0.2, s=0.3, r=0.1, seed=12), 0.25), gain=0.9)
    x.add(tone("saw", 140, 200, 0.3, d=0.2, s=0.2, r=0.1), gain=0.3)
    return x.normalize(0.75)


def spark():
    x = s(0.25)
    x.add(tone("square", 1800, 600, 0.25, d=0.15, s=0.2, r=0.08, duty=0.125, vib=0.08), gain=0.6)
    x.add(tone("noise", 12000, 8000, 0.2, d=0.12, s=0.15, r=0.05, seed=13), gain=0.4)
    return x.normalize(0.7)


def ice():
    x = s(0.28)
    x.add(arpeggio("tri", [note_hz(n) for n in ("E7", "B6", "G7", "D7")], 0.025, 0.2, s=0.3), gain=0.6)
    x.add(tone("noise", 14000, 9000, 0.15, d=0.08, s=0.1, r=0.05, seed=14), gain=0.4)
    return x.normalize(0.7)


def armor():
    x = s(0.2)
    x.add(tone("square", 520, 500, 0.2, d=0.1, s=0.3, r=0.08, duty=0.5), gain=0.4)
    x.add(tone("square", 787, 760, 0.2, d=0.1, s=0.3, r=0.08, duty=0.25), gain=0.4)
    x.add(tone("noise", 8000, 6000, 0.05, d=0.03, s=0.1, r=0.01, seed=15), gain=0.4)
    return x.normalize(0.7)


def magnet():
    x = s(0.3)
    x.add(tone("square", 200, 400, 0.3, d=0.1, s=0.5, r=0.08, duty=0.5, vib=0.15), gain=0.5)
    x.add(tone("tri", 400, 800, 0.3, d=0.1, s=0.5, r=0.08), gain=0.4)
    return x.normalize(0.7)


def rainbow():
    x = s(0.4)
    x.add(arpeggio("square", [note_hz(n) for n in ("C6", "E6", "G6", "B6", "D7", "G7")], 0.03, 0.35, duty=0.25,
                   s=0.45), gain=0.6)
    return x.normalize(0.7)


def laser():
    x = s(0.5)
    x.add(tone("saw", 2200, 180, 0.5, a=0.005, d=0.3, s=0.4, r=0.15), gain=0.6)
    x.add(tone("square", 1100, 90, 0.5, a=0.005, d=0.3, s=0.4, r=0.15, duty=0.125), gain=0.4)
    return x.normalize(0.85)


def freeze():
    x = s(0.6)
    x.add(arpeggio("tri", [note_hz(n) for n in ("B6", "F#6", "D7", "A6", "F#7")], 0.06, 0.55, s=0.5, r=0.2),
          gain=0.6)
    x.add(tone("noise", 15000, 12000, 0.5, a=0.05, d=0.2, s=0.2, r=0.2, seed=16), gain=0.2)
    return x.normalize(0.7)


def gravity():
    x = s(0.7)
    x.add(tone("sine", 70, 35, 0.7, a=0.05, d=0.4, s=0.5, r=0.2), gain=1.0)
    x.add(tone("square", 140, 70, 0.6, a=0.05, d=0.3, s=0.3, r=0.2, duty=0.5), gain=0.3)
    return x.normalize(0.9)


def nuke():
    x = s(1.1)
    x.add(tone("tri", 200, 1200, 0.25, a=0.01, d=0.1, s=0.8, r=0.02), gain=0.5)
    boom = lowpass(tone("noise", 5000, 200, 0.85, d=0.6, s=0.2, r=0.2, seed=17), 0.3)
    x.add(boom, start=int(0.22 * 22050), gain=1.0)
    x.add(tone("sine", 80, 25, 0.8, d=0.5, s=0.3, r=0.2), start=int(0.22 * 22050), gain=1.0)
    return x.normalize(0.95)


def shield():
    x = s(0.45)
    x.add(tone("square", 330, 660, 0.4, a=0.02, d=0.2, s=0.6, r=0.15, duty=0.25, vib=0.04), gain=0.5)
    x.add(tone("tri", 660, 1320, 0.4, a=0.02, d=0.2, s=0.6, r=0.15), gain=0.4)
    return x.normalize(0.7)


def vent():
    x = s(0.6)
    x.add(lowpass(tone("noise", 6000, 3000, 0.6, a=0.03, d=0.3, s=0.4, r=0.2, seed=18), 0.5), gain=0.9)
    x.add(tone("sine", 120, 240, 0.5, d=0.3, s=0.3, r=0.2), gain=0.3)
    return x.normalize(0.75)


def shift():
    x = s(0.3)
    x.add(arpeggio("square", [note_hz(n) for n in ("G5", "D6", "G6", "D6")], 0.04, 0.28, duty=0.5, s=0.4), gain=0.6)
    return x.normalize(0.6)


def chain(big):
    notes = ("C6", "D#6", "G6", "A#6", "C7") if big else ("G5", "C6", "E6")
    dur = 0.45 if big else 0.25
    x = s(dur + 0.05)
    x.add(arpeggio("square", [note_hz(n) for n in notes], 0.04, dur, duty=0.125, s=0.6), gain=0.6)
    if big:
        x.add(tone("noise", 10000, 3000, dur, d=0.2, s=0.3, r=0.1, seed=19), gain=0.3)
    return x.normalize(0.8)


def overheat_start():
    x = s(0.9)
    for k in range(4):
        x.add(tone("square", 880, 660, 0.12, d=0.05, s=0.6, r=0.03, duty=0.5), start=int(k * 0.15 * 22050), gain=0.5)
    x.add(lowpass(tone("noise", 4000, 800, 0.4, d=0.3, s=0.2, r=0.1, seed=20), 0.3), start=int(0.55 * 22050),
          gain=0.9)
    return x.normalize(0.85)


def overheat_success():
    x = s(0.8)
    x.add(arpeggio("square", [note_hz(n) for n in ("C5", "G5", "C6", "E6", "G6", "C7")], 0.06, 0.75, duty=0.25,
                   s=0.6, r=0.2), gain=0.6)
    x.add(arpeggio("tri", [note_hz(n) for n in ("C4", "G4", "C5")], 0.12, 0.75, s=0.6, r=0.2), gain=0.5)
    return x.normalize(0.85)


def overheat_fail():
    x = s(0.8)
    x.add(tone("saw", 400, 80, 0.8, d=0.5, s=0.4, r=0.2), gain=0.6)
    x.add(tone("square", 300, 60, 0.8, d=0.5, s=0.4, r=0.2, duty=0.5), gain=0.4)
    return x.normalize(0.85)


def boss_hit():
    x = s(0.25)
    x.add(tone("square", 300, 120, 0.2, d=0.1, s=0.3, r=0.05, duty=0.5), gain=0.6)
    x.add(tone("noise", 5000, 1500, 0.2, d=0.12, s=0.1, r=0.05, seed=21), gain=0.6)
    return x.normalize(0.85)


def boss_attack():
    x = s(0.5)
    x.add(lowpass(tone("noise", 2000, 300, 0.5, d=0.3, s=0.3, r=0.15, seed=22), 0.3), gain=0.9)
    x.add(tone("saw", 110, 55, 0.45, d=0.3, s=0.3, r=0.1), gain=0.6)
    return x.normalize(0.9)


def boss_charge():
    x = s(0.6)
    x.add(tone("square", 150, 600, 0.6, a=0.05, d=0.4, s=0.8, r=0.05, duty=0.25, vib=0.06), gain=0.5)
    return x.normalize(0.7)


def boss_defeat():
    x = s(1.6)
    for k in range(5):
        x.add(lowpass(tone("noise", 4000, 300, 0.4, d=0.3, s=0.2, r=0.1, seed=30 + k), 0.3),
              start=int(k * 0.22 * 22050), gain=0.8)
    x.add(tone("sine", 70, 20, 1.2, d=0.8, s=0.3, r=0.3), start=int(0.6 * 22050), gain=1.0)
    return x.normalize(0.95)


def stage_clear():
    x = s(1.1)
    seq = [("C5", 0.0), ("E5", 0.1), ("G5", 0.2), ("C6", 0.3), ("G5", 0.45), ("C6", 0.55)]
    for n, t in seq:
        x.add(tone("square", note_hz(n), note_hz(n), 0.25, d=0.1, s=0.5, r=0.1, duty=0.25),
              start=int(t * 22050), gain=0.5)
    x.add(tone("tri", note_hz("C4"), note_hz("C4"), 0.9, d=0.3, s=0.6, r=0.3), start=int(0.3 * 22050), gain=0.5)
    return x.normalize(0.85)


def menu_move():
    x = s(0.05)
    x.add(tone("square", 1320, 1320, 0.05, d=0.02, s=0.3, r=0.02, duty=0.25), gain=0.5)
    return x.normalize(0.45)


def menu_ok():
    x = s(0.14)
    x.add(arpeggio("square", [note_hz("A5"), note_hz("E6")], 0.05, 0.14, duty=0.25, s=0.5), gain=0.6)
    return x.normalize(0.6)


def menu_back():
    x = s(0.12)
    x.add(arpeggio("square", [note_hz("E6"), note_hz("A5")], 0.05, 0.12, duty=0.25, s=0.5), gain=0.6)
    return x.normalize(0.55)


def unlock():
    x = s(0.7)
    x.add(arpeggio("square", [note_hz(n) for n in ("G5", "B5", "D6", "G6", "B6", "D7", "G7")], 0.05, 0.65,
                   duty=0.125, s=0.6, r=0.2), gain=0.6)
    return x.normalize(0.8)


def game_over():
    x = s(1.2)
    seq = [("G4", 0.0), ("F#4", 0.25), ("F4", 0.5), ("E4", 0.75)]
    for n, t in seq:
        x.add(tone("square", note_hz(n), note_hz(n) * 0.97, 0.3, d=0.15, s=0.5, r=0.1, duty=0.5),
              start=int(t * 22050), gain=0.5)
    return x.normalize(0.8)


def denied():
    x = s(0.18)
    x.add(tone("square", 140, 120, 0.18, d=0.05, s=0.6, r=0.05, duty=0.5), gain=0.6)
    return x.normalize(0.55)


def garbage():
    x = s(0.3)
    x.add(tone("saw", 90, 60, 0.3, d=0.2, s=0.3, r=0.08), gain=0.7)
    x.add(tone("noise", 2000, 800, 0.25, d=0.15, s=0.2, r=0.08, seed=23), gain=0.5)
    return x.normalize(0.8)


def danger():
    x = s(0.5)
    for k in range(2):
        x.add(tone("square", 990, 990, 0.12, d=0.03, s=0.7, r=0.03, duty=0.5), start=int(k * 0.22 * 22050),
              gain=0.4)
    return x.normalize(0.55)


def level_up():
    x = s(0.45)
    x.add(arpeggio("square", [note_hz(n) for n in ("C6", "G6", "C7")], 0.07, 0.4, duty=0.25, s=0.5), gain=0.6)
    return x.normalize(0.65)


def pause():
    x = s(0.15)
    x.add(tone("tri", 880, 440, 0.15, d=0.08, s=0.4, r=0.05), gain=0.6)
    return x.normalize(0.5)


def switch_board():
    x = s(0.18)
    x.add(tone("square", 500, 1000, 0.18, d=0.08, s=0.4, r=0.05, duty=0.25), gain=0.6)
    return x.normalize(0.55)


ALL = {
    "sfx_move": move, "sfx_rotate": rotate, "sfx_hold": hold, "sfx_hard_drop": hard_drop, "sfx_lock": lock,
    "sfx_clear1": lambda: clear(1), "sfx_clear2": lambda: clear(2), "sfx_clear3": lambda: clear(3),
    "sfx_clear4": lambda: clear(4), "sfx_bomb": bomb, "sfx_fire": fire, "sfx_spark": spark, "sfx_ice": ice,
    "sfx_armor": armor, "sfx_magnet": magnet, "sfx_rainbow": rainbow, "sfx_laser": laser, "sfx_freeze": freeze,
    "sfx_gravity": gravity, "sfx_nuke": nuke, "sfx_shield": shield, "sfx_vent": vent, "sfx_shift": shift,
    "sfx_chain": lambda: chain(False), "sfx_chain_big": lambda: chain(True), "sfx_overheat_start": overheat_start,
    "sfx_overheat_success": overheat_success, "sfx_overheat_fail": overheat_fail, "sfx_boss_hit": boss_hit,
    "sfx_boss_attack": boss_attack, "sfx_boss_charge": boss_charge, "sfx_boss_defeat": boss_defeat,
    "sfx_stage_clear": stage_clear, "sfx_menu_move": menu_move, "sfx_menu_ok": menu_ok, "sfx_menu_back": menu_back,
    "sfx_unlock": unlock, "sfx_game_over": game_over, "sfx_denied": denied, "sfx_garbage": garbage,
    "sfx_danger": danger, "sfx_level_up": level_up, "sfx_pause": pause, "sfx_switch_board": switch_board,
}
