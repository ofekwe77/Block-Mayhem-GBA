"""Indexed BMP writer for Butano/grit (uncompressed, no colour-space header).

Colours are given as (r, g, b) tuples with 8-bit channels; they are snapped to
the GBA's 15-bit colour grid so the converted palette is exact.
"""
import struct


def snap(c):
    """Snap an (r, g, b) colour to the GBA 5-bit-per-channel grid."""
    return tuple(min(248, (max(0, min(255, v)) + 4) // 8 * 8) for v in c)


def write_bmp(path, width, height, pixels, palette, bpp=None):
    """Write an indexed BMP.

    pixels  : list of rows (top to bottom), each a list of palette indices.
    palette : list of (r, g, b); padded to 16 or 256 entries.
    bpp     : 4 or 8 (default: 4 if the palette fits in 16 colours).
    """
    if bpp is None:
        bpp = 4 if len(palette) <= 16 else 8
    ncolors = 16 if bpp == 4 else 256
    pal = [snap(c) for c in palette] + [(0, 0, 0)] * (ncolors - len(palette))
    pal = pal[:ncolors]
    row_bytes = (width * bpp + 7) // 8
    stride = (row_bytes + 3) & ~3
    data = bytearray()
    for y in range(height - 1, -1, -1):          # BMP rows are stored bottom-up
        row = pixels[y]
        out = bytearray()
        if bpp == 8:
            for x in range(width):
                out.append(row[x] & 0xFF)
        else:
            for x in range(0, width, 2):
                hi = row[x] & 0xF
                lo = (row[x + 1] & 0xF) if x + 1 < width else 0
                out.append((hi << 4) | lo)
        out += b"\0" * (stride - len(out))
        data += out
    header_size = 14 + 40 + ncolors * 4
    file_size = header_size + len(data)
    with open(path, "wb") as f:
        f.write(b"BM")
        f.write(struct.pack("<IHHI", file_size, 0, 0, header_size))
        f.write(struct.pack("<IiiHHIIiiII", 40, width, height, 1, bpp, 0, len(data), 2835, 2835,
                            ncolors, ncolors))
        for (r, g, b) in pal:
            f.write(bytes((b, g, r, 0)))
        f.write(data)


def write_json(path, obj):
    import json
    with open(path, "w", newline="\n") as f:
        json.dump(obj, f, indent=4)
        f.write("\n")
