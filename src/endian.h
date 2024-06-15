#pragma once

#include "common.h"

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define IS_LITTLE_ENDIAN
static inline u8 u8_from_be(u8 x) { return x; }
static inline u16 u16_from_be(u16 x) { return __builtin_bswap16(x); }
static inline u32 u32_from_be(u32 x) { return __builtin_bswap32(x); }
static inline u64 u64_from_be(u64 x) { return __builtin_bswap64(x); }
static inline usize usize_from_be(usize x) {
#if PTR_SIZE == 8
  return __builtin_bswap64((u64)x);
#elif PTR_SIZE == 4
  return __builtin_bswap32((u32)x);
#elif PTR_SIZE == 2
  return __builtin_bswap16((u16)x);
#else
  __builtin_unreachable();
#endif
}
static inline u8 u8_from_le(u8 x) { return x; }
static inline u16 u16_from_le(u16 x) { return x; }
static inline u32 u32_from_le(u32 x) { return x; }
static inline u64 u64_from_le(u64 x) { return x; }
static inline usize usize_from_le(usize x) { return x; }
static inline u8 u8_to_be(u8 x) { return x; }
static inline u16 u16_to_be(u16 x) { return __builtin_bswap16(x); }
static inline u32 u32_to_be(u32 x) { return __builtin_bswap32(x); }
static inline u64 u64_to_be(u64 x) { return __builtin_bswap64(x); }
static inline usize usize_to_be(usize x) {
#if PTR_SIZE == 8
  return __builtin_bswap64((u64)x);
#elif PTR_SIZE == 4
  return __builtin_bswap32((u32)x);
#elif PTR_SIZE == 2
  return __builtin_bswap16((u16)x);
#else
  __builtin_unreachable();
#endif
}
static inline u8 u8_to_le(u8 x) { return x; }
static inline u16 u16_to_le(u16 x) { return x; }
static inline u32 u32_to_le(u32 x) { return x; }
static inline u64 u64_to_le(u64 x) { return x; }
static inline usize usize_to_le(usize x) { return x; }
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define IS_BIG_ENDIAN
static inline u8 u8_from_be(u8 x) { return x; }
static inline u16 u16_from_be(u16 x) { return x; }
static inline u32 u32_from_be(u32 x) { return x; }
static inline u64 u64_from_be(u64 x) { return x; }
static inline usize usize_from_be(usize x) { return x; }
static inline u8 u8_from_le(u8 x) { return x; }
static inline u16 u16_from_le(u16 x) { return __builtin_bswap16(x); }
static inline u32 u32_from_le(u32 x) { return __builtin_bswap32(x); }
static inline u64 u64_from_le(u64 x) { return __builtin_bswap64(x); }
static inline usize usize_from_le(usize x) {
#if PTR_SIZE == 8
  return __builtin_bswap64((u64)x);
#elif PTR_SIZE == 4
  return __builtin_bswap32((u32)x);
#elif PTR_SIZE == 2
  return __builtin_bswap16((u16)x);
#else
  __builtin_unreachable();
#endif
}
static inline u8 u8_to_be(u8 x) { return x; }
static inline u16 u16_to_be(u16 x) { return x; }
static inline u32 u32_to_be(u32 x) { return x; }
static inline u64 u64_to_be(u64 x) { return x; }
static inline usize usize_to_be(usize x) { return x; }
static inline u8 u8_to_le(u8 x) { return __builtin_bswap16(x); }
static inline u16 u16_to_le(u16 x) { return __builtin_bswap16(x); }
static inline u32 u32_to_le(u32 x) { return __builtin_bswap32(x); }
static inline u64 u64_to_le(u64 x) { return __builtin_bswap64(x); }
static inline usize usize_to_le(usize x) {
#if PTR_SIZE == 8
  return __builtin_bswap64((u64)x);
#elif PTR_SIZE == 4
  return __builtin_bswap32((u32)x);
#elif PTR_SIZE == 2
  return __builtin_bswap16((u16)x);
#else
  __builtin_unreachable();
#endif
}
#else
#error "unsupported endianness"
static inline u8 u8_from_be(u8 x) {
  (void)x;
  __builtin_unreachable();
}
static inline u16 u16_from_be(u16 x) {
  (void)x;
  __builtin_unreachable();
}
static inline u32 u32_from_be(u32 x) {
  (void)x;
  __builtin_unreachable();
}
static inline u64 u64_from_be(u64 x) {
  (void)x;
  __builtin_unreachable();
}
static inline usize usize_from_be(usize x) {
  (void)x;
  __builtin_unreachable();
}
static inline u8 u8_from_le(u8 x) {
  (void)x;
  __builtin_unreachable();
}
static inline u16 u16_from_le(u16 x) {
  (void)x;
  __builtin_unreachable();
}
static inline u32 u32_from_le(u32 x) {
  (void)x;
  __builtin_unreachable();
}
static inline u64 u64_from_le(u64 x) {
  (void)x;
  __builtin_unreachable();
}
static inline usize usize_from_le(usize x) {
  (void)x;
  __builtin_unreachable();
}
static inline u8 u8_to_be(u8 x) {
  (void)x;
  __builtin_unreachable();
}
static inline u16 u16_to_be(u16 x) {
  (void)x;
  __builtin_unreachable();
}
static inline u32 u32_to_be(u32 x) {
  (void)x;
  __builtin_unreachable();
}
static inline u64 u64_to_be(attribute(u64 x) {
  (void)x;
  __builtin_unreachable();
}
static inline usize usize_to_be(usize x) {
  (void)x;
  __builtin_unreachable();
}
static inline u8 u8_to_le(u8 x) {
  (void)x;
  __builtin_unreachable();
}
static inline u16 u16_to_le(u16 x) {
  (void)x;
  __builtin_unreachable();
}
static inline u32 u32_to_le(u32 x) {
  (void)x;
  __builtin_unreachable();
}
static inline u64 u64_to_le(u64 x) {
  (void)x;
  __builtin_unreachable();
}
static inline usize usize_to_le(usize x) {
  (void)x;
  __builtin_unreachable();
}
#endif

static inline u8 u8_from_be_bytes(const u8 *x) { return *x; }
static inline u16 u16_from_be_bytes(const u8 *x) {
  u16 y;
  memcpy(&y, x, sizeof(y));
  return u16_from_be(y);
}
static inline u32 u32_from_be_bytes(const u8 *x) {
  u32 y;
  memcpy(&y, x, sizeof(y));
  return u32_from_be(y);
}
static inline u64 u64_from_be_bytes(const u8 *x) {
  u64 y;
  memcpy(&y, x, sizeof(y));
  return u64_from_be(y);
}
static inline usize usize_from_be_bytes(const u8 *x) {
  if (sizeof(usize) == 8)
    return u64_from_be_bytes(x);
  else if (sizeof(usize) == 4)
    return u32_from_be_bytes(x);
  else if (sizeof(usize) == 2)
    return u16_from_be_bytes(x);
  else
    __builtin_unreachable();
}
static inline u8 u8_from_le_bytes(const u8 *x) { return *x; }
static inline u16 u16_from_le_bytes(const u8 *x) {
  u16 y = 0;
  memcpy(&y, x, sizeof(y));
  return u16_from_le(y);
}
static inline u32 u32_from_le_bytes(const u8 *x) {
  u32 y = 0;
  memcpy(&y, x, sizeof(y));
  return u32_from_le(y);
}
static inline u64 u64_from_le_bytes(const u8 *x) {
  u64 y = 0;
  memcpy(&y, x, sizeof(y));
  return u64_from_le(y);
}
static inline usize usize_from_le_bytes(const u8 *x) {
#if PTR_SIZE == 8
  return u64_from_le_bytes(x);
#elif PTR_SIZE == 4
  return u32_from_le_bytes(x);
#elif PTR_SIZE == 2
  return u16_from_le_bytes(x);
#else
  __builtin_unreachable();
#endif
}

#define to_be(X) _Generic((X), u8 : u8_to_be, u16 : u16_to_be, u32 : u32_to_be, u64 : u64_to_be, usize : usize_to_be)(X)

#define to_le(X) _Generic((X), u8 : u8_to_le, u16 : u16_to_le, u32 : u32_to_le, u64 : u64_to_le, usize : usize_to_le)(X)

#define from_be(X)                                                                                                     \
  _Generic((X), u8 : u8_from_be, u16 : u16_from_be, u32 : u32_from_be, u64 : u64_from_be, usize : usize_from_be)(X)

#define from_le(X)                                                                                                     \
  _Generic((X), u8 : u8_from_le, u16 : u16_from_le, u32 : u32_from_le, u64 : u64_from_le, usize : usize_from_le)(X)
