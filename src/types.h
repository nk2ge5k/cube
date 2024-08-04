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

#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>

// Keywords that allow for the simple separation of the static
#define local static
#define broad static

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#define Fi32 "%" PRId32 ""
#define Fi64 "%" PRId64 ""

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef size_t usize;
typedef ssize_t ssize;

#define Fu32 "%" PRIu32 ""
#define Fu64 "%" PRIu64 ""

typedef float f32;
typedef double f64;

#define Ff32 "%f"
#define Ff64 "%lf"

#define FWD_STRUCT(name) typedef struct name name

#if defined(__GNUC__) || defined(__clang__)
# define UNUSED(name) _unused_ ## name __attribute__((unused))
#else
# define UNUSED(name) _unused_ ## name
#endif


#ifdef PARANOIA

void* _malloc(usize size, const char* filepath, i32 linenum);
void* _calloc(usize nmemb, usize size, const char* filepath, i32 linenum);
void* _realloc(void* ptr, usize size, const char* filepath, i32 linenum);
void _free(void* ptr, const char* filepath, i32 linenum);

#ifndef gmalloc
# define gmalloc(size) _malloc(size, __FILE__, __LINE__)
#endif

#ifndef gcalloc
# define gcalloc(nmemb, size) _calloc(nmemb, size, __FILE__, __LINE__)
#endif

#ifndef grealloc
# define grealloc(ptr, size) _realloc(ptr, size, __FILE__, __LINE__)
#endif

#ifndef gfree
# define gfree(ptr) _free(ptr, __FILE__, __LINE__)
#endif

#else

void* _malloc(usize size);
void* _calloc(usize nmemb, usize size);
void* _realloc(void* ptr, usize size);
void _free(void* ptr);

#ifndef gmalloc
# define gmalloc _malloc
#endif

#ifndef gcalloc
# define gcalloc _calloc
#endif

#ifndef grealloc
# define grealloc _realloc
#endif

#ifndef gfree
# define gfree _free
#endif

#endif

#ifdef __cplusplus
#define CAST(type, v) static_cast<type>(v)
#else
#define CAST(type, v) (type)(v)
#endif


// max_value returns maximum between two values.
// NOTE(nk2ge5k): use with caution - this macro must not be used with complex
// expressions as values, because it will force value recalculation.
#define max_value(a, b) (((a) > (b)) ? (a) : (b))

// min_value returns maximum between two values.
// NOTE(nk2ge5k): use with caution - this macro must not be used with complex
// expressions as values, because it will force value recalculation.
#define min_value(a, b) (((a) < (b)) ? (a) : (b))

// square returns square of the value.
// NOTE(nk2ge5k): use with caution - this macro must not be used with complex
// expressions as values, because it will force value recalculation.
#define square(a) ((a) * (a))

////////////////////////////////////////////////////////////////////////////////
/// Dynamic array
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
    #define cast_ptr(ptr) (decltype(ptr))
#else
    #define cast_ptr(...)
#endif

#define da_define(name, T) typedef struct { \
  u32 len;                                  \
  u32 cap;                                  \
  T*  arr;                                  \
} name

// Clears dynamic array
#define da_clear(da) (da)->len = 0;

// da_zero zerosout full length of the dynamic array
#define da_zero(da) memset((da)->arr, 0, (da)->len * sizeof(*(da)->arr))

// da_resize changes length of the dynamic array
#define da_resize(da, newlen) do {                                                      \
  if ((da)->cap < (newlen)) {                                                           \
    (da)->cap = (newlen);                                                               \
    (da)->arr = cast_ptr((da)->arr)grealloc((da)->arr, (da)->cap * sizeof(*(da)->arr)); \
  }                                                                                     \
  (da)->len = newlen;                                                                   \
} while (0)

// da_reserve changes capacity of the array without chaning its length
// NOTE: if new capacity is less then old capacity nothing will happen
#define da_reserve(da, newcap) do {                                                    \
  if ((da)->cap < (newcap)) {                                                          \
    (da)->cap = (newcap);                                                              \
    (da)->arr = cast_ptr((da)->arr)realloc((da)->arr, (da)->cap * sizeof(*(da)->arr)); \
  }                                                                                    \
} while (0)

// da_append ppends value to the dynamic array
#define da_append(da, val) do {                                                         \
  if ((da)->len >= (da)->cap) {                                                         \
    (da)->cap = (da)->cap == 0 ? 256 : (da)->cap * 2;                                   \
    (da)->arr = cast_ptr((da)->arr)grealloc((da)->arr, (da)->cap * sizeof(*(da)->arr)); \
  }                                                                                     \
  (da)->arr[(da)->len++] = (val);                                                       \
} while (0)

// da_append_array copies data form the val to the dynmaic array
#define da_append_array(da, val, size) do {            \
  da_resize(da, (da)->len + size);                     \
  memcpy((da)->arr, (val), sizeof(*(da)->arr) * size); \
} while (0)

// da_copy copies data from src dynamic array to the dst dynamic array
#define da_copy(dst, src) do {                  \
  da_clear(dst);                                \
  da_append_array(dst, (src)->arr, (src)->len); \
} while (0)


// da_free frees memory allocated by the array
#define da_free(da) do {   \
  if ((da)->arr != NULL) { \
    gfree((da)->arr);      \
    (da)->len = 0;         \
    (da)->cap = 0;         \
  }                        \
} while (0)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This is just a random utils that i do not know where to put, so for now
 * they are just here
 */


bool f64eq(f64 a, f64 b);

#define DECL_SWAP_INT(T) \
  inline void swap##T(T* a, T* b) { \
     *a ^= *b; \
     *b ^= *a; \
     *a ^= *b; \
  }

DECL_SWAP_INT(i8)
DECL_SWAP_INT(i16)
DECL_SWAP_INT(i32)
DECL_SWAP_INT(i64)
DECL_SWAP_INT(u8)
DECL_SWAP_INT(u16)
DECL_SWAP_INT(u32)
DECL_SWAP_INT(u64)

#undef DECL_SWAP_INT

#ifdef __cplusplus
}
#endif

#endif
