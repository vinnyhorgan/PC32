# setup

LDI, 0, 0
LDI, 1, 0
RND, 5, 255
RND, 6, 255
RND, 7, 255

# main loop

.x
JSRA, setPixel
ADDI, 0, 1
RND, 5, 255
RND, 6, 255
RND, 7, 255
CMPI, 0, 300
BEQ, y
JMPA, x

.y
LDI, 0, 0
ADDI, 1, 1
CMPI, 1, 200
BEQ, end
JMPA, x

.end
HLT

# set pixel subroutine

.setPixel
LD, 2, 0
LD, 3, 1
MULI, 3, 300
ADD, 2, 3
MULI, 2, 3
ADDI, 2, 868576
STB, 5, 2
ADDI, 2, 1
STB, 6, 2
ADDI, 2, 1
STB, 7, 2
RET
