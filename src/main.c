#include "../include/vec.c"
#include "../include/vec.h"
#include "raylib.h"
#include "rlgl.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define RAYGUI_IMPLEMENTATION
#include "../include/raygui/src/raygui.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION 100
#endif

#define MAX_POSTPRO_SHADERS 13

typedef enum {
  FX_GRAYSCALE = 0,
  FX_POSTERIZATION,
  FX_DREAM_VISION,
  FX_PIXELIZER,
  FX_CROSS_HATCHING,
  FX_CROSS_STITCHING,
  FX_PREDATOR_VIEW,
  FX_SCANLINES,
  FX_FISHEYE,
  FX_SOBEL,
  FX_BLOOM,
  FX_BLUR,
  NONE
} PostproShader;

static const char *postproShaderText[] = {
    "GRAYSCALE",     "POSTERIZATION",  "DREAM_VISION",
    "PIXELIZER",     "CROSS_HATCHING", "CROSS_STITCHING",
    "PREDATOR_VIEW", "SCANLINES",      "FISHEYE",
    "SOBEL",         "BLOOM",          "BLUR",
    "NONE"};

// clang-format off
const int level[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
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

typedef enum AnimationType {
  REPEATING = 1,
  ONESHOT = 2,
} AnimationType;

typedef struct Animation {
  int first;
  int last;
  int current_frame;

  float speed;
  float duration_left;

  AnimationType type;
} Animation;

void animation_update(Animation* self) {
  float delta = GetFrameTime();
  self->duration_left -= delta;

  if (self->duration_left <= 0.0) {
    self->duration_left = self->speed;
    self->current_frame++;

    if (self->current_frame > self->last) {
      switch (self->type) {
      case REPEATING:
        self->current_frame = self->first;
        break;
      case ONESHOT:
        self->current_frame = self->last;
        break;
      }
    }
  }
}

Rectangle animation_frame(Animation* self, int num_frames_per_row) {
  int x = (self->current_frame % num_frames_per_row) * 32.0;
  int y = (self->current_frame / num_frames_per_row) * 32.0;

  return (Rectangle) {
    .x = (float)x,
    .y = (float)y,
    .width = 32.0,
    .height = 32.0
  };
}

bool isIdle = true;

void move_player(Sprite *player) {
  float move_speed = 250.0f;

  player->velocity.x = 0;

  if (IsKeyDown(KEY_D)) {
    player->velocity.x = move_speed;
    player->dir = Right;
    isIdle = false;
  }
  if (IsKeyDown(KEY_A)) {
    player->velocity.x = -move_speed;
    player->dir = Left;
    isIdle = false;
  }
  if (IsKeyPressed(KEY_SPACE)) {
    player->velocity.y = -1300.0;
  }

  if (player->velocity.x == 0) {
    isIdle = true;
  }
}

void apply_gravity(Sprite *sprite) {
  float gravity = 50.0f;

  sprite->velocity.y += gravity;
  if (sprite->velocity.y > 600.0) {
    sprite->velocity.y = 750.0;
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
  const int level_height = 11;
  Sprite* sprite_vec = vector_create();

  for (size_t i = 0; i < level_height * level_width; i++) {
    size_t x = i % level_width;
    size_t y = i / level_width;

    // if number > 0, then it is a sprite
    if (level[i] > 0) { 
      Sprite s = {
        .texture = temp_texture,
        .dest_rect = (Rectangle){
          .x = x * 64.0f,
          .y = y * 64.0f,
          .width = 64.0f,
          .height = 64.0f
        }};
      vector_add(&sprite_vec, s);
      }
    }
  return sprite_vec;
  }

int main(void) {
  const int SCREEN_WIDTH = 1280;
  const int SCREEN_HEIGHT = 720;
  const char *TITLE = "Raylib Program";
  int fps = 165;

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TITLE);

  Texture2D player_idle_stripe = LoadTexture("resources/player_idle_stripe.png");
  Texture2D player_walk_stripe = LoadTexture("resources/player_walk_stripe.png");
  Texture2D tile_texture = LoadTexture("resources/ground.png");

  Animation idle_anim = (Animation) {
    .first = 0,
    .last = 5,
    .current_frame = 0,
    .speed = 0.1,
    .duration_left = 0.1,
    .type = REPEATING  
  };

  Animation walk_anim = (Animation) {
    .first = 0,
    .last = 5,
    .current_frame = 0,
    .speed = 0.1,
    .duration_left = 0.1,
    .type = REPEATING
  };

  Animation jump_anim = (Animation) {

  };

  Shader shaders[MAX_POSTPRO_SHADERS] = { 0 };

  shaders[FX_GRAYSCALE] = LoadShader(0, TextFormat("resources/shaders/grayscale.fs"));
  shaders[FX_POSTERIZATION] = LoadShader(0, TextFormat("resources/shaders/posterization.fs"));
  shaders[FX_DREAM_VISION] = LoadShader(0, TextFormat("resources/shaders/dream_vision.fs"));
  shaders[FX_PIXELIZER] = LoadShader(0, TextFormat("resources/shaders/pixelizer.fs"));
  shaders[FX_CROSS_HATCHING] = LoadShader(0, TextFormat("resources/shaders/cross_hatching.fs"));
  shaders[FX_CROSS_STITCHING] = LoadShader(0, TextFormat("resources/shaders/cross_stitching.fs"));
  shaders[FX_PREDATOR_VIEW] = LoadShader(0, TextFormat("resources/shaders/predator.fs"));
  shaders[FX_SCANLINES] = LoadShader(0, TextFormat("resources/shaders/scanlines.fs"));
  shaders[FX_FISHEYE] = LoadShader(0, TextFormat("resources/shaders/fisheye.fs"));
  shaders[FX_SOBEL] = LoadShader(0, TextFormat("resources/shaders/sobel.fs"));
  shaders[FX_BLOOM] = LoadShader(0, TextFormat("resources/shaders/bloom.fs"));
  shaders[FX_BLUR] = LoadShader(0, TextFormat("resources/shaders/blur.fs"));

  int currentShader = NONE;
  RenderTexture2D target = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);

  Sprite player = 
      (Sprite){.texture = player_idle_stripe,
               .dir = Right,
               .dest_rect = (Rectangle){
                   .x = 10.0, .y = -200.0, .width = 50.0, .height = 90.5}};

  Sprite* level_tiles = load_level(tile_texture);

  SetTargetFPS(fps);

  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_RIGHT)) currentShader++;
    else if (IsKeyPressed(KEY_LEFT)) currentShader--;

    if (currentShader >= MAX_POSTPRO_SHADERS) currentShader = 0;
    else if (currentShader < 0) currentShader = MAX_POSTPRO_SHADERS - 1;

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

    BeginTextureMode(target);
    ClearBackground(RAYWHITE);
    size_t n = vector_size(level_tiles);
    for (size_t i = 0; i < n; i++) {
      Sprite* tile = &level_tiles[i];
      DrawTexturePro(tile->texture, (Rectangle){0, 0, 128, 128},
                   tile->dest_rect, (Vector2){0, 0}, 0.0, SKYBLUE);
    }
    const float sprite_frame_width = 128.0f;
    const float sprite_frame_height = 128.0f;

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
    if (isIdle == true){
      Rectangle source_frame = animation_frame(&idle_anim, 6);

      source_frame.width *= player.dir;
      DrawTexturePro(player_idle_stripe, source_frame, 
                  player_draw_rect, (Vector2){0, 0}, 0.0, WHITE);

    } else {
      Rectangle source_frame = animation_frame(&walk_anim, 6);

      source_frame.width *= player.dir;
      DrawTexturePro(player_walk_stripe, source_frame, 
                   player_draw_rect, (Vector2){0, 0}, 0.0, WHITE);
    }
    EndTextureMode();

    if (isIdle == true) {
      animation_update(&idle_anim);
    }
    else {
      animation_update(&walk_anim);
    }

    // draw
    BeginDrawing();
    ClearBackground(RAYWHITE);

    if (currentShader != NONE){
      BeginShaderMode(shaders[currentShader]);
      DrawTextureRec(target.texture, (Rectangle){0,0,(float)target.texture.width, (float)-target.texture.height}, (Vector2){0,0}, WHITE);
      EndShaderMode();
    } else {
      DrawTextureRec(target.texture, (Rectangle){0,0,(float)target.texture.width, (float)-target.texture.height}, (Vector2){0,0}, WHITE);
    }

    DrawRectangle(0, 9, 580, 30, Fade(LIGHTGRAY, 0.7f));
    DrawText("CURRENT SHADER:", 10, 15, 20, BLACK);
    DrawText(postproShaderText[currentShader], 330, 15, 20, RED);
    DrawText("< >", 540, 10, 30, DARKBLUE);
    DrawFPS(700, 15);
    if (GuiButton((Rectangle){200,200,300,100}, "Cycle shader")) {
      currentShader++;
      printf("Current shader: %s\n", postproShaderText[currentShader]);
    }

    EndDrawing();
  }
  for (int i = 0; i < MAX_POSTPRO_SHADERS; i++) UnloadShader(shaders[i]);
  UnloadRenderTexture(target);
  UnloadTexture(player_idle_stripe);
  UnloadTexture(player_walk_stripe);
  UnloadTexture(tile_texture);
  CloseWindow();

  return 0;
}
