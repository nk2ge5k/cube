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

#include "types.h"

#include <math.h>
#include <stdlib.h>

#define EPSILON 0.000001f

bool f64eq(f64 a, f64 b) { 
  int result = (fabs(a - b)) <= (EPSILON*fmax(1.0f, fmax(fabs(a), fabs(b))));
  return result;
}

#ifdef PARANOIA

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define FILENAME(filepath) (strrchr(filepath, '/') ? strrchr(filepath, '/') + 1 : filepath)

void* _malloc(usize size, const char* filepath, int linenum) {
#ifdef LOG_ALLOC
  fprintf(stderr, "[%s:%d]: gmalloc(%lu)\n", FILENAME(filepath), linenum, size);
#endif
  if (size <= 0) {
    fprintf(stderr, "%s:%d: malloc size %lu < 0\n", FILENAME(filepath), linenum, size);
  }
  return malloc(size);
}

void* _calloc(usize nmemb, usize size, const char* filepath, i32 linenum) {
#ifdef LOG_ALLOC
  fprintf(stderr, "[%s:%d]: gcalloc(%lu, %lu)\n", FILENAME(filepath), linenum, nmemb, size);
#endif
  if (nmemb <= 0 || size <= 0) {
    fprintf(stderr, "%s:%d: calloc nmemb %lu < 0 || size %lu < 0\n", 
        FILENAME(filepath), linenum, nmemb, size);
  }
  return calloc(nmemb, size);
}

void* _realloc(void* ptr, usize size, const char* filepath, int linenum) {
#ifdef LOG_ALLOC
  fprintf(stderr, "[%s:%d] grealloc(%p, %lu)\n", FILENAME(filepath), linenum, ptr, size);
#endif
  if (size <= 0) {
    fprintf(stderr, "%s:%d: realloc size %lu < 0\n", FILENAME(filepath), linenum, size);
  }
  return realloc(ptr, size);
}

void _free(void* ptr, const char* filepath, int linenum) {
#ifdef LOG_ALLOC
  fprintf(stderr, "[%s:%d] gfree(%p)\n", FILENAME(filepath), linenum, ptr);
#endif
  if (ptr == NULL) {
    fprintf(stderr, "%s:%d: freeing NULL\n", FILENAME(filepath), linenum);
    return;
  }
  free(ptr);
}

#else

#include "debug.h"

void* _malloc(usize size) {
  assertf(size > 0, "malloc size %lu <= 0", size);
  return malloc(size);
}

void* _calloc(usize nmemb, usize size) {
  assertf(nmemb > 0 && size > 0, "calloc nmemb %lu <= 0 size %lu <= 0", nmemb, size);
  return calloc(nmemb, size);
}

void* _realloc(void* ptr, usize size) {
  assertf(size > 0, "realloc size %lu <= 0", size);
  return realloc(ptr, size);
}

void _free(void* ptr) {
  assertf(ptr != NULL, "freeing NULL");
  free(ptr);
}

#endif
