// Copyright 2024, Geogii Chernukhin <nk2ge5k@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:

// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <raylib.h>
#include <raymath.h>

#include "types.h"
#include "debug.h"

// Default window dimensions
#define DEFAULT_WIDHT  800
#define DEFALUT_HEIGHT 600

local bool isPowerOfTwo(u32 x) {
  return (x & (x - 1)) == 0;
}

local u8 lerpU8(u8 start, u8 end, f64 amount) {
  u8 result = start + CAST(u8, (amount * (end - start)));
  return result;
}

local f64 clamp(f64 val, f64 min, f64 max) {
  if (val > max) return max;
  if (val < min) return min;
  return val;
}

local Color lerpColor2(f64 amount, Color start, Color end) {
  Color result = {
    .r = lerpU8(start.r, end.r, amount),
    .g = lerpU8(start.g, end.g, amount),
    .b = lerpU8(start.g, end.b, amount),
    .a = lerpU8(start.a, end.a, amount),
  };
  return result;
}

local i32 cube(void) {

  InitWindow(DEFAULT_WIDHT, DEFALUT_HEIGHT, "3D Cube");

  // Define the camera to look into our 3d world
  Camera3D camera = { 
    .position   = { .x = 10.0f, .y = 10.0f, .z = 10.0f },
    .target     = { .x = 0.0f,  .y = 0.0f,  .z = 0.0f },
    .up         = { .x = 0.0f,  .y = 1.0f,  .z = 0.0f },
    .fovy       = 45.0f,
    .projection = CAMERA_PERSPECTIVE,
  };

  f32 exterior_cube_side = 6.0f;
  f32 gap_size           = 0.05f;
  i32 cubes_per_edge     = 10;
  f32 scale              = 1.0f;

  // Limit cursor to relative movement inside the window
  // DisableCursor();
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_O)) {
      cubes_per_edge++;
    } else if (cubes_per_edge > 1 && IsKeyPressed(KEY_L)) {
      cubes_per_edge--;
    }

    if (IsKeyDown(KEY_I)) {
      scale += 0.01;
    } else if (scale > 1 && IsKeyDown(KEY_K)) {
      scale -= 0.01;
    }

    f32 interior_cube_size = (exterior_cube_side - (gap_size * (cubes_per_edge - 1))) / cubes_per_edge;

    // TODO: would be better if camera was orbital, probably.
    UpdateCamera(&camera, CAMERA_ORBITAL);

    BeginDrawing();
    {
      ClearBackground(WHITE);

      BeginMode3D(camera);

      f32 end   = exterior_cube_side * 0.5;
      f32 start = -end;
      assertf((start + end) == 0, "Expected to center on center");

      f32 half_size = interior_cube_size * 0.5;
      Vector3 cube_size = {
        .x = interior_cube_size,
        .y = interior_cube_size,
        .z = interior_cube_size,
      };

      {
        for (f32 z = start; z <= end; z += interior_cube_size + gap_size) {
          for (f32 y = start; y <= end; y += interior_cube_size + gap_size) {
            for (f32 x = start; x <= end; x += interior_cube_size + gap_size) {

              Vector3 position = Vector3Scale(
                  Vector3AddValue((Vector3){
                    .x = x,
                    .y = y,
                    .z = z,
                  }, half_size),
                  scale
              );

              Vector3 norm = Vector3Normalize(position);
              Color color = {
                .r = lerpU8(20, 255, norm.x),
                .g = lerpU8(20, 255, norm.y),
                .b = lerpU8(20, 255, norm.z),
                .a = 0xff,
              };

              DrawCubeV(position, cube_size, color);
              // DrawCubeWiresV(position, cube_size, WHITE);
            }
          }
        }

      }
      EndMode3D();

    }
    EndDrawing();
  }

  return 0;
}

typedef struct {
  u32 stride;
  bool* v;
} Field;

local void fieldInit(Field* field, u32 stride) {
  if (stride > field->stride) {
    u32 size = stride * stride;
    field->v = (bool*)realloc(field->v, size * sizeof(bool));
  }
  field->stride = stride;
}

local void fieldFree(Field* field) {
  free(field->v);
}

local u32 fieldCellIndex(Field* field, u32 x, u32 y) {
  return field->stride * y + x;
}

local bool fieldCellIsAlive(Field* field, u32 x, u32 y) {
  u32 idx = fieldCellIndex(field, x, y);
  return field->v[idx];
}

local void fieldCellSet(Field* field, u32 x, u32 y, bool alive) {
  u32 idx = fieldCellIndex(field, x, y);
  field->v[idx] = alive;
}

local i32 gameOfLife(void) {
  InitWindow(DEFAULT_WIDHT, DEFALUT_HEIGHT, "Game of life");

  i32 size = 20;
}

i32 main(void) {
  if (true) {
    return gameOfLife();
  }
  return cube();
}
