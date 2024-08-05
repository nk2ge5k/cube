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

#include "types.h"

// Default window dimensions
#define DEFAULT_WIDHT  800
#define DEFALUT_HEIGHT 600

i32 main(void) {

  InitWindow(DEFAULT_WIDHT, DEFALUT_HEIGHT, "3D Cube");

  // Define the camera to look into our 3d world
  Camera3D camera = { 
    .position   = { .x = 10.0f, .y = 10.0f, .z = 10.0f },
    .target     = { .x = 0.0f,  .y = 0.0f,  .z = 0.0f },
    .up         = { .x = 0.0f,  .y = 1.0f,  .z = 0.0f },
    .fovy       = 45.0f,
    .projection = CAMERA_PERSPECTIVE,
  };


  // Limit cursor to relative movement inside the window
  DisableCursor();

  // Set our game to run at 60 frames-per-second
  SetTargetFPS(60);

  f32 cube_size  = 2.0f;
  i32 cube_count = 4; // check if power of two

  Vector3 cubePosition = { 
    .x = 0.0f,
    .y = 0.0f,
    .z = 0.0f,
  };

  while (!WindowShouldClose()) {
    // TODO: would be better if camera was orbital, probably.
    UpdateCamera(&camera, CAMERA_ORBITAL);

    BeginDrawing();
    {
      ClearBackground(BLACK);

      BeginMode3D(camera);
      {
        DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, WHITE);
        DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, GRAY);
      }
      EndMode3D();

    }
    EndDrawing();
  }

  return 0;
}
