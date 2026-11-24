
import struct
import sys

def u32(data, offset):
    return struct.unpack_from("<I", data, offset)[0]

def parse_rich_header(path):
    with open(path, "rb") as f:
        data = f.read()

    if data[:2] != b"MZ":
        print("Keine gültige MZ-Signatur!")
        return

    pe_offset = u32(data, 0x3C)
    if data[pe_offset:pe_offset + 4] != b"PE\x00\x00":
        print("Keine gültige PE-Signatur!")
        return

    # Rich Header befindet sich vor dem PE Header
    rich_offset = data.find(b"Rich", 0x40, pe_offset)

    if rich_offset == -1 or rich_offset + 8 > pe_offset:
        print("Kein Rich Header gefunden!")
        return

    key = u32(data, rich_offset + 4)

    # DanS ist ebenfalls XOR-verschleiert
    dans_encoded = 0x536E6144 ^ key  # "DanS" als Little Endian

    dans_offset = -1
    for offset in range(0x40, rich_offset, 4):
        if u32(data, offset) == dans_encoded:
            dans_offset = offset
            break

    if dans_offset == -1:
        print("Keine DanS-Signatur gefunden!")
        return

    print(f"PE Header:    0x{pe_offset:08X}")
    print(f"Rich Header:  0x{rich_offset:08X}")
    print(f"DanS:         0x{dans_offset:08X}")
    print(f"XOR Key:      0x{key:08X}")

    print("\nOffset    Encoded     Decoded")
    print("-" * 36)

    for offset in range(dans_offset, rich_offset, 4):
        encoded = u32(data, offset)
        decoded = encoded ^ key

        print(f"{offset:08X}  {encoded:08X}  {decoded:08X}")

    print("\nCompiler-Einträge:")
    print("ProdID  Build   Count")
    print("-" * 27)

    # Erste 4 DWORDs sind DanS und drei Nullwerte
    for offset in range(dans_offset + 16, rich_offset, 8):
        if offset + 8 > rich_offset:
            break

        comp_id = u32(data, offset) ^ key
        count = u32(data, offset + 4) ^ key

        prod_id = comp_id >> 16
        build = comp_id & 0xFFFF

        print(f"{prod_id:6d}  {build:5d}  {count:6d}")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Aufruf: python rich_header.py <datei.exe>")
    else:
        parse_rich_header(sys.argv[1])
