#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__APPLE__) && defined(__MACH__)
#define MODERN_APPLE
#endif

#if defined(__unix__) || defined(__unix) || defined(unix) || defined(MODERN_APPLE)
#define UNIX_OR_MODERN_APPLE
#endif

#ifdef UNIX_OR_MODERN_APPLE
#include <execinfo.h>
#endif

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
typedef u8 char8;
typedef u16 char16;
typedef u32 char32;

#define attribute(...) __attribute__((__VA_ARGS__))

/// Take reference of an rvalue.
/// Useful in some macros.
#define ref_rvalue(X) ((typeof(X) *)&(struct { typeof(X) _; }){X})

/// Alloc and copy something onto the heap.
#define put_on_heap(X) ((typeof(X) *)memcpy(xalloc(typeof(X), 1), REF_RVALUE(X), sizeof(X)))

/// Aka pointer casting, aka reinterpret casting.
#define transmute(TY, X) (*(TY *)ref_rvalue(X))

/// Number of elements in an array.
#define arr_len(X) (sizeof(X) / sizeof((X)[0]))

/// Return `0` to the caller if value is `0`
#define TRY(X)                                                                                                         \
  ({                                                                                                                   \
    __auto_type X_ = (X);                                                                                              \
    if (X_ == 0)                                                                                                       \
      return 0;                                                                                                        \
    X_;                                                                                                                \
  })

#if UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
#define PTR_SIZE 8
#elif UINTPTR_MAX == 0xFFFFFFFF
#define PTR_SIZE 4
#elif UINTPTR_MAX == 0xFFFF
#define PTR_SIZE 2
#else
#error "unsupported architecture"
#endif

// Panic and assert.

#ifdef UNIX_OR_MODERN_APPLE
static inline void print_stacktrace(FILE *f) {
  void *callstack[128];
  i32 frames = backtrace(callstack, 128);
  char **strs = backtrace_symbols(callstack, frames);
  for (u32 i = 0; i < (u32)frames; i++) {
    fprintf(f, "%s\n", strs[i]);
  }
  free(strs);
}
#endif

#ifndef panic_handler
attribute(noreturn, noinline) static inline void default_panic_handler() {
#ifdef UNIX_OR_MODERN_APPLE
  print_stacktrace(stderr);
#else
  fprint(stderr, "PANIC\n");
#endif
  exit(1);
}
#define panic_handler default_panic_handler
#endif

#define panic()                                                                                                        \
  ({                                                                                                                   \
    fprintf(stderr, "[%s@%s:%d] PANIC\n", __FUNCTION__, __FILE__, __LINE__);                                           \
    panic_handler();                                                                                                   \
  })

#define panic_printf(...)                                                                                              \
  ({                                                                                                                   \
    fprintf(stderr, "[%s@%s:%d] PANIC\n", __FUNCTION__, __FILE__, __LINE__);                                           \
    fprintf(stderr, __VA_ARGS__);                                                                                      \
    panic_handler();                                                                                                   \
  })

/// Assert with stacktrace on failure.
#define xassert(COND)                                                                                                  \
  ({                                                                                                                   \
    if (!(COND)) {                                                                                                    \
      panic_printf("Assertion failed: (%s) == false\n", #COND);                                                        \
    }                                                                                                                  \
  })

// Alloc failure handler.

#ifndef alloc_fail_handler
#define alloc_fail_handler default_alloc_fail_handler
attribute(noreturn, noinline) static inline void default_alloc_fail_handler() {
  fprintf(stderr, "ALLOC FAILED\n");
  exit(1);
}
#endif

// Default allocator.

#ifndef xalloc
attribute(always_inline) static inline void *xalloc_(usize len) {
  void *p = malloc(len);
  if (p == NULL)
    alloc_fail_handler();
  return p;
}
#define xalloc(TY, COUNT) ((TY *)xalloc_(sizeof(TY) * COUNT))
#endif

#ifndef xrealloc
attribute(always_inline) static inline void *xrealloc_(void *p, usize len) {
  p = realloc(p, len);
  if (p == NULL)
    alloc_fail_handler();
  return p;
}
#define xrealloc(P, TY, COUNT) ((TY *)xrealloc_((P), sizeof(TY) * COUNT))
#endif

#ifndef xfree
attribute(always_inline) static inline void xfree_(void *p) {
  free(p);
}
#define xfree(P) (xfree_(P))
#endif

#ifdef DEBUG
/// Assert only in debug mode.
#define debug_assert(COND) (assert(COND))
#else
/// Assert only in debug mode.
#define debug_assert(COND) ({})
#endif
