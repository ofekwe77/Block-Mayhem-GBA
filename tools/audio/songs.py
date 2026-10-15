"""The BLOCK MAYHEM soundtrack (original compositions).

The 'SPICE motif' (E-G-B / A-G-A, rising to E6) opens the title theme and
returns transformed in World 6, the final boss theme and the ending.
"""

V = {}
CR = {"crash": True}
EC = {"echo": True}
HA = {"harmony": True}
UP = {"lead_up": True}
PAD = {"pad": True}

TITLE = {
    "name": "Ignition Theme", "bpm": 132, "style": "epic",
    "sections": {
        "intro": {"chords": ["Em", "C", "D", "B7"], "drums": "sparse", "arp": "pad", "bass": "root",
                  "melody": "E5:2 G5:2 B5:4 A5:2 G5:2 A5:4 B5:2 D6:2 E6:6 D6:2 B5:4 "
                            "C6:4 B5:2 A5:2 G5:4 A5:4 B5:8 F#5:4 D#5:4"},
        "A": {"chords": ["Em", "C", "D", "B7"], "drums": "rock", "arp": "up", "bass": "pump",
              "melody": "E5:1 r:1 E5:2 G5:2 B5:2 A5:2 G5:2 E5:2 G5:2 A5:3 B5:3 D6:2 E6:4 D6:2 B5:2 "
                        "C6:2 C6:2 B5:2 A5:2 G5:2 A5:2 B5:4 B5:6 A5:2 G5:2 F#5:2 D#5:4"},
        "B": {"chords": ["C", "D", "Em", "B7"], "drums": "drive", "arp": "updown", "bass": "pump",
              "melody": "G5:2 A5:2 B5:4 C6:4 B5:4 A5:2 B5:2 C6:4 D6:8 E6:4 D6:2 C6:2 B5:4 G5:4 A5:8 B5:8"},
    },
    "arrangement": [("intro", {"fill": False}), ("A", CR), ("B", V), ("A", EC), ("B", HA)],
    "restart": 1,
}

MENU = {
    "name": "Control Room", "bpm": 112, "style": "funk",
    "sections": {
        "A": {"chords": ["Gmaj7", "Em7", "Cmaj7", "D"], "drums": "funk", "arp": "pulse", "bass": "walk",
              "melody": "B5:3 A5:1 G5:2 D5:2 E5:4 r:4 G5:2 A5:2 B5:2 D6:2 B5:4 A5:4 "
                        "G5:3 E5:1 G5:2 A5:2 B5:4 r:4 A5:2 G5:2 F#5:2 D5:2 F#5:8"},
        "B": {"chords": ["Cmaj7", "D", "Bm7", "Em7"], "drums": "funk", "arp": "updown", "bass": "walk",
              "melody": "E6:4 D6:2 B5:2 C6:4 B5:4 A5:2 B5:2 C6:2 D6:2 A5:8 F#5:2 A5:2 D6:4 B5:4 A5:4 "
                        "G5:4 E5:4 D5:4 E5:4"},
    },
    "arrangement": [("A", V), ("A", EC), ("B", V), ("B", HA)],
}

W1 = {
    "name": "Assembly Line Riot", "bpm": 144, "style": "bright", "hot_drums": "blast",
    "sections": {
        "A": {"chords": ["Em", "Em", "C", "D"], "drums": "rock", "arp": "up", "bass": "pump",
              "melody": "E5:2 E5:1 r:1 E5:2 G5:2 A5:2 G5:2 E5:2 D5:2 E5:2 r:2 B4:2 D5:2 E5:4 r:4 "
                        "C5:2 C5:1 r:1 E5:2 G5:2 C6:2 B5:2 G5:2 E5:2 D5:2 F#5:2 A5:2 D6:2 C6:2 A5:2 F#5:4"},
        "B": {"chords": ["G", "D", "Em", "C"], "drums": "drive", "arp": "up", "bass": "pump",
              "melody": "B5:4 A5:2 B5:2 D6:4 B5:4 A5:4 F#5:2 A5:2 D6:6 r:2 G5:2 A5:2 B5:4 E6:4 D6:2 B5:2 "
                        "C6:4 B5:2 A5:2 G5:2 E5:2 G5:4"},
        "C": {"chords": ["Am", "Am", "B7", "B7"], "drums": "half", "arp": "updown", "bass": "root",
              "melody": "A5:6 C6:2 E6:4 D6:4 C6:4 B5:2 A5:2 E5:8 F#5:4 A5:4 B5:4 D#6:4 F#6:8 D#6:4 B5:4"},
    },
    "arrangement": [("A", CR), ("A", EC), ("B", CR), ("B", HA), ("C", V), ("A", UP), ("B", dict(CR, harmony=True))],
}

W2 = {
    "name": "Cryostasis", "bpm": 108, "style": "cold", "hot_drums": "half",
    "sections": {
        "A": {"chords": ["Bm", "G", "Em", "F#"], "drums": "sparse", "arp": "bell", "bass": "drone", "arp_octave": 5,
              "melody": "F#5:6 D5:2 B4:4 C#5:4 D5:4 E5:4 G5:6 F#5:2 E5:6 B4:2 G5:4 F#5:4 E5:2 D5:2 C#5:4 A#4:8"},
        "B": {"chords": ["G", "A", "F#m", "Bm"], "drums": "half", "arp": "up", "arp_step": 2, "bass": "root",
              "melody": "B5:4 A5:2 G5:2 D6:8 C#6:4 B5:2 A5:2 E5:8 F#5:4 A5:4 C#6:4 F#6:4 D6:6 C#6:2 B5:8"},
        "C": {"chords": ["Bm", "Bm", "G", "F#"], "drums": "none", "arp": "bell", "bass": "drone",
              "melody": "r:8 B5:4 C#6:4 D6:12 C#6:4 B5:8 F#5:8 G5:4 A5:4 A#5:8"},
    },
    "arrangement": [("C", {"fill": False}), ("A", V), ("A", EC), ("B", V), ("B", EC), ("C", PAD)],
    "restart": 1,
}

W3 = {
    "name": "Magma Rush", "bpm": 168, "style": "aggressive", "hot_drums": "blast",
    "sections": {
        "A": {"chords": ["Am", "A#", "Am", "E"], "drums": "drive", "arp": "up", "bass": "gallop",
              "melody": "A5:2 A5:1 r:1 C6:2 A5:2 E5:2 F5:2 E5:2 C5:2 D5:2 F5:2 A#5:4 A5:2 F5:2 D5:4 "
                        "A5:2 C6:2 E6:4 D6:2 C6:2 A5:4 G#5:4 B5:4 D6:2 B5:2 G#5:4"},
        "B": {"chords": ["F", "G", "Am", "E"], "drums": "blast", "arp": "updown", "bass": "pump",
              "melody": "C6:4 A5:2 C6:2 F6:8 D6:4 B5:2 D6:2 G6:8 E6:2 D6:2 C6:2 B5:2 A5:4 C6:4 B5:4 G#5:4 E5:8"},
        "C": {"chords": ["Dm", "Am", "E", "Am"], "drums": "half", "arp": "pad", "bass": "root",
              "melody": "F5:8 E5:4 D5:4 E5:8 C5:4 A4:4 G#4:4 B4:4 E5:4 G#5:4 A5:16"},
    },
    "arrangement": [("A", CR), ("A", EC), ("B", CR), ("C", V), ("A", HA), ("B", dict(CR, echo=True))],
}

W4 = {
    "name": "Neon Overclock", "bpm": 118, "style": "funk", "hot_drums": "drive",
    "sections": {
        "A": {"chords": ["F#m7", "B", "F#m7", "B"], "drums": "funk", "arp": "pulse", "bass": "funk", "staccato": True,
              "melody": "C#6:2 r:1 A5:1 B5:2 C#6:2 r:2 E6:2 C#6:4 B5:2 A5:1 r:1 F#5:4 D#5:2 E5:2 F#5:4 "
                        "C#6:2 r:1 A5:1 B5:2 C#6:2 r:2 F#6:2 E6:4 D#6:2 C#6:2 B5:4 A5:2 B5:2 C#6:4"},
        "B": {"chords": ["Dmaj7", "E", "C#m7", "F#m7"], "drums": "four", "arp": "updown", "bass": "walk",
              "melody": "A5:4 C#6:2 E6:2 F#6:8 E6:2 D#6:2 B5:4 G#5:8 G#5:2 B5:2 C#6:4 E6:4 D#6:4 C#6:8 r:4 A5:2 B5:2"},
    },
    "arrangement": [("A", CR), ("A", EC), ("B", V), ("B", HA), ("A", UP)],
}

W5 = {
    "name": "Event Horizon", "bpm": 100, "style": "cosmic", "hot_drums": "seven",
    "sections": {
        "A": {"chords": ["E", "F#", "G#m", "F#"], "drums": "seven", "arp": "rand", "bass": "seven",
              "melody": "B5:3 A#5:2 G#5:2 F#5:3 E5:6 F#5:3 G#5:2 A#5:2 C#6:3 B5:6 D#6:3 C#6:2 B5:2 G#5:3 E6:6 "
                        "D#6:5 C#6:5 A#5:6"},
        "B": {"chords": ["C", "D", "E", "F#"], "drums": "half", "arp": "up", "arp_step": 3, "bass": "drone",
              "melody": "E6:3 D6:3 C6:4 G5:6 F#6:3 E6:3 D6:4 A5:6 G#6:3 F#6:3 E6:4 B5:6 A#5:4 C#6:4 F#6:8"},
    },
    "arrangement": [("A", V), ("A", EC), ("B", PAD), ("A", HA), ("B", EC)],
}

W6 = {
    "name": "Total Meltdown", "bpm": 152, "style": "epic", "hot_drums": "blast",
    "sections": {
        "A": {"chords": ["Cm", "G#", "A#", "G7"], "drums": "drive", "arp": "up", "bass": "pump",
              "melody": "C5:1 r:1 C5:2 Eb5:2 G5:2 F5:2 Eb5:2 C5:2 Eb5:2 F5:3 G5:3 Bb5:2 C6:4 Bb5:2 G5:2 "
                        "Ab5:2 Ab5:2 G5:2 F5:2 Eb5:2 F5:2 G5:4 G5:6 F5:2 Eb5:2 D5:2 B4:4"},
        "B": {"chords": ["Cm", "C#", "Cm", "G"], "drums": "blast", "arp": "updown", "bass": "gallop",
              "melody": "C6:2 C6:1 r:1 Eb6:2 C6:2 G5:2 Ab5:2 G5:2 Eb5:2 F5:2 Ab5:2 Db6:4 C6:2 Ab5:2 F5:4 "
                        "C6:2 Eb6:2 G6:4 F6:2 Eb6:2 C6:4 B5:4 D6:4 F6:2 D6:2 B5:4"},
        "C": {"chords": ["G#", "A#", "Cm", "Cm"], "drums": "funk", "arp": "bell", "bass": "funk",
              "melody": "Eb6:6 C6:2 Ab5:4 Bb5:4 C6:4 D6:4 F6:6 Eb6:2 G6:2 r:1 Eb6:1 F6:2 G6:2 r:2 C7:2 G6:4 C7:16"},
    },
    "arrangement": [("A", CR), ("A", EC), ("B", CR), ("C", V), ("B", HA), ("A", dict(UP, crash=True))],
}

BOSS = {
    "name": "Mechanical Menace", "bpm": 150, "style": "aggressive",
    "sections": {
        "A": {"chords": ["Dm", "A#", "C", "A"], "drums": "drive", "arp": "up", "bass": "gallop",
              "melody": "D5:2 D5:2 F5:2 A5:2 D6:4 C6:2 A5:2 Bb5:4 A5:2 F5:2 D5:4 F5:4 "
                        "E5:2 G5:2 C6:4 Bb5:2 A5:2 G5:4 A5:4 C#6:4 E6:4 A6:4"},
        "B": {"chords": ["Gm", "Dm", "D#", "A"], "drums": "blast", "arp": "updown", "bass": "pump",
              "melody": "G5:4 Bb5:4 D6:8 F6:4 E6:2 D6:2 A5:8 Bb5:4 G5:2 Bb5:2 Eb6:8 E6:4 C#6:4 A5:2 G5:2 E5:4"},
    },
    "arrangement": [("A", CR), ("A", EC), ("B", CR), ("B", HA)],
}

FINAL = {
    "name": "Core of Mayhem", "bpm": 166, "style": "epic",
    "sections": {
        "A": {"chords": ["C#m", "A", "B", "G#"], "drums": "drive", "arp": "up", "bass": "gallop",
              "melody": "C#6:2 C#6:2 E6:2 G#6:2 F#6:2 E6:2 D#6:2 E6:2 C#6:4 A5:4 E5:4 A5:4 "
                        "B5:2 D#6:2 F#6:4 E6:2 D#6:2 B5:4 G#5:2 C6:2 D#6:4 G#6:4 F#6:2 D#6:2"},
        "B": {"chords": ["A", "B", "C#m", "C#m"], "drums": "blast", "arp": "updown", "bass": "pump",
              "melody": "E6:4 F#6:4 A6:8 G#6:4 F#6:4 D#6:8 E6:2 F#6:2 G#6:4 B6:4 G#6:4 C#7:16"},
        "C": {"chords": ["C#m", "A", "F#m", "G#"], "drums": "drive", "arp": "up", "bass": "pump",
              "melody": "C#6:1 r:1 C#6:2 E6:2 G#6:2 F#6:2 E6:2 C#6:2 E6:2 F#6:3 G#6:3 B6:2 C#7:4 B6:2 G#6:2 "
                        "A6:2 A6:2 G#6:2 F#6:2 E6:2 F#6:2 G#6:4 G#6:6 F#6:2 E6:2 D#6:2 C6:4"},
    },
    "arrangement": [("A", CR), ("A", EC), ("B", CR), ("C", HA), ("B", dict(CR, echo=True)), ("C", UP)],
}

VICTORY = {
    "name": "Stage Clear", "bpm": 140, "style": "bright",
    "sections": {
        "A": {"rows": 32, "chords": ["C", "F", "G", "C"], "drums": "rock", "arp": "up", "bass": "pump",
              "melody": "C5:2 E5:2 G5:2 C6:4 G5:2 C6:4 D6:2 E6:2 F6:2 E6:4 D6:2 C6:4"},
    },
    "arrangement": [("A", CR), ("A", HA)],
}

GAME_OVER = {
    "name": "Shutdown", "bpm": 80, "style": "cold",
    "sections": {
        "A": {"rows": 32, "chords": ["Am", "F", "Dm", "E"], "drums": "none", "arp": "pad", "bass": "root",
              "melody": "E5:4 D5:4 C5:4 B4:4 A4:8 r:8"},
    },
    "arrangement": [("A", {"fill": False})],
}

ENDING = {
    "name": "Afterglow", "bpm": 120, "style": "bright",
    "sections": {
        "A": {"chords": ["E", "C#m", "A", "B"], "drums": "half", "arp": "updown", "bass": "walk",
              "melody": "E5:2 G#5:2 B5:4 A5:2 G#5:2 A5:4 B5:2 D#6:2 E6:6 D#6:2 B5:4 "
                        "C#6:4 B5:2 A5:2 G#5:4 A5:4 B5:8 F#5:4 D#5:4"},
        "B": {"chords": ["A", "B", "E", "E"], "drums": "rock", "arp": "up", "bass": "pump",
              "melody": "A5:2 B5:2 C#6:4 E6:4 C#6:4 B5:2 C#6:2 D#6:4 F#6:8 G#6:4 F#6:2 E6:2 D#6:4 B5:4 E6:16"},
    },
    "arrangement": [("A", PAD), ("A", EC), ("B", CR), ("B", HA)],
}

CLASSIC = {
    "name": "Quiet Assembly", "bpm": 120, "style": "bright",
    "sections": {
        "A": {"chords": ["Dm7", "G", "Dm7", "G"], "drums": "break", "arp": "updown", "bass": "walk",
              "melody": "A5:4 F5:2 E5:2 D5:4 F5:4 G5:2 A5:2 B5:4 A5:4 G5:4 F5:4 D5:2 E5:2 F5:4 A5:4 C6:4 B5:4 G5:8"},
        "B": {"chords": ["F", "C", "G", "A"], "drums": "break", "arp": "up", "bass": "walk",
              "melody": "C6:4 A5:2 G5:2 F5:4 A5:4 G5:4 E5:2 D5:2 C5:4 E5:4 D5:2 E5:2 G5:4 B5:4 D6:4 C#6:8 E6:8"},
    },
    "arrangement": [("A", V), ("A", EC), ("B", V), ("B", HA)],
}

SONGS = {
    "mus_title": TITLE, "mus_menu": MENU, "mus_w1": W1, "mus_w2": W2, "mus_w3": W3, "mus_w4": W4, "mus_w5": W5,
    "mus_w6": W6, "mus_boss": BOSS, "mus_final": FINAL, "mus_victory": VICTORY, "mus_gameover": GAME_OVER,
    "mus_ending": ENDING, "mus_classic": CLASSIC,
}

HOT = ["mus_w1", "mus_w2", "mus_w3", "mus_w4", "mus_w5", "mus_w6", "mus_boss", "mus_final"]
