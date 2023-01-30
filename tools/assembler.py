# extremely basic assembler for testing purposes

import sys
import re

labels = {}

def write_byte(bytes):
    with open("out.bin", "ab") as f:
        f.write(bytearray(bytes))

def parselabels():
    with open(sys.argv[1]) as f:
        bytes = 0

        for line in f:
            if line[0] == "#":
                continue

            if line[0] == ".":
                label = line[1:].strip()
                labels[label] = bytes
                print("Label: " + label + " = " + str(bytes))

                continue

            line = line.strip()
            token = re.split(r'[,]', line)

            opcode = token[0].upper().strip()

            if opcode == "NOP":
                bytes += 1
            elif opcode == "HLT":
                bytes += 1
            elif opcode == "ADD":
                bytes += 3
            elif opcode == "ADDI":
                bytes += 6
            elif opcode == "AND":
                bytes += 3
            elif opcode == "ANDI":
                bytes += 6
            elif opcode == "BEQ":
                bytes += 5
            elif opcode == "BGE":
                bytes += 5
            elif opcode == "BGEU":
                bytes += 5
            elif opcode == "BGT":
                bytes += 5
            elif opcode == "BGTU":
                bytes += 5
            elif opcode == "BLE":
                bytes += 5
            elif opcode == "BLEU":
                bytes += 5
            elif opcode == "BLT":
                bytes += 5
            elif opcode == "BLTU":
                bytes += 5
            elif opcode == "BNE":
                bytes += 5
            elif opcode == "CMP":
                bytes += 3
            elif opcode == "CMPI":
                bytes += 6
            elif opcode == "DIV":
                bytes += 3
            elif opcode == "DIVI":
                bytes += 6
            elif opcode == "DIVU":
                bytes += 3
            elif opcode == "JMP":
                bytes += 2
            elif opcode == "JMPA":
                bytes += 5
            elif opcode == "JSR":
                bytes += 2
            elif opcode == "JSRA":
                bytes += 5
            elif opcode == "LD":
                bytes += 3
            elif opcode == "LDA":
                bytes += 6
            elif opcode == "LDI":
                bytes += 6
            elif opcode == "LDR":
                bytes += 3
            elif opcode == "MUL":
                bytes += 3
            elif opcode == "MULI":
                bytes += 6
            elif opcode == "MULU":
                bytes += 3
            elif opcode == "NEG":
                bytes += 2
            elif opcode == "NOT":
                bytes += 2
            elif opcode == "OR":
                bytes += 3
            elif opcode == "ORI":
                bytes += 6
            elif opcode == "POP":
                bytes += 2
            elif opcode == "PUSH":
                bytes += 2
            elif opcode == "RET":
                bytes += 1
            elif opcode == "STB":
                bytes += 3
            elif opcode == "STA":
                bytes += 6
            elif opcode == "SUB":
                bytes += 3
            elif opcode == "SUBI":
                bytes += 6
            elif opcode == "XOR":
                bytes += 3
            elif opcode == "XORI":
                bytes += 6
            elif opcode == "RND":
                bytes += 3
            elif opcode == "INT":
                bytes += 2

        return bytes

def main():
    if len(sys.argv) != 2:
        print("Usage: assembler.py <filename>")
        return

    with open("out.bin", "wb") as f:
        f.close()

    bytes = parselabels()

    with open(sys.argv[1]) as f:
        for line in f:
            if line[0] == "#":
                continue

            if line[0] == ".":
                continue

            if line[0] == "\n":
                continue

            line = line.strip()
            token = re.split(r'[,]', line)

            opcode = token[0].upper().strip()

            if opcode == "NOP":
                write_byte([0x00])
            elif opcode == "HLT":
                write_byte([0x01])
            elif opcode == "ADD":
                r1 = int(token[1].strip())
                r2 = int(token[2].strip())

                write_byte([0x02, r1, r2])
            elif opcode == "ADDI":
                r1 = int(token[1].strip())
                imm = int(token[2].strip())

                write_byte([0x03, r1, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "AND":
                r1 = int(token[1].strip())
                r2 = int(token[2].strip())

                write_byte([0x04, r1, r2])
            elif opcode == "ANDI":
                r1 = int(token[1].strip())
                imm = int(token[2].strip())

                write_byte([0x05, r1, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "BEQ":
                imm = labels[token[1].strip()]

                write_byte([0x06, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "BGE":
                imm = labels[token[1].strip()]

                write_byte([0x07, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "BGEU":
                imm = labels[token[1].strip()]

                write_byte([0x08, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "BGT":
                imm = labels[token[1].strip()]

                write_byte([0x09, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "BGTU":
                imm = labels[token[1].strip()]

                write_byte([0x0A, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "BLE":
                imm = labels[token[1].strip()]

                write_byte([0x0B, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "BLEU":
                imm = labels[token[1].strip()]

                write_byte([0x0C, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "BLT":
                imm = labels[token[1].strip()]

                write_byte([0x0D, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "BLTU":
                imm = labels[token[1].strip()]

                write_byte([0x0E, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "BNE":
                imm = labels[token[1].strip()]

                write_byte([0x0F, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "CMP":
                r1 = int(token[1].strip())
                r2 = int(token[2].strip())

                write_byte([0x10, r1, r2])
            elif opcode == "CMPI":
                r1 = int(token[1].strip())
                imm = int(token[2].strip())

                write_byte([0x11, r1, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "DIV":
                r1 = int(token[1].strip())
                r2 = int(token[2].strip())

                write_byte([0x12, r1, r2])
            elif opcode == "DIVI":
                r1 = int(token[1].strip())
                imm = int(token[2].strip())

                write_byte([0x13, r1, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "DIVU":
                r1 = int(token[1].strip())
                r2 = int(token[2].strip())

                write_byte([0x14, r1, r2])
            elif opcode == "JMP":
                r1 = int(token[1].strip())

                write_byte([0x15, r1])
            elif opcode == "JMPA":
                imm = labels[token[1].strip()]

                write_byte([0x16, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "JSR":
                r1 = int(token[1].strip())

                write_byte([0x17, r1])
            elif opcode == "JSRA":
                imm = labels[token[1].strip()]

                write_byte([0x18, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "LD":
                r1 = int(token[1].strip())
                r2 = int(token[2].strip())

                write_byte([0x19, r1, r2])
            elif opcode == "LDA":
                r1 = int(token[1].strip())
                imm = int(token[2].strip())

                write_byte([0x1A, r1, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "LDI":
                r1 = int(token[1].strip())
                imm = int(token[2].strip())

                write_byte([0x1B, r1, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "LDR":
                r1 = int(token[1].strip())
                r2 = int(token[2].strip())

                write_byte([0x1C, r1, r2])
            elif opcode == "MUL":
                r1 = int(token[1].strip())
                r2 = int(token[2].strip())

                write_byte([0x1D, r1, r2])
            elif opcode == "MULI":
                r1 = int(token[1].strip())
                imm = int(token[2].strip())

                write_byte([0x1E, r1, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "MULU":
                r1 = int(token[1].strip())
                r2 = int(token[2].strip())

                write_byte([0x1F, r1, r2])
            elif opcode == "NEG":
                r1 = int(token[1].strip())

                write_byte([0x20, r1])
            elif opcode == "NOT":
                r1 = int(token[1].strip())

                write_byte([0x21, r1])
            elif opcode == "OR":
                r1 = int(token[1].strip())
                r2 = int(token[2].strip())

                write_byte([0x22, r1, r2])
            elif opcode == "ORI":
                r1 = int(token[1].strip())
                imm = int(token[2].strip())

                write_byte([0x23, r1, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "POP":
                r1 = int(token[1].strip())

                write_byte([0x24, r1])
            elif opcode == "PUSH":
                r1 = int(token[1].strip())

                write_byte([0x25, r1])
            elif opcode == "RET":
                write_byte([0x26])
            elif opcode == "STB":
                r1 = int(token[1].strip())
                r2 = int(token[2].strip())

                write_byte([0x27, r1, r2])
            elif opcode == "STA":
                r1 = int(token[1].strip())
                imm = int(token[2].strip())

                write_byte([0x28, r1, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "SUB":
                r1 = int(token[1].strip())
                r2 = int(token[2].strip())

                write_byte([0x29, r1, r2])
            elif opcode == "SUBI":
                r1 = int(token[1].strip())
                imm = int(token[2].strip())

                write_byte([0x2A, r1, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "XOR":
                r1 = int(token[1].strip())
                r2 = int(token[2].strip())

                write_byte([0x2B, r1, r2])
            elif opcode == "XORI":
                r1 = int(token[1].strip())
                imm = int(token[2].strip())

                write_byte([0x2C, r1, (imm >> 24) & 0xFF, (imm >> 16) & 0xFF, (imm >> 8) & 0xFF, imm & 0xFF])
            elif opcode == "RND":
                r1 = int(token[1].strip())
                r2 = int(token[2].strip())

                write_byte([0x2D, r1, r2])
            elif opcode == "INT":
                r1 = int(token[1].strip())

                write_byte([0x2E, r1])

    print("Assembled " + str(bytes) + " bytes")

main()
