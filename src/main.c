#include <stdio.h>
#include <stdint.h>

#include "raylib.h"
#include "raymath.h"

#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "raylib-nuklear.h"

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))

#define MEMORY (1 << 20)
#define MAX_SPEED 20.0f

uint8_t memory[MEMORY];
uint32_t reg[16];
uint32_t pc;
uint32_t sp;

bool running = true;
float speed = 1.0f;
int cyclesPerFrame;

int startAddress = 0;
int endAddress = 1000;

void reset() {
    pc = 0;
    sp = 0;

    for (int i = 0; i < 16; i++) {
        reg[i] = 0;
    }

    for (int i = 0; i < MEMORY; i++) {
        memory[i] = 0;
    }

    cyclesPerFrame = (int)(speed * 1000000 / 60);
}

void setSpeed(float sp) {
    speed = sp;
    cyclesPerFrame = (int)(speed * 1000000 / 60);
}

int main() {
    const int screenWidth = 300;
    const int screenHeight = 200;
    const int windowScale = 2;

    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth * windowScale, screenHeight * windowScale, "PC32");
    SetWindowMinSize(screenWidth, screenHeight);
    SetTargetFPS(60);

    MaximizeWindow();

    RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

    struct nk_context *ctx = InitNuklear(10);

    reset();

    while (!WindowShouldClose()) {
        SetWindowTitle(TextFormat("PC32 - %d FPS", GetFPS()));

        UpdateNuklear(ctx);

        float scale = MIN((float)GetScreenWidth()/screenWidth, (float)GetScreenHeight()/screenHeight);

        Vector2 mouse = GetMousePosition();
        Vector2 virtualMouse = { 0 };
        virtualMouse.x = (mouse.x - (GetScreenWidth() - (screenWidth*scale))*0.5f)/scale;
        virtualMouse.y = (mouse.y - (GetScreenHeight() - (screenHeight*scale))*0.5f)/scale;
        virtualMouse = Vector2Clamp(virtualMouse, (Vector2){ 0, 0 }, (Vector2){ (float)screenWidth, (float)screenHeight });

        if (nk_begin(ctx, "CPU", nk_rect(100, 100, 250, 870),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE)) {

            nk_layout_row_dynamic(ctx, 30, 2);

            nk_label(ctx, "PC", NK_TEXT_LEFT);
            nk_label(ctx, TextFormat("%06X", pc), NK_TEXT_LEFT);

            nk_label(ctx, "SP", NK_TEXT_LEFT);
            nk_label(ctx, TextFormat("%06X", sp), NK_TEXT_LEFT);

            nk_label(ctx, "REGISTERS", NK_TEXT_LEFT);

            nk_spacing(ctx, 1);

            for (int i = 0; i < 16; i++) {
                nk_label(ctx, TextFormat("A%d", i), NK_TEXT_LEFT);
                nk_label(ctx, TextFormat("%06X", reg[i]), NK_TEXT_LEFT);
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

            nk_layout_row_dynamic(ctx, 30, 3);

            if (nk_button_label(ctx, "RESET")) {
                reset();
            }

            if (nk_button_label(ctx, "RUN")) {
                running = true;
            }

            if (nk_button_label(ctx, "STOP")) {
                running = false;
            }
        }
        nk_end(ctx);

        if (nk_begin(ctx, "MEMORY", nk_rect(500, 100, 500, 500),
                NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE)) {

            nk_layout_row_dynamic(ctx, 30, 2);

            nk_property_int(ctx, "Start Address", 0, &startAddress, 0xFFFFF, 1, 1);
            nk_property_int(ctx, "End Address", 0, &endAddress, 0xFFFFF, 1, 1);

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
                uint8_t opcode = memory[pc];
                pc++;

                if (pc >= MEMORY) {
                    pc = 0;
                }

                switch (opcode) {
                case 0x0:
                    cycles += 4;

                    break;
                }
            }
        }

        BeginTextureMode(target);

            ClearBackground(WHITE);

        EndTextureMode();

        BeginDrawing();

            ClearBackground(BLACK);

            DrawTexturePro(target.texture, (Rectangle){ 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height },
                (Rectangle){ (GetScreenWidth() - ((float)screenWidth*scale))*0.5f, (GetScreenHeight() - ((float)screenHeight*scale))*0.5f,
                (float)screenWidth*scale, (float)screenHeight*scale }, (Vector2){ 0, 0 }, 0.0f, WHITE);

            DrawNuklear(ctx);

        EndDrawing();
    }

    UnloadNuklear(ctx);

    UnloadRenderTexture(target);

    CloseWindow();

    return 0;
}
