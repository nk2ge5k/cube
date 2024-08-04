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

#include "debug.h"

#include <sys/time.h>

i64 ustime(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    i64 ust = ((i64)tv.tv_sec)*1000000;
    ust += tv.tv_usec;

    return ust;
}

i64 mstime(void) {
    return ustime()/1000;
}

#if DEBUG

#include <stdarg.h>

#ifndef MAX_PROFILE_TIMERS
#define MAX_PROFILE_TIMERS 50
#endif

broad usize _nprofiles = 0;
broad i64 _timers[MAX_PROFILE_TIMERS];

void beginTimeProfile() { 
  _timers[_nprofiles] = mstime(); 
  _nprofiles++;
}

void endTimeProfile(const char* format, ...) {
  i64 elapsed = mstime() - _timers[_nprofiles - 1];
  _nprofiles--;

  va_list args;
  va_start(args, format);

  char textbuf[1024];
  vsnprintf(textbuf, 1024, format, args);
  va_end(args);

  fprintf(stderr, "%*s%s\t%lu\n", (int)_nprofiles, "", textbuf, elapsed);
}

#else

void beginTimeProfile() {}
void endTimeProfile(const char* format, ...) {}

#endif
