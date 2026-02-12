#!/usr/bin/env python3
import sys
import os
import re

def make_c_identifier(filename):
    name = os.path.basename(filename)
    name = re.sub(r'[^0-9a-zA-Z_]', '_', name)
    if name[0].isdigit():
        name = "_" + name
    return name

def main(binfile):
    varname = make_c_identifier(binfile)

    base, _ = os.path.splitext(binfile)
    outfile = base + ".cpp"

    with open(binfile, "rb") as f:
        data = f.read()

    with open(outfile, "w", encoding="utf-8") as o:
        o.write("#include <stdint.h>\n\n")
        o.write(f"const uint8_t __attribute__((aligned(4))) {varname}[] = {{\n")

        for i, b in enumerate(data):
            if i % 16 == 0:
                o.write("    ")
            o.write(f"0x{b:02X}, ")
            if i % 16 == 15:
                o.write("\n")

        if len(data) % 16 != 0:
            o.write("\n")

        o.write("};\n\n")
        o.write(f"const unsigned int {varname}_size = {len(data)};\n")

    print(f"generated: {outfile}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("usage: bin2c.py input.bin")
        sys.exit(1)

    main(sys.argv[1])
