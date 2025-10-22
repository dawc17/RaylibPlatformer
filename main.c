#include "raylib.h"
#include <stdio.h>

int main(void) {
  InitWindow(800, 450, "Window");

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Yippe", 180, 200, 20, LIGHTGRAY);
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
