"""Validates the GBA ROM image (PART 56.2 Build): size, entry branch, header fields, header complement
checksum, the boot logo region and the SRAM save-type marker emulators and flash carts use.

Usage: python3 tools/check_rom.py BlockMayhem.gba        (exit 0 = valid, 1 = invalid)
"""
import sys
import zlib

# CRC-32 of the 156-byte boot logo region (0x04-0x9F) as written by the devkitPro header tool (gbafix):
# the value every bootable GBA cartridge carries. A mismatch means real hardware would refuse to boot the ROM.
EXPECTED_LOGO_CRC = 0xD0BEB55E


def check(path):
    data = open(path, "rb").read()
    problems = []
    info = {}
    size = len(data)
    info["size"] = "%d bytes (%.2f MiB)" % (size, size / 1048576.0)
    if size < 0xC0:
        return ["file too small to hold a GBA header"], info
    if size > 32 * 1024 * 1024:
        problems.append("ROM larger than 32 MiB")
    if data[3] != 0xEA:
        problems.append("byte 3 is 0x%02X, expected 0xEA (ARM branch to the program entry)" % data[3])
    logo = data[0x04:0xA0]
    crc = zlib.crc32(logo) & 0xFFFFFFFF
    info["logo_crc"] = "0x%08X" % crc
    if crc != EXPECTED_LOGO_CRC:
        problems.append("boot logo region CRC 0x%08X, expected 0x%08X" % (crc, EXPECTED_LOGO_CRC))
    title = data[0xA0:0xAC]
    code = data[0xAC:0xB0]
    maker = data[0xB0:0xB2]
    info["title"] = title.rstrip(b"\0").decode("ascii", "replace")
    info["game_code"] = code.decode("ascii", "replace")
    info["maker"] = maker.decode("ascii", "replace")
    info["version"] = data[0xBC]
    if not all(32 <= b < 127 or b == 0 for b in title):
        problems.append("title is not ASCII")
    if not all(32 <= b < 127 for b in code):
        problems.append("game code is not printable ASCII")
    if data[0xB2] != 0x96:
        problems.append("fixed byte 0xB2 is 0x%02X, expected 0x96" % data[0xB2])
    complement = (-(sum(data[0xA0:0xBD]) + 0x19)) & 0xFF
    info["complement"] = "0x%02X" % data[0xBD]
    if data[0xBD] != complement:
        problems.append("header complement 0x%02X, expected 0x%02X" % (data[0xBD], complement))
    if b"SRAM_V" not in data:
        problems.append("no SRAM_Vnnn save-type marker (emulators would not provide save memory)")
    else:
        i = data.index(b"SRAM_V")
        info["save_type"] = data[i:i + 9].decode("ascii", "replace")
    return problems, info


def main():
    if len(sys.argv) != 2:
        print(__doc__)
        return 2
    problems, info = check(sys.argv[1])
    for k, v in info.items():
        print("  %-10s %s" % (k, v))
    for p in problems:
        print("  PROBLEM: " + p)
    print("ROM header check: " + ("PASS" if not problems else "FAIL"))
    return 0 if not problems else 1


if __name__ == "__main__":
    sys.exit(main())
