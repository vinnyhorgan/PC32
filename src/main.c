#include <stdio.h>
#include <stdint.h>

#include "raylib.h"
#include "raymath.h"

#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "raylib-nuklear.h"

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))

#define MEMORY (1 << 20)
#define MAX_SPEED 50.0f
#define REFRESH_RATE 60
#define SCREEN_WIDTH 300
#define SCREEN_HEIGHT 200

enum {
    OP_NOP = 0,
    OP_ADD,
    OP_AND,
    OP_JMP,
    OP_LD,
    OP_LDI,
    OP_LDR,
};

uint8_t memory[MEMORY];
uint32_t reg[16];
uint32_t pc;
uint32_t sp;

bool running = false;
float speed = 1.0f;
int cyclesPerFrame;

int startAddress = 0;
int endAddress = 1000;

void setSpeed(float spd) {
    speed = spd;
    cyclesPerFrame = (int)(speed * 1000000 / REFRESH_RATE);
}

void reset() {
    pc = 0;
    sp = 0;

    for (int i = 0; i < 16; i++) {
        reg[i] = 0;
    }

    for (int i = 0; i < MEMORY; i++) {
        memory[i] = 0;
    }

    setSpeed(speed);

    memory[0] = OP_LD;
    memory[1] = 0;
    memory[2] = 0;
    memory[3] = 0;
    memory[4] = 0;
    memory[5] = 0x10;
    memory[6] = OP_LD;
    memory[7] = 1;
    memory[8] = 0;
    memory[9] = 0;
    memory[10] = 0;
    memory[11] = 0x10;
    memory[12] = OP_ADD;
    memory[13] = 0;
    memory[14] = 1;
}

uint8_t readByte(uint32_t address) {
    return memory[address];
}

uint16_t readWord(uint32_t address) {
    return (memory[address] << 8) | memory[address + 1];
}

uint32_t readLong(uint32_t address) {
    return (memory[address] << 24) | (memory[address + 1] << 16) | (memory[address + 2] << 8) | memory[address + 3];
}

void writeByte(uint32_t address, uint8_t value) {
    memory[address] = value;
}

void writeWord(uint32_t address, uint16_t value) {
    memory[address] = value >> 8;
    memory[address + 1] = value & 0xFF;
}

void writeLong(uint32_t address, uint32_t value) {
    memory[address] = value >> 24;
    memory[address + 1] = (value >> 16) & 0xFF;
    memory[address + 2] = (value >> 8) & 0xFF;
    memory[address + 3] = value & 0xFF;
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
    case OP_AND:
        r1 = readByte(pc);
        pc++;
        r2 = readByte(pc);
        pc++;

        reg[r1] &= reg[r2];

        cycles = 4;
        break;
    case OP_JMP:
        r1 = readByte(pc);
        pc++;

        pc = reg[r1];

        cycles = 4;
        break;
    case OP_LD:
        r1 = readByte(pc);
        pc++;

        int address = readLong(pc);
        pc += 4;

        reg[r1] = readLong(address);

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
    }

    return cycles;
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

        if (nk_begin(ctx, "CPU", nk_rect(100, 100, 250, 870),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE)) {

            nk_layout_row_dynamic(ctx, 30, 2);

            nk_label(ctx, "PC", NK_TEXT_LEFT);
            nk_label(ctx, TextFormat("%08X", pc), NK_TEXT_LEFT);

            nk_label(ctx, "SP", NK_TEXT_LEFT);
            nk_label(ctx, TextFormat("%08X", sp), NK_TEXT_LEFT);

            nk_label(ctx, "REGISTERS", NK_TEXT_LEFT);

            nk_spacing(ctx, 1);

            for (int i = 0; i < 16; i++) {
                nk_label(ctx, TextFormat("A%d", i), NK_TEXT_LEFT);
                nk_label(ctx, TextFormat("%08X", reg[i]), NK_TEXT_LEFT);
            }

            nk_label(ctx, "SPEED", NK_TEXT_LEFT);
            nk_label(ctx, TextFormat("%.2f MHz", speed), NK_TEXT_LEFT);

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

        if (nk_begin(ctx, "MEMORY", nk_rect(500, 100, 500, 500),
                NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE)) {

            nk_layout_row_dynamic(ctx, 30, 2);

            nk_property_int(ctx, "Start Address", 0, &startAddress, 0xFFFFFFFF, 1, 1);
            nk_property_int(ctx, "End Address", 0, &endAddress, 0xFFFFFFFF, 1, 1);

            nk_layout_row_dynamic(ctx, 30, 1);

            nk_label(ctx, "MEMORY", NK_TEXT_LEFT);

            nk_spacing(ctx, 1);

            for (int i = startAddress; i < endAddress; i+=16) {
                nk_layout_row_dynamic(ctx, 30, 17);

                nk_label(ctx, TextFormat("%04X: ", i), NK_TEXT_LEFT);

                for (int j = 0; j < 16; j++) {
                    if (pc == i+j) {
                        nk_label_colored(ctx, TextFormat("%02X", memory[i+j]), NK_TEXT_RIGHT, nk_rgb(255, 0, 0));
                    } else {
                        nk_label(ctx, TextFormat("%02X", memory[i+j]), NK_TEXT_RIGHT);
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

        BeginTextureMode(target);

            ClearBackground(WHITE);

        EndTextureMode();

        BeginDrawing();

            ClearBackground(BLACK);

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
