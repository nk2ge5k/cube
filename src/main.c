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

local u32 cellIndex(u32 stride, i32 x, i32 y) {
  x = modi32(x, stride);
  y = modi32(y, stride);

  u32 idx = stride * y + x;

  return idx;
}

// fieldCellIndex returns index of the cell in the array.
local u32 fieldCellIndex(Field* field, i32 x, i32 y) {
  u32 idx = cellIndex(field->stride, x, y);
  u32 len = field->stride * field->stride;

  assertf(idx < len, "Index %u is out of bounds (length: %u)", idx, len);

  return idx;
}

// fieldCellSet sets cell state.
local void fieldCellSet(Field* field, i32 x, i32 y, State state) {
  u32 idx = fieldCellIndex(field, x, y);
  field->current[idx] = state;
}

local State cellState(u8* cells, u32 stride, i32 x, i32 y) {
  u32 idx = cellIndex(stride, x, y);
  return cells[idx];
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

  usize size = (field->stride * field->stride) * sizeof(*field->current);

  // Updating current state of the field
  memcpy(field->current, field->next, size);
}

local i32 randomi32(i32 min, i32 max) {
  return rand() % (max + 1 - min) + min;
}

typedef struct {
  // Constant size of the history (maximal difference between start and the end)
  u32 size;
  // Constant size of element
  u32 elem_size;

  // Start of the history (oldest state)
  u32 start;
  // End of the history (latest state)
  u32 end;

  // History data (mem = size * strid * sizeof(u8))
  u8* data;
} History;

// historyCreate initializes history with given size.
local void historyCreate(History* history, u32 elem_size, u32 size) {
  // Constants
  history->size      = size;
  history->elem_size = elem_size;
  // Cursors
  history->start     = 0;
  history->end       = 0;
  // Data
  history->data      = gmalloc(elem_size * size * sizeof(*history->data));
}

local usize historyDataIndex(History* history, u32 index) {
  // @question: Is it make sense to fix index here (index % history->size)?
  //  I am afraid that this may complecate wraping, but may be it is opposite?
  return index % history->size;
}


local void historyPush(History* history, Field field) {
  u32 field_size = field.stride * field.stride;
  assertf(history->elem_size == field_size,
      "History stride does not match field stride %u != %u",
      history->elem_size, field_size);

  u32 write = historyDataIndex(history, history->end);
  memcpy(history->data + (write * history->elem_size), field.current, history->elem_size);

  history->end++;

  // Calculate new start for the history
  u32 diff = history->end - history->start;
  if (diff > history->size) {
    history->start += (diff - history->size);
  }
}

// historyGetItem copies i-th item from the history to dst array,
// returns true if item copied successfully, or false if item with index i
// does not exists.
local bool historyGetItem(History* history, Field* field, u32 i) {
  u32 len = history->end - history->start;
  if (i >= len) {
    return false;
  }

  usize data_index = historyDataIndex(history, history->start + i);
  u8* ptr          = history->data + (data_index * history->elem_size);
  field->current   = ptr;
  field->next      = ptr; // just in case

  return true;
}

// historySize returns size of the history
local usize historySize(History* history) {
  return history->end - history->start;
}

// historyFree frees resources allocated by the history
local void historyFree(History* history) {
  gfree(history->data);
}

// Game holds data necessary for the rendering
typedef struct {
  // Field rectangle
  Rectangle rect;
  // Field
  Field field;
  // Field history
  History history;

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
  historyCreate(&game.history, field_size * field_size, 200);

  return game;
}

// gameClose closes the game and frees allocated resources.
local void gameClose(Game* game) {
  game->pause = true;
  fieldFree(&game->field);
  historyFree(&game->history);
}

// gameSaveHistoryState saves current state the history
local void gameSaveHistoryState(Game* game) {
  historyPush(&game->history, game->field);
}

// gameUpdate updates game state form the user inputs as well as from ticks
local void gameUpdate(Game* game) {
  // Toggle pause on space.
  if (IsKeyPressed(KEY_SPACE)) {
    game->pause = !game->pause;
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
  bool should_update = (game->pause && IsKeyPressed(KEY_ENTER)) ||
    (!game->pause && (time - game->last_tick_at) > game->seconds_per_tick);


  if (should_update) {
    gameSaveHistoryState(game);
    fieldUpdate(&game->field);
    game->last_tick_at = time;
  }
}

local Color state2DColor(State state) {
  switch (state) {
    case EMPTY:
      return BLANK;
      break;
    case DEAD:
      return Fade(BLUE, 0.3);
      break;
    case DIYING:
      return Fade(BLUE, 0.5);
      break;
    case ALIVE:
      return Fade(GREEN, 0.5);
      break;
  }
}

local void renderCell2D(Rectangle area, u32 stride, i32 x, i32 y, Color color) {
  x = modi32(x, stride);
  y = modi32(y, stride);

  f32 cell_width  = area.width  / stride;
  f32 cell_height = area.height / stride;

  Rectangle rect = {
    .x      = area.x + (cell_width * x),
    .y      = area.y + (cell_height * y),
    .width  = cell_width,
    .height = cell_height,
  };

  DrawRectangleRec(rect, color);
}


local void renderCellLines2D(Rectangle area, u32 stride, i32 x, i32 y, f32 thick, Color color) {
  x = modi32(x, stride);
  y = modi32(y, stride);

  f32 cell_width  = area.width  / stride;
  f32 cell_height = area.height / stride;

  Rectangle rect = {
    .x      = area.x + (cell_width * x),
    .y      = area.y + (cell_height * y),
    .width  = cell_width,
    .height = cell_height,
  };

  DrawRectangleLinesEx(rect, thick, color);
}

// gameRender2D renders game field and updates game state if necessary
local void gameRender2D(Game* game) {
  if (!game->pause) return;

  for (u32 y = 0; y < game->field.stride; y++) {
    for (u32 x = 0; x < game->field.stride; x++) {
      State state = fieldCellState(&game->field, x, y);
      Color color = state2DColor(state);

      renderCell2D(game->rect, game->field.stride, x, y, color);
      if (game->pause) {
        renderCellLines2D(game->rect, game->field.stride, x, y, 0.5f, Fade(GRAY, 0.5));
      }
    }
  }

  if (game->selected) {
    i32 x = game->x;
    i32 y = game->y;
    renderCell2D(game->rect, game->field.stride, x, y, GRAY);
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
  f32 y_start            = -(cube_width);

  Vector3 cube_vec = {
    .x = interior_cube_size,
    .y = interior_cube_size,
    .z = interior_cube_size,
  };

  BeginMode3D(camera);
  for (u32 z = 0; z < game->field.stride; z++) {
    for (u32 x = 0; x < game->field.stride; x++) {
      State state = fieldCellState(&game->field, x, z);
      if (state == EMPTY || state == DEAD) {
        continue;
      }

      Vector3 position = {
        .x = (start + (x * interior_cube_size)),
        .y = (y_start + (0 * interior_cube_size)),
        .z = (start + (z * interior_cube_size)),
      };


      if (state == ALIVE) {

        Vector3 norm = Vector3Normalize(position);

        Color color = {
          .b = 55 + (200 * norm.x),
          .g = 55 + (200 * norm.y),
          .r = 0xff,
          .a = 0xff,
        };

        DrawCubeV(position, cube_vec, color);
        DrawCubeWiresV(position, cube_vec, GRAY);
      } else {
        DrawCubeV(position, cube_vec, Fade(GRAY, 0.5));
      }
    }
  }

  Field history_item = { .stride = game->field.stride };

  i32 history_size = historySize(&game->history);
  for (i32 i = (history_size - 1); i >= 0; i--) {
    assertf(historyGetItem(&game->history, &history_item, i),
        "missing expected history item %d", i);

    for (u32 z = 0; z < history_item.stride; z++) {
      for (u32 x = 0; x < history_item.stride; x++) {
        State state = fieldCellState(&history_item, x, z);
        if (state != ALIVE) {
          continue;
        }

        i32 y = history_size - i;

        Vector3 position = {
          .x = (start + (x * interior_cube_size)),
          .y = (y_start + (y * interior_cube_size)),
          .z = (start + (z * interior_cube_size)),
        };

        Vector3 norm = Vector3Normalize(position);

        Color color = {
          .b = 55 + (200 * norm.x),
          .g = 55 + (200 * norm.y),
          .r = 55 + (200 * ((f32)i / (f32)history_size)),
          .a = 0xff,
        };

        DrawCubeV(position, cube_vec, color);
        DrawCubeWiresV(position, cube_vec, GRAY);
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

  Game game = gameCreate(rect, 40, 0.1);

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
      gameRender2D(&game);
    }
    EndDrawing();
  }

  gameClose(&game);
  return 0;
}

i32 main(void) {
  return gameOfLife();
}
