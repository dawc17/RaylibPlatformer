#include "../include/vec.h"
#include "raylib.h"
#include "rlgl.h"
#include <stddef.h>
#include <stdio.h>

// clang-format off
const int level[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 0, 0, 0,
  0, 0, 1, 1, 0, 1, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1
};
//clang-format on



enum SpriteDirection { Left = -1, Right = 1 };

typedef struct Sprite {
  Texture2D texture;
  Rectangle dest_rect;
  Vector2 velocity;
  enum SpriteDirection dir;
} Sprite;

void move_player(Sprite *player) {
  float move_speed = 100.0f;

  player->velocity.x = 0;

  if (IsKeyDown(KEY_D)) {
    player->velocity.x = move_speed;
    player->dir = Left;
  }
  if (IsKeyDown(KEY_A)) {
    player->velocity.x = -move_speed;
    player->dir = Right;
  }
  if (IsKeyPressed(KEY_SPACE)) {
    player->velocity.y = -1000.0;
  }
}

void apply_gravity(Sprite *sprite) {
  float gravity = 50.0f;

  sprite->velocity.y += gravity;
  if (sprite->velocity.y > 600.0) {
    sprite->velocity.y = 600.0;
  }
}

void apply_velocity_y(Sprite *sprite) {
  sprite->dest_rect.y += sprite->velocity.y * GetFrameTime();
}

void apply_velocity_x(Sprite *sprite) {
  sprite->dest_rect.x += sprite->velocity.x * GetFrameTime();
}

void check_collisions_y(Sprite *sprite, Sprite* tiles) {
  size_t num_tiles = vector_size(tiles);
  for (size_t i = 0; i < num_tiles; i++){
    Sprite* tile = &tiles[i];
    if (CheckCollisionRecs(sprite->dest_rect, tile->dest_rect)) {
      if (sprite->dest_rect.y > tile->dest_rect.y) {
        sprite->dest_rect.y = tile->dest_rect.y + tile->dest_rect.height;
      } else {
        sprite->dest_rect.y = tile->dest_rect.y - sprite->dest_rect.height;
      }
    }
  }
}

void check_collisions_x(Sprite *sprite, Sprite* tiles) {
  size_t num_tiles = vector_size(tiles);
  for (size_t i = 0; i < num_tiles; i++){
    Sprite* tile = &tiles[i];
    if (CheckCollisionRecs(sprite->dest_rect, tile->dest_rect)) {
      if (sprite->dest_rect.x > tile->dest_rect.x) {
        sprite->dest_rect.x = tile->dest_rect.x + tile->dest_rect.width;
      } else {
        sprite->dest_rect.x = tile->dest_rect.x - sprite->dest_rect.width;
      }
    }
  }
}

Sprite* load_level(Texture2D temp_texture) {
  const int level_width = 9;
  const int level_height = 5;
  Sprite* sprite_vec = vector_create();

  for (size_t i = 0; i < level_height * level_width; i++) {
    size_t x = i % level_width;
    size_t y = i / level_width;

    // if number > 0, then it is a sprite
    if (level[i] > 0) {
      Sprite s = {
        .texture = temp_texture,
        .dest_rect = (Rectangle){
          .x = x * 32.0f,
          .y = y * 32.0f,
          .width = 32.0f,
          .height = 32.0f
        }};
      vector_add(&sprite_vec, s);
      }
    }
  return sprite_vec;
  }

int main(void) {
  const int SCREEN_WIDTH = 600;
  const int SCREEN_HEIGHT = 400;
  const char *TITLE = "Raylib Program";
  int fps = 165;

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TITLE);

  Texture2D player_sheet = LoadTexture("resources/Player.png");

  Sprite player =
      (Sprite){.texture = player_sheet,
               .dir = Right,
               .dest_rect = (Rectangle){
                   .x = 10.0, .y = -200.0, .width = 20.0, .height = 28.0}};

  Sprite* level_tiles = load_level(player_sheet);

  SetTargetFPS(fps);

  while (!WindowShouldClose()) {
    // update
    move_player(&player);
    apply_gravity(&player);

    // after all movement update
    apply_velocity_y(&player);
    check_collisions_y(&player, level_tiles);
    apply_velocity_x(&player);
    check_collisions_x(&player, level_tiles);

    // keep from falling
    if (player.dest_rect.y > GetScreenHeight() - player.dest_rect.height) {
      player.dest_rect.y = GetScreenHeight() - player.dest_rect.height;
    }

    // draw
    BeginDrawing();
    ClearBackground(RAYWHITE);
    size_t n = vector_size(level_tiles);
    for (size_t i = 0; i < n; i++) {
      Sprite* tile = &level_tiles[i];
      DrawTexturePro(tile->texture, (Rectangle){0, 0, 32, 32},
                   tile->dest_rect, (Vector2){0, 0}, 0.0, SKYBLUE);
    }

    // visual size of sprite frame
    const float sprite_frame_width = 32.0f;
    const float sprite_frame_height = 32.0f;

    // horizontal offset to center the sprite
    // (the difference between the sprite (32) and hitbox (20)
    // and divides it by 2 to get the padding on each side)
    float offset_x = (sprite_frame_width - player.dest_rect.width) / 2.0f;

    // create the new rectangle for *drawing*
    // relative to the hitbox (player.dest_rect)
    Rectangle player_draw_rect = {
        .x = player.dest_rect.x - offset_x, // position drawing box left of hitbox
        .y = player.dest_rect.y,            // align top of drawing box with top of hitbox
        .width = sprite_frame_width,
        .height = sprite_frame_height
    };

    DrawTexturePro(player.texture, (Rectangle){0, 0, 32 * player.dir, 32},
                   player_draw_rect, (Vector2){0, 0}, 0.0, SKYBLUE);
    EndDrawing();
  }
  UnloadTexture(player_sheet);
  CloseWindow();

  return 0;
}
