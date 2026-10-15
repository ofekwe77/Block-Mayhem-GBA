"""Tiny additive / subtractive synthesizer for original BLOCK MAYHEM sound effects.

Everything is generated from oscillators, noise and envelopes (no samples from
any other source). Output: 8-bit mono WAV at 22050 Hz (Butano / Maxmod format).
"""
import math
import random
import struct

RATE = 22050


class Sound:
    def __init__(self, seconds):
        self.n = int(seconds * RATE)
        self.buf = [0.0] * self.n

    def add(self, other_buf, start=0, gain=1.0):
        for i, v in enumerate(other_buf):
            j = start + i
            if 0 <= j < self.n:
                self.buf[j] += v * gain

    def normalize(self, peak=0.92):
        m = max(1e-6, max(abs(v) for v in self.buf))
        k = peak / m
        self.buf = [v * k for v in self.buf]
        return self

    def write(self, path):
        data = bytes(max(0, min(255, int(128 + v * 127))) for v in self.buf)
        with open(path, "wb") as f:
            f.write(b"RIFF" + struct.pack("<I", 36 + len(data)) + b"WAVE")
            f.write(b"fmt " + struct.pack("<IHHIIHH", 16, 1, 1, RATE, RATE, 1, 8))
            f.write(b"data" + struct.pack("<I", len(data)))
            f.write(data)


def env_adsr(n, a, d, s, r):
    """Attack/decay/sustain-level/release (seconds) envelope of n samples."""
    out = []
    an, dn, rn = int(a * RATE), int(d * RATE), int(r * RATE)
    for i in range(n):
        if i < an:
            v = i / max(1, an)
        elif i < an + dn:
            v = 1 - (1 - s) * (i - an) / max(1, dn)
        elif i < n - rn:
            v = s
        else:
            v = s * max(0.0, (n - i) / max(1, rn))
        out.append(v)
    return out


def osc(kind, freq_fn, seconds, duty=0.5, seed=1):
    """Oscillator with a frequency function f(t) (Hz). kind: square, tri, saw, sine, noise."""
    n = int(seconds * RATE)
    out = []
    phase = 0.0
    rnd = random.Random(seed)
    noise_val = 0.0
    noise_hold = 0
    for i in range(n):
        t = i / RATE
        f = freq_fn(t)
        phase += f / RATE
        p = phase % 1.0
        if kind == "square":
            v = 1.0 if p < duty else -1.0
        elif kind == "tri":
            v = 4 * p - 1 if p < 0.5 else 3 - 4 * p
        elif kind == "saw":
            v = 2 * p - 1
        elif kind == "sine":
            v = math.sin(2 * math.pi * p)
        else:
            # sample-and-hold noise; f controls the "pitch" of the noise
            hold = max(1, int(RATE / max(1.0, f)))
            if noise_hold <= 0:
                noise_val = rnd.uniform(-1, 1)
                noise_hold = hold
            noise_hold -= 1
            v = noise_val
        out.append(v)
    return out


def mul(a, b):
    return [x * y for x, y in zip(a, b)]


def lowpass(buf, k):
    out = []
    y = 0.0
    for v in buf:
        y += (v - y) * k
        out.append(y)
    return out


def tone(kind, f0, f1, seconds, a=0.002, d=0.05, s=0.6, r=0.05, duty=0.5, vib=0.0, seed=1, curve=1.0):
    """Pitch glide from f0 to f1 with an ADSR envelope."""
    def freq(t):
        k = (t / seconds) ** curve if seconds else 0
        f = f0 * ((f1 / f0) ** k) if f0 > 0 and f1 > 0 else f0 + (f1 - f0) * k
        if vib:
            f *= 1 + vib * math.sin(2 * math.pi * 7 * t)
        return f
    o = osc(kind, freq, seconds, duty, seed)
    return mul(o, env_adsr(len(o), a, d, s, r))


def arpeggio(kind, notes, step, seconds, duty=0.5, a=0.002, d=0.08, s=0.5, r=0.05):
    def freq(t):
        idx = int(t / step) % len(notes)
        return notes[idx]
    o = osc(kind, freq, seconds, duty)
    return mul(o, env_adsr(len(o), a, d, s, r))


def note_hz(name):
    names = {"C": 0, "C#": 1, "D": 2, "D#": 3, "E": 4, "F": 5, "F#": 6, "G": 7, "G#": 8, "A": 9, "A#": 10, "B": 11}
    pitch = name[:-1]
    octave = int(name[-1])
    semis = names[pitch] + (octave - 4) * 12 - 9
    return 440.0 * 2 ** (semis / 12)
