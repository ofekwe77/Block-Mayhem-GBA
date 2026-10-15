"""Converts generated indexed BMPs to PNG previews (for visual inspection).

Usage: python preview.py input.bmp output.png [scale] [bank_for_4bpp_with_external_palette]
"""
import struct
import sys
import zlib


def read_bmp(path):
    d = open(path, "rb").read()
    off = struct.unpack("<I", d[10:14])[0]
    w, h = struct.unpack("<ii", d[18:26])
    bpp = struct.unpack("<H", d[28:30])[0]
    ncol = struct.unpack("<I", d[46:50])[0] or (1 << bpp)
    pal = []
    for i in range(ncol):
        b, g, r, _ = d[54 + i * 4: 58 + i * 4]
        pal.append((r, g, b))
    stride = ((w * bpp + 7) // 8 + 3) & ~3
    rows = []
    for y in range(h):
        row_off = off + (h - 1 - y) * stride
        row = []
        for x in range(w):
            if bpp == 8:
                row.append(d[row_off + x])
            else:
                byte = d[row_off + x // 2]
                row.append((byte >> 4) if x % 2 == 0 else (byte & 15))
        rows.append(row)
    return w, h, rows, pal


def write_png(path, w, h, rgb_rows):
    raw = b"".join(b"\0" + bytes(sum(([r, g, b] for (r, g, b) in row), [])) for row in rgb_rows)

    def chunk(t, data):
        c = struct.pack(">I", len(data)) + t + data
        return c + struct.pack(">I", zlib.crc32(t + data) & 0xFFFFFFFF)
    png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b"")
    open(path, "wb").write(png)


def main():
    src, dst = sys.argv[1], sys.argv[2]
    scale = int(sys.argv[3]) if len(sys.argv) > 3 else 2
    w, h, rows, pal = read_bmp(src)
    if len(sys.argv) > 4:
        ext_w, ext_h, _, ext_pal = read_bmp(sys.argv[4])
        bank = int(sys.argv[5]) if len(sys.argv) > 5 else 3
        pal = ext_pal[bank * 16: bank * 16 + 16]
    bg = (255, 0, 255)
    out = []
    for y in range(h):
        line = []
        for x in range(w):
            i = rows[y][x]
            c = bg if (i % 16 == 0 and len(pal) <= 16) else pal[i % len(pal)]
            line.extend([c] * scale)
        for _ in range(scale):
            out.append(line)
    write_png(dst, w * scale, h * scale, out)


if __name__ == "__main__":
    main()
