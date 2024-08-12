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

#include <time.h>
#include <stdlib.h>

#include <raylib.h>
#include <raymath.h>

#include "types.h"
#include "debug.h"

// Default window dimensions
#define DEFAULT_WIDHT  1000
#define DEFALUT_HEIGHT 1000

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

local i32 modi32(i32 a, i32 b) {
  if (a < 0) {
    return (b + a) % b;
  }
  return a % b;
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

local Vector2 textDrawf(f32 x, f32 y,
    Font font, i32 font_size, i32 font_spacing, Color color,
    const char *format, ...) {
  char buf[1024];

  va_list argptr;
  va_start(argptr, format);
  vsnprintf(buf, 1024, format, argptr);
  va_end(argptr);

  Vector2 size = MeasureTextEx(font, buf, font_size, font_spacing);
  Vector2 pos  = { .x = CAST(f32, x), .y = CAST(f32, y) };
  DrawTextEx(font, buf, pos, font_size, font_spacing, color);

  return size;
}

////////////////////////////////////////////////////////////////////////////////
/// Game of life
////////////////////////////////////////////////////////////////////////////////

typedef enum {
  EMPTY  = 0,
  DEAD   = 2,
  DIYING = 3,
  ALIVE  = 4,
} State;

// Field represents playing field.
typedef struct {
  // Size of the side of the field
  u32 stride;
  // Current state of the field
  u8* current;
  // Temporary array that holds state of the cells for the next game tick.
  u8* next;
} Field;

// fieldInit initializes field with given stride - field is always a square.
local void fieldInit(Field* field, u32 stride) {
  u32 size = stride * stride;
  field->current = (u8*)calloc(size, sizeof(u8));
  field->next    = (u8*)calloc(size, sizeof(u8));
  field->stride  = stride;
}

// fieldFree frees resouces allocated by the field.
local void fieldFree(Field* field) {
  free(field->current);
  free(field->next);
}

// fieldCellIndex returns index of the cell in the array.
local u32 fieldCellIndex(Field* field, i32 x, i32 y) {
  x = modi32(x, field->stride);
  y = modi32(y, field->stride);

  u32 idx = field->stride * y + x;
  u32 len = field->stride * field->stride;

  assertf(idx < len, "Index %u is out of bounds (length: %u)", idx, len);

  return idx;
}

// fieldCellSet sets cell state.
local void fieldCellSet(Field* field, i32 x, i32 y, State state) {
  u32 idx = fieldCellIndex(field, x, y);
  field->current[idx] = state;
}

// fieldCellState returns cell state
local State fieldCellState(Field* field, i32 x, i32 y) {
  u32 idx = fieldCellIndex(field, x, y);
  return field->current[idx];
}

// fieldCellIsAlive checks if the cell at given coordinates is alive.
local bool fieldCellIsAlive(Field* field, i32 x, i32 y) {
  return fieldCellState(field, x, y) == ALIVE;
}


// fieldNext returns state of the cell at the next game tick.
local State fieldNext(Field* field, i32 x, i32 y) {
  u32 alive_neighbors = 0;
  alive_neighbors += fieldCellIsAlive(field, x,     y + 1); // S
  alive_neighbors += fieldCellIsAlive(field, x - 1, y + 1); // SW
  alive_neighbors += fieldCellIsAlive(field, x - 1, y    ); // W
  alive_neighbors += fieldCellIsAlive(field, x - 1, y - 1); // NW
  alive_neighbors += fieldCellIsAlive(field, x,     y - 1); // N
  alive_neighbors += fieldCellIsAlive(field, x + 1, y - 1); // NE
  alive_neighbors += fieldCellIsAlive(field, x + 1, y    ); // E
  alive_neighbors += fieldCellIsAlive(field, x + 1, y + 1); // SE

  State state = fieldCellState(field, x, y);

	// Alive when:
	//   exactly 3 neighbors: on,
	//   exactly 2 neighbors: maintain current state,
  if (alive_neighbors == 3 || (alive_neighbors == 2 && state == ALIVE)) {
    return ALIVE;
  }

  switch (state) {
    case ALIVE:
      return DIYING;
    case DIYING:
      return DEAD;
    case DEAD:
      return DEAD;
    default:
      return EMPTY;
  }
}

// fieldUpdate updates current state of the field.
local void fieldUpdate(Field* field) {
  // @slow: I am not sure but it seems like it would be faster to work
  //  with the array directly rather then converting x and y coordinates
  //  to the index.
  //  At least iteration through the array is somewhat sequential, probably
  //  will not be predicted correctly because of fieldNext function that
  //  accessing cells out of the order.
  for (u32 y = 0; y < field->stride; y++) {
    for (u32 x = 0; x < field->stride; x++) {
      u32 index = fieldCellIndex(field, x, y);
      field->next[index] = fieldNext(field, x, y);
    }
  }

  usize size = (field->stride * field->stride) * sizeof(bool);

  // Updating current state of the field
  memcpy(field->current, field->next, size);
}

local i32 randomi32(i32 min, i32 max) {
  return rand() % (max + 1 - min) + min;
}

// Game holds data necessary for the rendering
typedef struct {
  // Field rectangle
  Rectangle rect;
  // Field
  Field field;

  // Number of entires (game generations) in the history
  u32 history_size;
  // maximum number of entries (game generations) saved in the history
  u32 max_history_size;
  // field history
  u8* history;

  bool selected;
  // selected coordinates
  i32 x;
  i32 y;

  // Pause is a flag that stops game ticks
  bool pause;
  // Number of seconds per single game tick
  f64 seconds_per_tick;
  // Time of the last tick
  f64 last_tick_at;
} Game;

// gameCreate creates new game with given field size and update speed
local Game gameCreate(Rectangle rect, u32 field_size, f64 seconds_per_tick) {
  Game game = {
    .rect             = rect,
    .pause            = true,
    .seconds_per_tick = seconds_per_tick,
    .last_tick_at     = 0,
  };
  fieldInit(&game.field, field_size);

  return game;
}

// gameClose closes the game and frees allocated resources.
local void gameClose(Game* game) {
  game->pause = true;
  fieldFree(&game->field);
}

// gameUpdate updates game state form the user inputs as well as from ticks
local void gameUpdate(Game* game) {
  // Toggle pause on space.
  if (IsKeyPressed(KEY_SPACE)) {
    game->pause = !game->pause;
  }

  f64 spt = game->seconds_per_tick;
  if (IsKeyDown(KEY_W)) {
    spt -= 0.01;
  } else if (IsKeyDown(KEY_S)) {
    spt += 0.01;
  }

  if (spt > 0) {
    game->seconds_per_tick = spt;
  }


  if (game->pause) {
    Vector2 pos = GetMousePosition();
    if (CheckCollisionPointRec(pos, game->rect)) {
      f32 cell_width  = game->rect.width  / game->field.stride;
      f32 cell_height = game->rect.height / game->field.stride;

      i32 x = (pos.x - game->rect.x) / cell_width;
      i32 y = (pos.y - game->rect.y) / cell_height;

      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        bool alive = fieldCellIsAlive(&game->field, x, y);
        fieldCellSet(&game->field, x, y, alive ? DEAD : ALIVE);
      } else {
        game->x = x;
        game->y = y;
      }

      game->selected = true;
    }
  } else {
    game->selected = false;
  }

  f64 time = GetTime();
  if (!game->pause && (time - game->last_tick_at) > game->seconds_per_tick) {
    fieldUpdate(&game->field);
    game->last_tick_at = time;
  }
}

local void gameRenderCell(Game* game, i32 x, i32 y, Color color) {
  x = modi32(x, game->field.stride);
  y = modi32(y, game->field.stride);

  f32 cell_width  = game->rect.width  / game->field.stride;
  f32 cell_height = game->rect.height / game->field.stride;

  Rectangle rect = {
    .x      = game->rect.x + (cell_width * x),
    .y      = game->rect.y + (cell_height * y),
    .width  = cell_width,
    .height = cell_height,
  };

  DrawRectangleRec(rect, color);
}

local void gameRenderCellLines(Game* game, i32 x, i32 y, f32 thick, Color color) {
  x = modi32(x, game->field.stride);
  y = modi32(y, game->field.stride);

  f32 cell_width  = game->rect.width  / game->field.stride;
  f32 cell_height = game->rect.height / game->field.stride;

  Rectangle rect = {
    .x      = game->rect.x + (cell_width * x),
    .y      = game->rect.y + (cell_height * y),
    .width  = cell_width,
    .height = cell_height,
  };

  DrawRectangleLinesEx(rect, thick, color);
}

// gameRender renders game field and updates game state if necessary
local void gameRender(Game* game) {
  if (!game->pause) return;

  for (u32 y = 0; y < game->field.stride; y++) {
    for (u32 x = 0; x < game->field.stride; x++) {
      Color color;
      switch (fieldCellState(&game->field, x, y)) {
        case EMPTY:
          color = BLANK;
          break;
        case DEAD:
          color = Fade(BLUE, 0.3);
          break;
        case DIYING:
          color = Fade(BLUE, 0.5);
          break;
        case ALIVE:
          color = Fade(GREEN, 0.5);
          break;
      }
      gameRenderCell(game, x, y, color);

      if (game->pause) {
        gameRenderCellLines(game, x, y, 0.5f, Fade(GRAY, 0.5));
      }
    }
  }

  if (game->selected) {
    i32 x = game->x;
    i32 y = game->y;
    gameRenderCell(game, x, y, GRAY);
  }
}

local void gameRender3D(Game* game, Camera camera) { 
  f32 cube_width = 6.0f;               // size of the cube side
  f32 gap_size   = 1.05f;              // size of the gap between the cubes
  i32 ncubes     = game->field.stride; // cubes per edge

  f32 interior_cube_size = cube_width / ncubes;
  f32 half_size          = interior_cube_size * 0.5;
  f32 end                = cube_width * 0.5;
  f32 start              = -end;
  i32 z                  = game->field.stride * 0.5;

  Vector3 cube_vec = {
    .x = interior_cube_size,
    .y = interior_cube_size,
    .z = interior_cube_size,
  };

  BeginMode3D(camera);
  for (u32 y = 0; y < game->field.stride; y++) {
    for (u32 x = 0; x < game->field.stride; x++) {
      State state = fieldCellState(&game->field, x, y);
      if (state == EMPTY || state == DEAD) {
        continue;
      }

      Vector3 position = {
        .x = (start + (x * interior_cube_size)),
        .y = (start + (z * interior_cube_size)),
        .z = (start + (y * interior_cube_size)),
      };

      switch (state) {
        case DIYING:
          DrawCubeV(position, cube_vec, Fade(GRAY, 0.5));
          break;
        case ALIVE:
          DrawCubeV(position, cube_vec, GRAY);
          DrawCubeWiresV(position, cube_vec, WHITE);
          break;
        default:
          break;
      }
    }
  }
  EndMode3D();
}

local i32 gameOfLife(void) {
  InitWindow(DEFAULT_WIDHT, DEFALUT_HEIGHT, "Game of life");

  Camera3D camera = { 
    .position   = { .x = 10.0f, .y = 10.0f, .z = 10.0f },
    .target     = { .x = 0.0f,  .y = 0.0f,  .z = 0.0f },
    .up         = { .x = 0.0f,  .y = 1.0f,  .z = 0.0f },
    .fovy       = 45.0f,
    .projection = CAMERA_PERSPECTIVE,
  };

  i32 width  = GetScreenWidth();
  i32 height = GetScreenHeight();
  i32 min    = (width < height) ? width : height;

  Rectangle rect = {
    .width  = min / 3.0f,
    .height = min / 3.0f,
    .x      = 10.0f,
    .y      = 10.0f,
  };

  Game game = gameCreate(rect, 20, 0.1);

  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    if (!game.pause) {
      UpdateCamera(&camera, CAMERA_FREE);
    }

    gameUpdate(&game);

    BeginDrawing();
    {
      ClearBackground(BLACK);
      gameRender3D(&game, camera);
      gameRender(&game);
    }
    EndDrawing();
  }

  gameClose(&game);
  return 0;
}

i32 main(void) {
  return gameOfLife();
}
