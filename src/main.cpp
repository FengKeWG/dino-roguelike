#include "raylib.h"

int main() {
    InitWindow(1920, 1080, "Dino Roguelike");
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Hello, World!", 400, 380, 200, BLACK);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
