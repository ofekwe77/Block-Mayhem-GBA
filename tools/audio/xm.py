"""Minimal FastTracker 2 XM writer (enough for Maxmod / mmutil)."""
import struct

KEY_OFF = 97


class Instrument:
    def __init__(self, name, data, loop_start=0, loop_len=0, volume=64, relative_note=0, finetune=0,
                 panning=128, envelope=None, fadeout=1024, sixteen_bit=False):
        self.name = name
        self.data = data                    # list of ints (-128..127 or -32768..32767)
        self.loop_start = loop_start
        self.loop_len = loop_len
        self.volume = volume
        self.relative_note = relative_note
        self.finetune = finetune
        self.panning = panning
        self.envelope = envelope or []      # list of (tick, value 0..64)
        self.fadeout = fadeout
        self.sixteen_bit = sixteen_bit


class Pattern:
    def __init__(self, rows, channels):
        self.rows = rows
        self.channels = channels
        self.cells = [[None] * channels for _ in range(rows)]

    def set(self, row, ch, note=0, instr=0, vol=0, fx=0, param=0):
        if 0 <= row < self.rows:
            self.cells[row][ch] = (note, instr, vol, fx, param)


class Module:
    def __init__(self, name, channels, speed=6, bpm=125):
        self.name = name
        self.channels = channels
        self.speed = speed
        self.bpm = bpm
        self.patterns = []
        self.order = []
        self.instruments = []
        self.restart = 0

    def write(self, path):
        out = bytearray()
        out += b"Extended Module: "
        out += self.name.encode("ascii")[:20].ljust(20, b" ")
        out += b"\x1a"
        out += b"BlockMayhem Composer".ljust(20, b" ")
        out += struct.pack("<H", 0x0104)
        order = self.order[:256]
        out += struct.pack("<IHHHHHHHH", 276, len(order), self.restart, self.channels, len(self.patterns),
                           len(self.instruments), 1, self.speed, self.bpm)
        out += bytes(order) + bytes(256 - len(order))

        for p in self.patterns:
            data = bytearray()
            for row in p.cells:
                for c in row:
                    if c is None:
                        data.append(0x80)
                        continue
                    note, instr, vol, fx, param = c
                    mask = 0x80
                    body = bytearray()
                    if note:
                        mask |= 1
                        body.append(note)
                    if instr:
                        mask |= 2
                        body.append(instr)
                    if vol:
                        mask |= 4
                        body.append(vol)
                    if fx:
                        mask |= 8
                        body.append(fx)
                    if param:
                        mask |= 16
                        body.append(param)
                    data.append(mask)
                    data += body
            out += struct.pack("<IBHH", 9, 0, p.rows, len(data))
            out += data

        for ins in self.instruments:
            env = ins.envelope[:12]
            out += struct.pack("<I", 263)
            out += ins.name.encode("ascii")[:22].ljust(22, b"\0")
            out += struct.pack("<BH", 0, 1)
            out += struct.pack("<I", 40)
            out += bytes(96)                            # all notes -> sample 0
            pts = bytearray()
            for (t, v) in env:
                pts += struct.pack("<HH", t, v)
            pts += bytes(48 - len(pts))
            out += pts
            out += bytes(48)                            # panning envelope
            out += struct.pack("<BB", len(env), 0)
            out += struct.pack("<BBB", 0, 0, 0)         # vol sustain / loop
            out += struct.pack("<BBB", 0, 0, 0)         # pan sustain / loop
            out += struct.pack("<BB", 1 if env else 0, 0)
            out += struct.pack("<BBBB", 0, 0, 0, 0)     # vibrato
            out += struct.pack("<H", ins.fadeout)
            out += bytes(22)
            # sample header
            n = len(ins.data)
            width = 2 if ins.sixteen_bit else 1
            loop_type = 1 if ins.loop_len > 0 else 0
            flags = loop_type | (0x10 if ins.sixteen_bit else 0)
            out += struct.pack("<IIIBbBBbB", n * width, ins.loop_start * width, ins.loop_len * width, ins.volume,
                               ins.finetune, flags, ins.panning, ins.relative_note, 0)
            out += ins.name.encode("ascii")[:22].ljust(22, b"\0")
            # delta-encoded sample data
            prev = 0
            if ins.sixteen_bit:
                for v in ins.data:
                    d = (v - prev) & 0xFFFF
                    out += struct.pack("<H", d)
                    prev = v
            else:
                for v in ins.data:
                    d = (v - prev) & 0xFF
                    out.append(d)
                    prev = v
        with open(path, "wb") as f:
            f.write(out)
