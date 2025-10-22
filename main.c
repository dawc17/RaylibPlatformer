#include "raylib.h"

int main(void) {
  InitWindow(1280, 720, "Window");

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("hallo ben du er svart", 180, 200, 60, BLACK);
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
