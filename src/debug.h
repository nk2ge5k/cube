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

#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "types.h"

#define __FILENAME__                                                           \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define put_prefix_(dst) fprintf(dst, "[%s:%d]: ", __FILENAME__, __LINE__)

#define STD_ERROR strerror(errno)

#define errorf(format, ...)                                                    \
  do {                                                                         \
    put_prefix_(stderr);                                                       \
    fprintf(stderr, "ERROR " format "\n", ##__VA_ARGS__);                      \
  } while (0)


#define assertf(expression, format, ...)                                       \
  do {                                                                         \
    if (!(expression)) {                                                       \
      errorf("Assertion (" #expression ") failed: " format, ##__VA_ARGS__);    \
      abort();                                                                 \
    }                                                                          \
  } while (0)

#ifndef DEBUG

#define debugf(format, ...)

#else

#include <stdlib.h>

#define debugf(format, ...)                                                    \
  do {                                                                         \
    put_prefix_(stderr);                                                       \
    fprintf(stderr, "DEBUG " format "\n", ##__VA_ARGS__);                      \
  } while (0)

#endif


#ifdef __cplusplus
extern "C" {
#endif

i64 ustime(void);
i64 mstime(void);

void beginTimeProfile();
void endTimeProfile(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif
