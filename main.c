#include "raylib.h"
#include <math.h>
#include "rlgl.h"

int main(void)
{
  const int SCREEN_WIDTH = 800;
  const int SCREEN_HEIGHT = 450;
  const char *TITLE = "Raylib Program";
  int FPS = 60;

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TITLE);

  float rotation = 0.0f;

  SetTargetFPS(FPS);

  while (!WindowShouldClose())
  {
    rotation += 0.2f;
    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawRectangleGradientH(SCREEN_WIDTH / 4 * 2 - 90, 170, 180, 130, MAROON, GOLD);

    EndDrawing();
  }
  CloseWindow();

  return 0;
}
