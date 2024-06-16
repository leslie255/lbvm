#pragma once

#include "common.h"

#define DEF_PRINT_FUNC(TY, FMT)                                                                                        \
  static inline void print_##TY(TY x) { printf(FMT, x); }

DEF_PRINT_FUNC(u8, "%u");
DEF_PRINT_FUNC(u16, "%u");
DEF_PRINT_FUNC(u32, "%u");
DEF_PRINT_FUNC(u64, "%llu");
DEF_PRINT_FUNC(usize, "%zu");
DEF_PRINT_FUNC(i8, "%d");
DEF_PRINT_FUNC(i16, "%d");
DEF_PRINT_FUNC(i32, "%d");
DEF_PRINT_FUNC(i64, "%lld");
DEF_PRINT_FUNC(isize, "%zd");
DEF_PRINT_FUNC(f64, "%lf");
DEF_PRINT_FUNC(f32, "%f");
DEF_PRINT_FUNC(char, "%c");
static inline void print_bool(bool x) { printf("%s", x ? "true" : "false"); }
static inline void print_str(const char *x) {
  printf("\"");
  for (usize i = 0; x[i] != '\0'; ++i) {
    char c = x[i];
    switch (c) {
    case '\a':
      printf("\\a");
      break;
    case '\b':
      printf("\\b");
      break;
    case '\e':
      printf("\\e");
      break;
    case '\f':
      printf("\\f");
      break;
    case '\n':
      printf("\\n");
      break;
    case '\r':
      printf("\\r");
      break;
    case '\t':
      printf("\\t");
      break;
    case '\v':
      printf("\\v");
      break;
    case '\\':
      printf("\\\\");
      break;
    case '\'':
      printf("\\\'");
      break;
    case '\"':
      printf("\\\"");
      break;
    default:
      printf("%c", c);
    }
  }
  printf("\"");
}

#define DEF_PRINT_HEX_FUNC(TY, FMT)                                                                                    \
  static inline void print_hex_##TY(TY x) { printf("0x" FMT, x); }

DEF_PRINT_HEX_FUNC(u8, "%02X");
DEF_PRINT_HEX_FUNC(u16, "%04X");
DEF_PRINT_HEX_FUNC(u32, "%08X");
DEF_PRINT_HEX_FUNC(u64, "%016llX");
DEF_PRINT_HEX_FUNC(usize, "%016zX");
DEF_PRINT_HEX_FUNC(i8, "%02X");
DEF_PRINT_HEX_FUNC(i16, "%04X");
DEF_PRINT_HEX_FUNC(i32, "%08X");
DEF_PRINT_HEX_FUNC(i64, "%016llX");
DEF_PRINT_HEX_FUNC(isize, "%016zX");

/* clang-format off */
#define print(X)                                                                                                       \
  _Generic((X),                                                                                                        \
           u8: print_u8,                                                                                               \
           u16: print_u16,                                                                                             \
           u32: print_u32,                                                                                             \
           u64: print_u64,                                                                                             \
           usize: print_usize,                                                                                         \
           i8:  print_i8,                                                                                              \
           i16: print_i16,                                                                                             \
           i32: print_i32,                                                                                             \
           i64: print_i64,                                                                                             \
           isize: print_isize,                                                                                         \
           f32: print_f32,                                                                                             \
           f64: print_f64,                                                                                             \
           bool: print_bool,                                                                                           \
           char: print_char,                                                                                           \
           char*: print_str,                                                                                           \
           const char*: print_str                                                                                      \
)(X)
/* clang-format on */

#define println(X)                                                                                                     \
  ({                                                                                                                   \
    print(X);                                                                                                          \
    printf("\n");                                                                                                      \
    ({});                                                                                                              \
  })

/* clang-format off */
#define print_hex(X)                                                                                                   \
  _Generic((X),                                                                                                        \
           u8: print_hex_u8,                                                                                           \
           u16: print_hex_u16,                                                                                         \
           u32: print_hex_u32,                                                                                         \
           u64: print_hex_u64,                                                                                         \
           usize: print_hex_usize,                                                                                     \
           i8: print_hex_i8,                                                                                           \
           i16: print_hex_i16,                                                                                         \
           i32: print_hex_i32,                                                                                         \
           i64: print_hex_i64,                                                                                         \
           isize: print_hex_isize                                                                                      \
)(X)
/* clang-format on */

/// Print the current source location (e.g. [func@source.c:255]) for quick and dirty debugging.
#define dbg()                                                                                                          \
  ({                                                                                                                   \
    printf("[%s@%s:%d]\n", __FUNCTION__, __FILE__, __LINE__);                                                          \
    ({});                                                                                                              \
  })

/// Print the a value for quick and dirty debugging, with the current source location (e.g. [func@source.c:255]).
/// Only works on primitive types.
#define dbg_println(X)                                                                                                 \
  ({                                                                                                                   \
    __auto_type X_ = (X);                                                                                              \
    printf("[%s@%s:%d] (%s) = ", __FUNCTION__, __FILE__, __LINE__, #X);                                                \
    print(X_);                                                                                                         \
    printf("\n");                                                                                                      \
    X_;                                                                                                                \
  })

/// Like `printf` but starts with the current source location (e.g. [func@source.c:255]), for quick and dirty debugging.
#define dbg_printf(...)                                                                                                \
  ({                                                                                                                   \
    int I_ = 0;                                                                                                        \
    I_ += printf("[%s@%s:%d] ", __FUNCTION__, __FILE__, __LINE__);                                                     \
    I_ += printf(__VA_ARGS__);                                                                                         \
    I_;                                                                                                                \
  })

/// Print a value in hex format for quick and dirty debugging, with the current source location (e.g.
/// [func@source.c:255]).
/// Only works on integral types.
#define dbg_println_hex(X)                                                                                             \
  ({                                                                                                                   \
    __auto_type X_ = (X);                                                                                              \
    printf("[%s@%s:%d] (%s) = ", __FUNCTION__, __FILE__, __LINE__, #X);                                                \
    print_hex(X_);                                                                                                     \
    printf("\n");                                                                                                      \
    X_;                                                                                                                \
  })
