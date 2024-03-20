#pragma once

#include "common.h"

// Pretty sure `_lbvm` prefixes are safe to use despite starting with underscore.
// Unless Louisiana Board of Veterinary Medicine uses this prefix in their C projects.
#define DEF_PRINT_FUNC(TY, FMT)                                                                                        \
  static inline void _lbvm_print_##TY(TY x) { printf(FMT, x); }

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
static inline void _lbvm_print_bool(bool x) { printf("%s", x ? "true" : "false"); }
static inline void _lbvm_print_str(const char *x) { printf("%s", x); }

#define DEF_PRINT_HEX_FUNC(TY, FMT)                                                                                    \
  static inline void _lbvm_print_hex_##TY(TY x) { printf("0x" FMT, x); }

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

#define PRINT(X)                                                                                                       \
  _Generic((X),                                                                                                        \
           u8: _lbvm_print_u8,                                                                                              \
           u16: _lbvm_print_u16,                                                                                            \
           u32: _lbvm_print_u32,                                                                                            \
           u64: _lbvm_print_u64,                                                                                            \
           usize: _lbvm_print_usize,                                                                                        \
           i8:  _lbvm_print_i8,                                                                                             \
           i16: _lbvm_print_i16,                                                                                            \
           i32: _lbvm_print_i32,                                                                                            \
           i64: _lbvm_print_i64,                                                                                            \
           isize: _lbvm_print_isize,                                                                                        \
           f32: _lbvm_print_f32,                                                                                            \
           f64: _lbvm_print_f64,                                                                                            \
           bool: _lbvm_print_bool,                                                                                          \
           char: _lbvm_print_char,                                                                                          \
           char*: _lbvm_print_str,                                                                                          \
           const char*: _lbvm_print_str                                                                                     \
)(X)

#define PRINT_HEX(X)                                                                                                   \
  _Generic((X), u8                                                                                                     \
           : _lbvm_print_hex_u8, u16                                                                                        \
           : _lbvm_print_hex_u16, u32                                                                                       \
           : _lbvm_print_hex_u32, u64                                                                                       \
           : _lbvm_print_hex_u64, usize                                                                                     \
           : _lbvm_print_hex_usize, i8                                                                                      \
           : _lbvm_print_hex_i8, i16                                                                                        \
           : _lbvm_print_hex_i16, i32                                                                                       \
           : _lbvm_print_hex_i32, i64                                                                                       \
           : _lbvm_print_hex_i64, isize                                                                                     \
           : _lbvm_print_hex_isize)(X)

#define DBG() (printf("[%s@%s:%d]\n", __FUN CTION__, __FILE__, __LINE__))
#define DBG_PRINT_HEX(X) (printf("[%s@%s:%d] " #X " = ", __FUNCTION__, __FILE__, __LINE__), PRINT_HEX(X), printf("\n"))
#define DBG_PRINT(X) (printf("[%s@%s:%d] " #X " = ", __FUNCTION__, __FILE__, __LINE__), PRINT(X), printf("\n"))
