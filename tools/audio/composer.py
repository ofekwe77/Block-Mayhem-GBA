"""BLOCK MAYHEM chiptune composer.

A song = tempo + instrument set + sections. Each section has a chord
progression (one chord per bar or half bar), a hand-written lead melody, and
style choices for bass / arpeggio / drums. The arranger renders each section
into an XM pattern (64 rows = 4 bars of 16ths) on 8 channels:

  0 lead   1 echo / harmony   2 arpeggio / pad   3 bass
  4 kick   5 snare / clap     6 hats             7 accents (crash, toms, counter-line)

Melody notation (space separated tokens, 1 unit = one 16th note row):
  E5:4   note E in octave 5 lasting 4 rows
  r:2    rest for 2 rows
  ~:2    hold the previous note 2 more rows (tie)
"""
import math
import random
import zlib

from xm import Instrument, Module, Pattern, KEY_OFF

NOTE_INDEX = {"C": 0, "C#": 1, "DB": 1, "D": 2, "D#": 3, "EB": 3, "E": 4, "F": 5, "F#": 6, "GB": 6, "G": 7,
              "G#": 8, "AB": 8, "A": 9, "A#": 10, "BB": 10, "B": 11}

CHORDS = {
    "": (0, 4, 7), "m": (0, 3, 7), "7": (0, 4, 7, 10), "m7": (0, 3, 7, 10), "maj7": (0, 4, 7, 11),
    "sus2": (0, 2, 7), "sus4": (0, 5, 7), "dim": (0, 3, 6), "aug": (0, 4, 8), "5": (0, 7, 12),
    "add9": (0, 4, 7, 14), "m9": (0, 3, 7, 14), "6": (0, 4, 7, 9), "m6": (0, 3, 7, 9),
}


def xm_note(name):
    """'E5' -> XM note number (C-0 = 1)."""
    name = name.upper()
    pitch, octave = (name[:-1], int(name[-1]))
    return octave * 12 + NOTE_INDEX[pitch] + 1


def chord(sym):
    """'Em7' -> (root semitone 0..11, intervals)."""
    sym = sym.strip()
    if len(sym) > 1 and sym[1] in "#B" and sym[:2].upper() in NOTE_INDEX:
        root, kind = sym[:2].upper(), sym[2:]
    else:
        root, kind = sym[0].upper(), sym[1:]
    return NOTE_INDEX[root], CHORDS[kind]


def parse_melody(text):
    events = []     # (row, note or None (rest) or 'hold', length)
    row = 0
    for tok in text.split():
        name, _, length = tok.partition(":")
        length = int(length or 1)
        if name == "r":
            events.append((row, None, length))
        elif name == "~":
            if events:
                r0, n0, l0 = events[-1]
                events[-1] = (r0, n0, l0 + length)
        else:
            events.append((row, xm_note(name), length))
        row += length
    return events, row


# ---------------------------------------------------------------- instruments

def wave(kind, length=32, duty=0.5):
    out = []
    for i in range(length):
        p = i / length
        if kind == "square":
            v = 1.0 if p < duty else -1.0
        elif kind == "tri":
            v = 4 * p - 1 if p < 0.5 else 3 - 4 * p
        elif kind == "saw":
            v = 2 * p - 1
        elif kind == "sine":
            v = math.sin(2 * math.pi * p)
        elif kind == "organ":
            v = 0.6 * (1.0 if p < 0.5 else -1.0) + 0.4 * (1.0 if (p * 2) % 1 < 0.5 else -1.0)
        elif kind == "bell":
            v = 0.7 * math.sin(2 * math.pi * p) + 0.3 * math.sin(2 * math.pi * p * 3)
        elif kind == "growl":
            v = 0.6 * (2 * p - 1) + 0.4 * (1.0 if p < 0.3 else -1.0)
        else:
            v = 0.0
        out.append(int(max(-127, min(127, v * 100))))
    return out


def tonal(name, kind, duty=0.5, env=None, volume=64, octave_shift=0, length=32):
    data = wave(kind, length, duty)
    # 32-sample cycle at 8363 Hz ~ C-4; 64 samples -> one octave lower (compensate with +12).
    rel = (12 if length == 64 else 0) + octave_shift * 12
    return Instrument(name, data, 0, len(data), volume, rel, 0, 128, env, 512)


def drum(name, kind, volume=64):
    rate = 16726
    rnd = random.Random(zlib.crc32(name.encode()) & 0xFFFF)   # stable across runs (str hash is salted)
    out = []
    if kind == "kick":
        n = int(0.16 * rate)
        ph = 0.0
        for i in range(n):
            t = i / rate
            f = 40 + 140 * math.exp(-t * 32)
            ph += f / rate
            env = math.exp(-t * 18)
            out.append(math.sin(2 * math.pi * ph) * env)
    elif kind == "snare":
        n = int(0.18 * rate)
        ph = 0.0
        for i in range(n):
            t = i / rate
            ph += (190 - 60 * t) / rate
            env = math.exp(-t * 22)
            out.append((0.65 * rnd.uniform(-1, 1) + 0.45 * math.sin(2 * math.pi * ph)) * env)
    elif kind == "clap":
        n = int(0.2 * rate)
        for i in range(n):
            t = i / rate
            burst = 1.0 if (t < 0.01 or 0.018 < t < 0.026 or t > 0.034) else 0.3
            out.append(rnd.uniform(-1, 1) * math.exp(-t * 20) * burst)
    elif kind == "hat":
        n = int(0.045 * rate)
        prev = 0.0
        for i in range(n):
            t = i / rate
            v = rnd.uniform(-1, 1)
            hp = v - prev
            prev = v
            out.append(hp * 0.7 * math.exp(-t * 70))
    elif kind == "openhat":
        n = int(0.22 * rate)
        prev = 0.0
        for i in range(n):
            t = i / rate
            v = rnd.uniform(-1, 1)
            hp = v - prev
            prev = v
            out.append(hp * 0.6 * math.exp(-t * 14))
    elif kind == "crash":
        n = int(0.7 * rate)
        prev = 0.0
        for i in range(n):
            t = i / rate
            v = rnd.uniform(-1, 1)
            hp = v - prev * 0.5
            prev = v
            out.append(hp * 0.55 * math.exp(-t * 5))
    elif kind == "tom":
        n = int(0.22 * rate)
        ph = 0.0
        for i in range(n):
            t = i / rate
            ph += (110 + 120 * math.exp(-t * 20)) / rate
            out.append(math.sin(2 * math.pi * ph) * math.exp(-t * 14))
    elif kind == "zap":
        n = int(0.12 * rate)
        ph = 0.0
        for i in range(n):
            t = i / rate
            ph += (2000 * math.exp(-t * 30) + 200) / rate
            out.append((1.0 if (ph % 1) < 0.5 else -1.0) * 0.5 * math.exp(-t * 25))
    data = [int(max(-127, min(127, v * 120))) for v in out]
    return Instrument(name, data, 0, 0, volume, 12, 0, 128, None, 0)


ENV_PLUCK = [(0, 64), (3, 50), (10, 36), (40, 26), (120, 20)]
ENV_LEAD = [(0, 60), (2, 64), (8, 52), (30, 44), (200, 40)]
ENV_SOFT = [(0, 10), (6, 40), (14, 48), (60, 40), (200, 34)]
ENV_BASS = [(0, 64), (4, 54), (20, 44), (200, 40)]
ENV_STAB = [(0, 64), (2, 40), (6, 18), (12, 8), (20, 0)]
ENV_BELL = [(0, 64), (4, 40), (16, 22), (48, 10), (90, 0)]


def default_instruments(style):
    """Instrument set shared by every song (1-based instrument numbers in the returned order)."""
    lead_kind = {"bright": ("square", 0.25), "cold": ("bell", 0.5), "aggressive": ("growl", 0.5),
                 "funk": ("square", 0.125), "cosmic": ("sine", 0.5), "epic": ("saw", 0.5)}.get(style, ("square", 0.25))
    return [
        tonal("lead", lead_kind[0], lead_kind[1], ENV_LEAD),            # 1
        tonal("echo", "square", 0.5, ENV_PLUCK, 40),                    # 2
        tonal("arp", "square", 0.125, ENV_STAB),                        # 3
        tonal("bass", "tri", 0.5, ENV_BASS, 64, -1, 64),                # 4
        drum("kick", "kick"),                                           # 5
        drum("snare", "snare"),                                         # 6
        drum("hat", "hat", 40),                                         # 7
        drum("openhat", "openhat", 36),                                 # 8
        drum("crash", "crash", 44),                                     # 9
        drum("tom", "tom"),                                             # 10
        tonal("pad", "organ", 0.5, ENV_SOFT, 34),                       # 11
        tonal("bell", "bell", 0.5, ENV_BELL, 50),                       # 12
        drum("clap", "clap"),                                           # 13
        drum("zap", "zap", 40),                                         # 14
        tonal("sqbass", "square", 0.5, ENV_BASS, 48, -1, 64),           # 15
        tonal("sawlead", "saw", 0.5, ENV_LEAD, 52),                     # 16
    ]


I_LEAD, I_ECHO, I_ARP, I_BASS, I_KICK, I_SNARE, I_HAT, I_OHAT, I_CRASH, I_TOM, I_PAD, I_BELL, I_CLAP, I_ZAP, \
    I_SQBASS, I_SAW = range(1, 17)


# ---------------------------------------------------------------- drum styles (16-row bar patterns)

DRUMS = {
    "rock": {"k": "x...x...x.x.....", "s": "....x.......x...", "h": "x.x.x.x.x.x.x.x."},
    "drive": {"k": "x..x..x.x..x..x.", "s": "....x.......x...", "h": "xxxxxxxxxxxxxxxx"},
    "four": {"k": "x...x...x...x...", "s": "....x.......x...", "h": "..x...x...x...x."},
    "half": {"k": "x.........x.....", "s": "........x.......", "h": "x...x...x...x..."},
    "funk": {"k": "x..x......x..x..", "s": "....x..x....x...", "h": "x.xxx.xxx.xxx.x."},
    "break": {"k": "x.....x...x.....", "s": "....x.....x.x...", "h": "x.x.x.x.x.x.x.x."},
    "march": {"k": "x...x...x...x...", "s": "..x...x...x.x.x.", "h": "x.x.x.x.x.x.x.x."},
    "sparse": {"k": "x...............", "s": "........x.......", "h": "....x.......x..."},
    "seven": {"k": "x.....x...x.....", "s": "...x......x...x.", "h": "x.x.xx.x.x.xx.x."},
    "blast": {"k": "x.x.x.x.x.x.x.x.", "s": "....x.......x..x", "h": "xxxxxxxxxxxxxxxx"},
    "none": {"k": "................", "s": "................", "h": "................"},
}


def render_section(pat, sec, song, variation):
    """Renders one 64-row section into pattern 'pat'."""
    rows = pat.rows
    bars = rows // 16
    chords = sec["chords"]
    per_bar = len(chords) // bars if len(chords) >= bars else 1
    chord_rows = rows // len(chords)
    lead_vol = sec.get("lead_vol", 0x30)

    # ---- lead + echo / harmony
    if sec.get("melody"):
        events, _ = parse_melody(sec["melody"])
        octave_up = 12 if variation.get("lead_up") else 0
        for (row, note, length) in events:
            if row >= rows:
                break
            if note is None:
                pat.set(row, 0, KEY_OFF)
                continue
            n = note + octave_up
            pat.set(row, 0, n, sec.get("lead_instr", I_LEAD), 0x10 + lead_vol)
            if variation.get("echo"):
                if row + 3 < rows:
                    pat.set(row + 3, 1, n, I_ECHO, 0x10 + lead_vol // 2)
            elif variation.get("harmony"):
                pat.set(row, 1, n - sec.get("harmony_interval", 4), I_ECHO, 0x10 + lead_vol * 2 // 3)
            if length >= 3 and row + length < rows and sec.get("staccato"):
                pat.set(row + length - 1, 0, KEY_OFF)

    # ---- arpeggio / pad
    arp = sec.get("arp", "up")
    for ci, sym in enumerate(chords):
        root, iv = chord(sym)
        base_oct = sec.get("arp_octave", 5)
        start = ci * chord_rows
        tones = [xm_note("C%d" % base_oct) + root + i for i in iv]
        if arp == "pad" or variation.get("pad"):
            pat.set(start, 2, tones[0], I_PAD, 0x10 + 0x22)
            if len(tones) > 1:
                pat.set(start, 7, tones[1] + 12, I_PAD, 0x10 + 0x1c) if sec.get("pad_two") else None
        if arp in ("up", "updown", "rand", "pulse"):
            seq = tones + ([t + 12 for t in tones[:1]])
            if arp == "updown":
                seq = seq + seq[-2:0:-1]
            step = sec.get("arp_step", 1)
            for r in range(0, chord_rows, step):
                if arp == "pulse" and r % 4 != 0:
                    continue
                k = (r // step) % len(seq)
                if arp == "rand":
                    k = (r * 7 + ci * 3) % len(seq)
                pat.set(start + r, 2, seq[k], I_ARP, 0x10 + sec.get("arp_vol", 0x1c))
        if arp == "bell":
            for r in range(0, chord_rows, 4):
                k = (r // 4) % len(tones)
                pat.set(start + r, 2, tones[k] + 12, I_BELL, 0x10 + 0x24)

    # ---- bass
    bass = sec.get("bass", "pump")
    for ci, sym in enumerate(chords):
        root, iv = chord(sym)
        start = ci * chord_rows
        r0 = xm_note("C%d" % sec.get("bass_octave", 3)) + root
        fifth = r0 + 7
        instr = sec.get("bass_instr", I_BASS)
        vol = 0x10 + sec.get("bass_vol", 0x30)
        if bass == "pump":
            for r in range(0, chord_rows, 2):
                pat.set(start + r, 3, r0 + (12 if (r // 2) % 2 else 0), instr, vol)
        elif bass == "root":
            pat.set(start, 3, r0, instr, vol)
            if chord_rows >= 16:
                pat.set(start + 8, 3, r0, instr, vol)
        elif bass == "walk":
            seq = [r0, r0 + iv[1], fifth, r0 + iv[-1] if iv[-1] < 12 else r0 + 9]
            for r in range(0, chord_rows, 4):
                pat.set(start + r, 3, seq[(r // 4) % 4], instr, vol)
        elif bass == "funk":
            pattern = [0, None, None, 12, None, None, 0, None, 10, None, 0, None, None, 7, None, None]
            for r in range(chord_rows):
                v = pattern[r % 16]
                if v is not None:
                    pat.set(start + r, 3, r0 + v, instr, vol)
                elif r % 16 in (2, 5):
                    pat.set(start + r, 3, KEY_OFF)
        elif bass == "drone":
            if ci % 2 == 0:
                pat.set(start, 3, r0, instr, vol)
        elif bass == "gallop":
            for r in range(0, chord_rows, 4):
                pat.set(start + r, 3, r0, instr, vol)
                pat.set(start + r + 2, 3, r0, instr, vol - 8)
                pat.set(start + r + 3, 3, r0 + 12, instr, vol - 8)
        elif bass == "seven":
            for r in range(0, chord_rows):
                if r % 7 in (0, 3) or r % 7 == 5:
                    pat.set(start + r, 3, r0 + (7 if r % 7 == 5 else 0), instr, vol)

    # ---- drums
    style = variation.get("drums", sec.get("drums", "rock"))
    d = DRUMS[style]
    fill = variation.get("fill", True)
    for bar in range(bars):
        for r in range(16):
            row = bar * 16 + r
            if d["k"][r] == "x":
                pat.set(row, 4, xm_note("C4"), I_KICK, 0x10 + 0x38)
            if d["s"][r] == "x":
                pat.set(row, 5, xm_note("C4"), sec.get("snare_instr", I_SNARE), 0x10 + 0x30)
            if d["h"][r] == "x":
                accent = 0x20 if r % 4 == 0 else 0x16
                pat.set(row, 6, xm_note("C4"), I_HAT, 0x10 + accent)
        if fill and bar == bars - 1 and style not in ("none", "sparse"):
            for k, r in enumerate((12, 13, 14, 15)):
                pat.set(bar * 16 + r, 7, xm_note("C4") + (4 - k) * 2, I_TOM, 0x10 + 0x2c)
    if variation.get("crash", False):
        pat.set(0, 7, xm_note("C4"), I_CRASH, 0x10 + 0x30)
    if sec.get("counter"):
        events, _ = parse_melody(sec["counter"])
        for (row, note, length) in events:
            if row < rows and note is not None and not (fill and row >= rows - 4):
                pat.set(row, 7, note, sec.get("counter_instr", I_BELL), 0x10 + 0x22)


def build_song(song, path, hot=False):
    """song: dict(name, bpm, speed, style, sections{name: sec}, arrangement[(section, variation)])."""
    bpm = song["bpm"]
    if hot:
        bpm = int(bpm * 1.08)
    mod = Module(song["name"], 8, song.get("speed", 6), bpm)
    mod.instruments = default_instruments(song.get("style", "bright"))
    for idx, (sec_name, var) in enumerate(song["arrangement"]):
        sec = song["sections"][sec_name]
        v = dict(var)
        if hot:
            v["drums"] = song.get("hot_drums", "drive")
            v["echo"] = True
            v.setdefault("crash", idx % 2 == 0)
        pat = Pattern(sec.get("rows", 64), 8)
        render_section(pat, sec, song, v)
        mod.patterns.append(pat)
        mod.order.append(idx)
    mod.restart = song.get("restart", 0)
    mod.write(path)
    return mod
