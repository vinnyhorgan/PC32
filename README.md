# PC32

## DESIGN

The PC32 is a fantasy educational computer designed to teach modern software stacks without the
complexity of modern hardware. It emulates a computer from the early 1990s, the era of the first
32-bit machines, while simplifying the hardware as much as possible. The PC32 is inspired by the
Intel 80386 and the Motorola 68000 for its architecture, and by the Amiga line of computers and
early 3D accelerators like the 3dfx Voodoo for its graphics. The goal is to create a computer
that is easy to understand and program for, and to achieve this, the ISA is kept simple while
providing all current, modern features. The PC32 has a RISC architecture, variable-length
instructions, and 32-bit instruction set, with inspiration from x86, 68000, and MIPS assembly.

## HARDWARE

- 10 - 100 MHz clock speed
- 1 Mb - 16 Mb RAM
- 16 32-bit general purpose registers
- 1 32-bit program counter
- 1 32-bit stack pointer
- 1 32-bit status register
