"""Crops and upscales a PNG screenshot (nearest neighbour) for close inspection.
Usage: python zoom.py in.png out.png x y w h scale"""
import struct, sys, zlib

def read_png(path):
    d = open(path, "rb").read()
    pos = 8; idat = b""; w = h = 0
    while pos < len(d):
        n = struct.unpack(">I", d[pos:pos+4])[0]; t = d[pos+4:pos+8]; body = d[pos+8:pos+8+n]
        if t == b"IHDR": w, h = struct.unpack(">II", body[:8])
        if t == b"IDAT": idat += body
        pos += 12 + n
    raw = zlib.decompress(idat); rows = []; stride = w * 3; prev = bytearray(stride); p = 0
    for y in range(h):
        f = raw[p]; line = bytearray(raw[p+1:p+1+stride]); p += 1 + stride
        for i in range(stride):
            a = line[i-3] if i >= 3 else 0; b = prev[i]; c = prev[i-3] if i >= 3 else 0
            if f == 1: line[i] = (line[i] + a) & 255
            elif f == 2: line[i] = (line[i] + b) & 255
            elif f == 3: line[i] = (line[i] + (a + b) // 2) & 255
            elif f == 4:
                pa, pb, pc = abs(b - c), abs(a - c), abs(a + b - 2 * c)
                pr = a if pa <= pb and pa <= pc else (b if pb <= pc else c)
                line[i] = (line[i] + pr) & 255
        rows.append(line); prev = line
    return w, h, rows

def write_png(path, w, h, rows):
    raw = b"".join(b"\0" + bytes(r) for r in rows)
    def chunk(t, data):
        c = struct.pack(">I", len(data)) + t + data
        return c + struct.pack(">I", zlib.crc32(t + data) & 0xFFFFFFFF)
    open(path, "wb").write(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
                           + chunk(b"IDAT", zlib.compress(raw)) + chunk(b"IEND", b""))

src, dst = sys.argv[1], sys.argv[2]
x0, y0, cw, ch, s = map(int, sys.argv[3:8])
w, h, rows = read_png(src)
out = []
for y in range(y0, y0 + ch):
    line = bytearray()
    for x in range(x0, x0 + cw):
        px = rows[y][x*3:x*3+3]
        line += bytes(px) * s
    for _ in range(s):
        out.append(line)
write_png(dst, cw * s, ch * s, out)
