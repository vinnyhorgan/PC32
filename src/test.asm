LDI, 1, 856576

.loop
INT, 0
CMPI, 0, 259
BEQ, delete
STB, 0, 1
ADDI, 1, 1
JMPA, loop

.delete
SUBI, 1, 1
STB, 9, 1
JMPA, loop
