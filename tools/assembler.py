# extremely basic assembler for testing purposes

import sys
import re

def write_byte(bytes):
    with open("out.bin", "ab") as f:
        f.write(bytearray(bytes))

def main():
    if len(sys.argv) != 2:
        print("Usage: assembler.py <filename>")
        return

    with open("out.bin", "wb") as f:
        f.close()

    with open(sys.argv[1]) as f:
        for line in f:
            if line[0] == "#":
                continue

            if line[0] == ".":
                continue

            line = line.strip()
            token = re.split(r'[,]', line)

            opcode = token[0].upper().strip()

            print("Opcode: {}".format(opcode))

            if opcode == "ADD":
                r1 = int(token[1].strip())
                r2 = int(token[2].strip())

                write_byte([0x02, r1, r2])

main()
