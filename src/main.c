#include <stdio.h>
#include <stdint.h>

#include "raylib.h"
#include "raymath.h"

#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "raylib-nuklear.h"

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))

#define MEMORY (1 << 20)
#define MAX_SPEED 100.0f
#define REFRESH_RATE 60
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define VRAM MEMORY - (SCREEN_WIDTH * SCREEN_HEIGHT * 3)

typedef enum {
    VM_TEXT = 0,
    VM_BITMAP,
} VideoMode;

typedef enum {
    INT_KEYBOARD = 0,
} Interrupt;

enum {
    OP_NOP = 0,
    OP_HALT,
    OP_ADD,
    OP_ADDI,
    OP_AND,
    OP_ANDI,
    OP_BEQ,
    OP_BGE,
    OP_BGEU,
    OP_BGT,
    OP_BGTU,
    OP_BLE,
    OP_BLEU,
    OP_BLT,
    OP_BLTU,
    OP_BNE,
    OP_CMP,
    OP_CMPI,
    OP_DIV,
    OP_DIVI,
    OP_DIVU,
    OP_JMP,
    OP_JMPA,
    OP_JSR,
    OP_JSRA,
    OP_LD,
    OP_LDA,
    OP_LDI,
    OP_LDR,
    OP_MUL,
    OP_MULI,
    OP_MULU,
    OP_NEG,
    OP_NOT,
    OP_OR,
    OP_ORI,
    OP_POP,
    OP_PUSH,
    OP_RET,
    OP_STB,
    OP_STA,
    OP_SUB,
    OP_SUBI,
    OP_XOR,
    OP_XORI,
    OP_RND,
    OP_INT
};

uint8_t memory[MEMORY];
uint32_t reg[16];
uint32_t pc;
uint32_t sp;

bool zero = false;
bool carry = false;
bool overflow = false;
bool negative = false;

bool running = false;
float speed = 1.0f;
int cyclesPerFrame;

int startAddress = 0;

VideoMode videoMode = VM_TEXT;

Interrupt interrupt = -1;

uint8_t readByte(uint32_t address) {
    if (address > MEMORY) {
        printf("Invalid memory address: 0x%08X\n", address);
        exit(1);
    }

    return memory[address];
}

uint16_t readWord(uint32_t address) {
    if (address > MEMORY) {
        printf("Invalid memory address: 0x%08X\n", address);
        exit(1);
    }

    return (memory[address] << 8) | memory[address + 1];
}

uint32_t readLong(uint32_t address) {
    if (address > MEMORY) {
        printf("Invalid memory address: 0x%08X\n", address);
        exit(1);
    }

    return (memory[address] << 24) | (memory[address + 1] << 16) | (memory[address + 2] << 8) | memory[address + 3];
}

void writeByte(uint32_t address, uint8_t value) {
    if (address > MEMORY) {
        printf("Invalid memory address: 0x%08X\n", address);
        exit(1);
    }

    memory[address] = value;
}

void writeWord(uint32_t address, uint16_t value) {
    if (address > MEMORY) {
        printf("Invalid memory address: 0x%08X\n", address);
        exit(1);
    }

    memory[address] = value >> 8;
    memory[address + 1] = value & 0xFF;
}

void writeLong(uint32_t address, uint32_t value) {
    if (address > MEMORY) {
        printf("Invalid memory address: 0x%08X\n", address);
        exit(1);
    }

    memory[address] = value >> 24;
    memory[address + 1] = (value >> 16) & 0xFF;
    memory[address + 2] = (value >> 8) & 0xFF;
    memory[address + 3] = value & 0xFF;
}

void push(uint32_t value) {
    sp -= 4;
    writeLong(sp, value);
}

uint32_t pop() {
    uint32_t value = readLong(sp);
    sp += 4;
    return value;
}

void setSpeed(float spd) {
    speed = spd;
    cyclesPerFrame = (int)(speed * 1000000 / REFRESH_RATE);
}

void reset() {
    pc = 0;
    sp = VRAM - 4;

    for (int i = 0; i < 16; i++) {
        reg[i] = 0;
    }

    for (int i = 0; i < MEMORY; i++) {
        memory[i] = 0;
    }

    setSpeed(speed);

    FILE *file = fopen("out.bin", "rb");

    if (file) {
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);
        fread(memory, 1, size, file);
        fclose(file);
    }
}

void handleInterrupts() {
    switch (interrupt) {
        case INT_KEYBOARD:
            int key = GetKeyPressed();

            if (key == 0) {
                running = false;
            } else {
                reg[0] = key;
                running = true;
                interrupt = -1;
            }

            break;
    }
}

int step() {
    int cycles = 0;

    uint8_t opcode = readByte(pc);
    pc++;

    if (pc >= MEMORY) {
        pc = 0;
    }

    int r1, r2;

    switch (opcode) {
    case OP_NOP:
        cycles = 4;
        break;
    case OP_ADD:
        r1 = readByte(pc);
        pc++;
        r2 = readByte(pc);
        pc++;
        reg[r1] += reg[r2];
        cycles = 4;
        break;
    case OP_ADDI:
        r1 = readByte(pc);
        pc++;
        reg[r1] += readLong(pc);
        pc += 4;
        cycles = 4;
        break;
    case OP_AND:
        r1 = readByte(pc);
        pc++;
        r2 = readByte(pc);
        pc++;
        reg[r1] &= reg[r2];
        cycles = 4;
        break;
    case OP_ANDI:
        r1 = readByte(pc);
        pc++;
        reg[r1] &= readLong(pc);
        pc += 4;
        cycles = 4;
        break;
    case OP_BEQ:
        if (zero) {
            pc = readLong(pc);
        } else {
            pc += 4;
        }

        cycles = 4;
        break;
    case OP_BGE:
        if (negative || zero) {
            pc = readLong(pc);
        } else {
            pc += 4;
        }

        cycles = 4;
        break;
    case OP_BGEU:
        if (carry || zero) {
            pc = readLong(pc);
        } else {
            pc += 4;
        }

        cycles = 4;
        break;
    case OP_BGT:
        if (negative) {
            pc = readLong(pc);
        } else {
            pc += 4;
        }

        cycles = 4;
        break;
    case OP_BGTU:
        if (negative) {
            pc = readLong(pc);
        } else {
            pc += 4;
        }

        cycles = 4;
        break;
    case OP_BLE:
        if (!negative || zero) {
            pc = readLong(pc);
        } else {
            pc += 4;
        }

        cycles = 4;
        break;
    case OP_BLEU:
        if (!negative || zero) {
            pc = readLong(pc);
        } else {
            pc += 4;
        }

        cycles = 4;
        break;
    case OP_BLT:
        if (negative) {
            pc = readLong(pc);
        } else {
            pc += 4;
        }

        cycles = 4;
        break;
    case OP_BLTU:
        if (negative) {
            pc = readLong(pc);
        } else {
            pc += 4;
        }

        cycles = 4;
        break;
    case OP_BNE:
        if (!zero) {
            pc = readLong(pc);
        } else {
            pc += 4;
        }

        cycles = 4;
        break;
    case OP_CMP:
        r1 = readByte(pc);
        pc++;
        r2 = readByte(pc);
        pc++;
        zero = reg[r1] == reg[r2];
        carry = reg[r1] > reg[r2];
        overflow = false;
        negative = reg[r1] < reg[r2];
        cycles = 4;
        break;
    case OP_CMPI:
        r1 = readByte(pc);
        pc++;
        zero = reg[r1] == readLong(pc);
        carry = reg[r1] > readLong(pc);
        overflow = false;
        negative = reg[r1] < readLong(pc);
        pc += 4;
        cycles = 4;
        break;
    case OP_DIV:
        r1 = readByte(pc);
        pc++;
        r2 = readByte(pc);
        pc++;
        reg[r1] = (int)(reg[r1] / reg[r2]);
        reg[0] = reg[r1] % reg[r2];
        cycles = 4;
        break;
    case OP_DIVI:
        r1 = readByte(pc);
        pc++;
        reg[r1] = (int)(reg[r1] / readLong(pc));
        reg[0] = reg[r1] % readLong(pc);
        pc += 4;
        cycles = 4;
        break;
    case OP_DIVU:
        r1 = readByte(pc);
        pc++;
        r2 = readByte(pc);
        pc++;
        reg[r1] = (int)(reg[r1] / reg[r2]);
        reg[0] = reg[r1] % reg[r2];
        cycles = 4;
        break;
    case OP_JMP:
        r1 = readByte(pc);
        pc++;
        pc = readLong(reg[r1]);
        cycles = 4;
        break;
    case OP_JMPA:
        pc = readLong(pc);
        cycles = 4;
        break;
    case OP_JSR:
        r1 = readByte(pc);
        pc++;
        push(pc);
        pc = readLong(reg[r1]);
        cycles = 4;
        break;
    case OP_JSRA:
        push(pc + 4);
        pc = readLong(pc);
        cycles = 4;
        break;
    case OP_LD:
        r1 = readByte(pc);
        pc++;
        r2 = readByte(pc);
        pc++;
        reg[r1] = reg[r2];
        cycles = 4;
        break;
    case OP_LDA:
        r1 = readByte(pc);
        pc++;
        reg[r1] = readLong(readLong(pc));
        pc += 4;
        cycles = 4;
        break;
    case OP_LDI:
        r1 = readByte(pc);
        pc++;
        reg[r1] = readLong(pc);
        pc += 4;
        cycles = 4;
        break;
    case OP_LDR:
        r1 = readByte(pc);
        pc++;
        r2 = readByte(pc);
        pc++;
        reg[r1] = readLong(reg[r2]);
        cycles = 4;
        break;
    case OP_MUL:
        r1 = readByte(pc);
        pc++;
        r2 = readByte(pc);
        pc++;
        reg[r1] *= reg[r2];
        cycles = 4;
        break;
    case OP_MULI:
        r1 = readByte(pc);
        pc++;
        reg[r1] *= readLong(pc);
        pc += 4;
        cycles = 4;
        break;
    case OP_MULU:
        r1 = readByte(pc);
        pc++;
        r2 = readByte(pc);
        pc++;
        reg[r1] *= reg[r2];
        cycles = 4;
        break;
    case OP_NEG:
        r1 = readByte(pc);
        pc++;
        reg[r1] = -reg[r1];
        cycles = 4;
        break;
    case OP_NOT:
        r1 = readByte(pc);
        pc++;
        reg[r1] = ~reg[r1];
        cycles = 4;
        break;
    case OP_OR:
        r1 = readByte(pc);
        pc++;
        r2 = readByte(pc);
        pc++;
        reg[r1] |= reg[r2];
        cycles = 4;
        break;
    case OP_ORI:
        r1 = readByte(pc);
        pc++;
        reg[r1] |= readLong(pc);
        pc += 4;
        cycles = 4;
        break;
    case OP_POP:
        r1 = readByte(pc);
        pc++;
        reg[r1] = pop();
        cycles = 4;
        break;
    case OP_PUSH:
        r1 = readByte(pc);
        pc++;
        push(reg[r1]);
        cycles = 4;
        break;
    case OP_RET:
        pc = pop();
        cycles = 4;
        break;
    case OP_STB:
        r1 = readByte(pc);
        pc++;
        r2 = readByte(pc);
        pc++;
        writeByte(reg[r2], reg[r1]);
        cycles = 4;
        break;
    case OP_STA:
        r1 = readByte(pc);
        pc++;
        writeLong(readLong(pc), reg[r1]);
        pc += 4;
        cycles = 4;
        break;
    case OP_SUB:
        r1 = readByte(pc);
        pc++;
        r2 = readByte(pc);
        pc++;
        reg[r1] -= reg[r2];
        cycles = 4;
        break;
    case OP_SUBI:
        r1 = readByte(pc);
        pc++;
        reg[r1] -= readLong(pc);
        pc += 4;
        cycles = 4;
        break;
    case OP_XOR:
        r1 = readByte(pc);
        pc++;
        r2 = readByte(pc);
        pc++;
        reg[r1] ^= reg[r2];
        cycles = 4;
        break;
    case OP_XORI:
        r1 = readByte(pc);
        pc++;
        reg[r1] ^= readLong(pc);
        pc += 4;
        cycles = 4;
        break;
    case OP_HALT:
        running = false;
        cycles = cyclesPerFrame;
        break;
    case OP_RND:
        r1 = readByte(pc);
        pc++;
        r2 = readByte(pc);
        pc++;
        reg[r1] = GetRandomValue(0, r2);
        cycles = 4;
        break;
    case OP_INT:
        r1 = readByte(pc);
        pc++;

        interrupt = r1;

        cycles = cyclesPerFrame;
        break;
    default:
        printf("Unknown opcode: %02X\n", opcode);
    }

    handleInterrupts();

    return cycles;
}

void draw() {
    switch (videoMode)
    {
    case VM_BITMAP:
        int offset = 0;

        for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
            int x = i % SCREEN_WIDTH;
            int y = i / SCREEN_WIDTH;

            uint8_t r = readByte(VRAM + offset + 0);
            uint8_t g = readByte(VRAM + offset + 1);
            uint8_t b = readByte(VRAM + offset + 2);

            offset += 3;

            DrawPixel(x, y, (Color){r, g, b, 255});
        }

        break;
    case VM_TEXT:
        for (int i = 0; i < SCREEN_WIDTH / 8 * SCREEN_HEIGHT / 8; i++) {
            int x = i % (SCREEN_WIDTH / 8);
            int y = i / (SCREEN_WIDTH / 8);

            uint8_t c = readByte(VRAM + i);

            DrawText(TextFormat("%c", c), x * 8, y * 8, 8, WHITE);
        }
    }
}

int main() {
    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2, "PC32");
    SetWindowMinSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    SetTargetFPS(REFRESH_RATE);

    MaximizeWindow();

    RenderTexture2D target = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

    struct nk_context *ctx = InitNuklear(10);

    reset();

    while (!WindowShouldClose()) {
        SetWindowTitle(TextFormat("PC32 - %d FPS - %.2f MHz", GetFPS(), speed));

        UpdateNuklear(ctx);

        float scale = MIN((float)GetScreenWidth()/SCREEN_WIDTH, (float)GetScreenHeight()/SCREEN_HEIGHT);

        Vector2 mouse = GetMousePosition();
        Vector2 virtualMouse = { 0 };
        virtualMouse.x = (mouse.x - (GetScreenWidth() - (SCREEN_WIDTH*scale))*0.5f)/scale;
        virtualMouse.y = (mouse.y - (GetScreenHeight() - (SCREEN_HEIGHT*scale))*0.5f)/scale;
        virtualMouse = Vector2Clamp(virtualMouse, (Vector2){ 0, 0 }, (Vector2){ (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT });

        if (nk_begin(ctx, "CPU", nk_rect(100, 100, 250, 820),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE)) {

            nk_layout_row_dynamic(ctx, 20, 2);

            nk_label(ctx, "PC", NK_TEXT_LEFT);
            nk_label(ctx, TextFormat("%08X", pc), NK_TEXT_LEFT);

            nk_label(ctx, "SP", NK_TEXT_LEFT);
            nk_label(ctx, TextFormat("%08X", sp), NK_TEXT_LEFT);

            nk_label(ctx, "REGISTERS", NK_TEXT_LEFT);

            nk_spacing(ctx, 1);

            for (int i = 0; i < 16; i++) {
                nk_label(ctx, TextFormat("R %d", i), NK_TEXT_LEFT);
                nk_label(ctx, TextFormat("%08X", reg[i]), NK_TEXT_LEFT);
            }

            nk_label(ctx, "FLAGS", NK_TEXT_LEFT);

            nk_spacing(ctx, 1);

            nk_label(ctx, "ZERO", NK_TEXT_LEFT);
            nk_label(ctx, TextFormat("%d", zero), NK_TEXT_LEFT);

            nk_label(ctx, "CARRY", NK_TEXT_LEFT);
            nk_label(ctx, TextFormat("%d", carry), NK_TEXT_LEFT);

            nk_label(ctx, "OVERFLOW", NK_TEXT_LEFT);
            nk_label(ctx, TextFormat("%d", overflow), NK_TEXT_LEFT);

            nk_label(ctx, "NEGATIVE", NK_TEXT_LEFT);
            nk_label(ctx, TextFormat("%d", negative), NK_TEXT_LEFT);

            nk_label(ctx, "SPEED", NK_TEXT_LEFT);
            nk_label(ctx, TextFormat("%.2f MHz", speed), NK_TEXT_LEFT);
            nk_property_float(ctx, "Speed", 0.0f, &speed, MAX_SPEED, 0.0001f, 0.0001f);

            nk_layout_row_dynamic(ctx, 30, 1);

            nk_slider_float(ctx, 0.0f, &speed, MAX_SPEED, 0.0001f);
            setSpeed(speed);

            nk_layout_row_dynamic(ctx, 30, 2);

            nk_label(ctx, "CYCLES PER FRAME", NK_TEXT_LEFT);

            nk_label(ctx, TextFormat("%d", cyclesPerFrame), NK_TEXT_LEFT);

            nk_label(ctx, "CONTROLS", NK_TEXT_LEFT);

            nk_layout_row_dynamic(ctx, 30, 4);

            if (nk_button_label(ctx, "RESET")) {
                reset();
            }

            if (nk_button_label(ctx, "RUN")) {
                running = true;
            }

            if (nk_button_label(ctx, "STOP")) {
                running = false;
            }

            if (nk_button_label(ctx, "STEP")) {
                step();
            }
        }
        nk_end(ctx);

        if (nk_begin(ctx, "MEMORY", nk_rect(500, 100, 900, 500),
                NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE)) {

            nk_layout_row_dynamic(ctx, 30, 3);

            nk_property_int(ctx, "Start Address", 0, &startAddress, MEMORY - 1000, 1, 1);

            if (nk_button_label(ctx, "STACK")) {
                startAddress = sp;
            }

            if (nk_button_label(ctx, "CODE")) {
                startAddress = pc;
            }

            nk_layout_row_dynamic(ctx, 30, 1);

            nk_label(ctx, "MEMORY", NK_TEXT_LEFT);

            nk_spacing(ctx, 1);

            for (int i = startAddress; i < startAddress + 1000; i+=16) {
                nk_layout_row_dynamic(ctx, 30, 17);

                nk_label(ctx, TextFormat("%08X: ", i), NK_TEXT_LEFT);

                for (int j = 0; j < 16; j++) {
                    if (pc == i+j) {
                        nk_label_colored(ctx, TextFormat("%02X", memory[i+j]), NK_TEXT_CENTERED, nk_rgb(255, 0, 0));
                    } else if (sp == i+j) {
                        nk_label_colored(ctx, TextFormat("%02X", memory[i+j]), NK_TEXT_CENTERED, nk_rgb(0, 255, 0));
                    } else {
                        nk_label(ctx, TextFormat("%02X", memory[i+j]), NK_TEXT_CENTERED);
                    }
                }
            }
        }
        nk_end(ctx);

        if (running) {
            int cycles = 0;

            while (cycles < cyclesPerFrame) {
                cycles += step();
            }
        }

        handleInterrupts();

        BeginTextureMode(target);

            ClearBackground(BLACK);

            draw();

        EndTextureMode();

        BeginDrawing();

            ClearBackground(WHITE);

            DrawTexturePro(target.texture, (Rectangle){ 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height },
                (Rectangle){ (GetScreenWidth() - ((float)SCREEN_WIDTH*scale))*0.5f, (GetScreenHeight() - ((float)SCREEN_HEIGHT*scale))*0.5f,
                (float)SCREEN_WIDTH*scale, (float)SCREEN_HEIGHT*scale }, (Vector2){ 0, 0 }, 0.0f, WHITE);

            DrawNuklear(ctx);

        EndDrawing();
    }

    UnloadNuklear(ctx);

    UnloadRenderTexture(target);

    CloseWindow();

    return 0;
}
