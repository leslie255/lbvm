#pragma once

#include <assert.h>
#include <execinfo.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;
typedef float f32;
typedef double f64;
typedef size_t usize;
typedef ssize_t isize;

void _lbvm_print_stacktrace() {
  void *callstack[128];
  int frames = backtrace(callstack, 128);
  char **strs = backtrace_symbols(callstack, frames);
  for (u32 i = 0; i < frames; i++) {
    fprintf(stderr, "%s\n", strs[i]);
  }
  free(strs);
}

#define PANIC()                                                                                                        \
  (fprintf(stderr, "[%s@%s:%d] PANIC\n", __FUNCTION__, __FILE__, __LINE__), _lbvm_print_stacktrace(), exit(1))
#define PANIC_PRINT(...)                                                                                               \
  (fprintf(stderr, "[%s@%s:%d] PANIC\n", __FUNCTION__, __FILE__, __LINE__), fprintf(stderr, __VA_ARGS__),              \
   _lbvm_print_stacktrace(), exit(1))

/// Return `0` to the caller if value is `0`
#define TRY_NULL(X)                                                                                                    \
  {                                                                                                                    \
    if ((X) == 0) {                                                                                                    \
      return 0;                                                                                                        \
    }                                                                                                                  \
  }

#define TRANSMUTE(TY, X) (*(TY*)&(X))

#define TODO() (printf("[%s@%s:%d] Unimplemented\n", __FUNCTION__, __FILE__, __LINE__), exit(1))
