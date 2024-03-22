#pragma once

#include "common.h"
#include "debug_utils.h"
#include "values.h"

static inline void lbvm_check_platform_compatibility() {
  if (sizeof(void *) != 8) {
    PANIC_PRINT("This LBVM emulator requires 64-bit host platform\n");
  }
  u16 x = 1;
  u8 *ptr = (u8 *)&x;
  if (*ptr != 1) {
    PANIC_PRINT("This LBVM emulator requires little endian host platform\n");
  }
}

typedef struct machine Machine;

typedef void (*breakpoint_callback_t)(struct machine *);

struct machine {
  union machine_status_reg {
    uint64_t numeric;
    struct {
      bool flag_n : 1;
      bool flag_z : 1;
      bool flag_c : 1;
      bool flag_v : 1;
      bool flag_e : 1;
      bool flag_g : 1;
      bool flag_l : 1;
    };
  } reg_status;
  u16 pc;
  u64 reg_0;
  u64 reg_1;
  u64 reg_2;
  u64 reg_3;
  u64 reg_4;
  u64 reg_5;
  u64 reg_6;
  u64 reg_7;
  u64 reg_8;
  u64 reg_9;
  u64 reg_10;
  u64 reg_11;
  u64 reg_12;
  u64 reg_13;
  u64 reg_sp;
  u8 *vmem;
  void *breakpoint_callback_context;
  breakpoint_callback_t breakpoint_callback;
};

static inline u64 *machine_reg(Machine *machine, u8 reg_code) {
  switch (reg_code) {
  case REG_0:
    return &machine->reg_0;
  case REG_1:
    return &machine->reg_1;
  case REG_2:
    return &machine->reg_2;
  case REG_3:
    return &machine->reg_3;
  case REG_4:
    return &machine->reg_4;
  case REG_5:
    return &machine->reg_5;
  case REG_6:
    return &machine->reg_6;
  case REG_7:
    return &machine->reg_7;
  case REG_8:
    return &machine->reg_8;
  case REG_9:
    return &machine->reg_9;
  case REG_10:
    return &machine->reg_10;
  case REG_11:
    return &machine->reg_11;
  case REG_12:
    return &machine->reg_12;
  case REG_13:
    return &machine->reg_13;
  case REG_STATUS:
    return &machine->reg_status.numeric;
  case REG_SP:
    return &machine->reg_sp;
  default:
    PANIC_PRINT("Called `%s` with invalid reg code %u\n", __FUNCTION__, reg_code);
  }
}

#define MACHINE_CHECK_PC_OVERFLOW(MACHINE, LEN)                                                                        \
  if (MACHINE->pc + LEN > VMEM_SIZE) {                                                                                 \
    fprintf(stderr, "PC overflowed\n");                                                                                \
    return false;                                                                                                      \
  }

/// Fetch the 8 data bytes on pc for big instructions.
/// Make sure to do `MACHINE_CHECK_PC_OVERFLOW(machine, 8)` before this!
static inline u64 machine_fetch_data_qword(Machine *machine) {
  u64 value;
  memcpy(&value, &machine->vmem[machine->pc], 8);
  machine->pc += 8;
  return value;
}

static inline bool machine_jump_offset(Machine *machine, u16 offset) {
  MACHINE_CHECK_PC_OVERFLOW(machine, offset);
  machine->pc += offset;
  return true;
}

static inline size_t oplen_to_size(const u8 oplen) {
  switch (oplen) {
  case OPLEN_8:
    return 8;
  case OPLEN_4:
    return 4;
  case OPLEN_2:
    return 2;
  case OPLEN_1:
    return 1;
  default:
    PANIC();
  }
}

static inline void *solve_addr(Machine *machine, u8 vmem_flag, u64 addr) {
  if (vmem_flag) {
    return (void *)addr;
  } else {
    if (addr > VMEM_SIZE) {
      fprintf(stderr, "Out of bound stack access @ 0x%04X\n", machine->pc - 4);
      return NULL;
    }
    return &machine->vmem[addr];
  }
}

static inline u64 mask_val(u64 value, u8 oplen) {
  switch (oplen) {
  case OPLEN_8:
    return value;
  case OPLEN_4:
    return value & 0x00000000FFFFFFFF;
  case OPLEN_2:
    return value & 0x000000000000FFFF;
  case OPLEN_1:
    return value & 0x00000000000000FF;
  default:
    PANIC_PRINT("Called `%s` with illegal oplen %u\n", __FUNCTION__, oplen);
  }
}

static inline u64 mask_val_and_set_flag_n(Machine *machine, u64 value, u8 oplen) {
  switch (oplen) {
  case OPLEN_8:
    machine->reg_status.flag_n = (i64)value < 0;
    return value;
  case OPLEN_4:
    machine->reg_status.flag_n = (i32)value < 0;
    return value & 0x00000000FFFFFFFF;
  case OPLEN_2:
    machine->reg_status.flag_n = (i16)value < 0;
    return value & 0x000000000000FFFF;
  case OPLEN_1:
    machine->reg_status.flag_n = (i8)value < 0;
    return value & 0x00000000000000FF;
  default:
    PANIC_PRINT("Called `%s` with illegal oplen %u\n", __FUNCTION__, oplen);
  }
}

#define GET_OPERAND0(INST) ((INST)[1] & 0b00001111)
#define GET_OPERAND1(INST) (((INST)[1] & 0b11110000) >> 4)
#define GET_OPERAND2(INST) ((INST)[2] & 0b00001111)
#define GET_OPERAND3(INST) (((INST)[2] & 0b11110000) >> 4)
#define GET_FLAGS(INST) ((INST)[3])
#define GET_JUMP_OFFSET(INST) (((u16)((INST)[1])) | (u16)((INST)[2] << 8))

static inline bool machine_libc_call(Machine *machine, u8 callcode) {
  switch (callcode) {
  case LIBC_exit: {
    u8 arg0 = (*(u8 *)&(machine->reg_0));
    printf("Machine called libc function `exit` with code %u", arg0);
    return false;
  } break;
  case LIBC_malloc: {
    usize arg0 = (*(usize *)&(machine->reg_0));
    machine->reg_0 = (u64)malloc(arg0);
  } break;
  case LIBC_realloc: {
    void *arg0 = (*(void **)&(machine->reg_0));
    usize arg1 = (*(usize *)&(machine->reg_1));
    machine->reg_0 = (u64)realloc(arg0, arg1);
  } break;
  case LIBC_free: {
    void *arg0 = (*(void **)&(machine->reg_0));
    free(arg0);
  } break;
  case LIBC_fwrite: {
    const void *restrict arg0 = (*(const void *restrict *)&(machine->reg_0));
    size_t arg1 = (*(size_t *)&(machine->reg_1));
    size_t arg2 = (*(size_t *)&(machine->reg_2));
    FILE *arg3 = (*(FILE **)&(machine->reg_3));
    machine->reg_0 = fwrite(arg0, arg1, arg2, arg3);
  } break;
  case LIBC_fread: {
    void *restrict arg0 = (*(void *restrict *)&(machine->reg_0));
    size_t arg1 = (*(size_t *)&(machine->reg_1));
    size_t arg2 = (*(size_t *)&(machine->reg_2));
    FILE *arg3 = (*(FILE **)&(machine->reg_3));
    machine->reg_0 = fread(arg0, arg1, arg2, arg3);
  } break;
  case LIBC_printf: {
    const char *restrict arg0 = (*(const char *restrict *)&(machine->reg_0));
    u64 arg1 = (*(u64 *)&(machine->reg_1));
    u64 arg2 = (*(u64 *)&(machine->reg_2));
    u64 arg3 = (*(u64 *)&(machine->reg_3));
    u64 arg4 = (*(u64 *)&(machine->reg_4));
    u64 arg5 = (*(u64 *)&(machine->reg_5));
    u64 arg6 = (*(u64 *)&(machine->reg_6));
    u64 arg7 = (*(u64 *)&(machine->reg_7));
    u64 arg8 = (*(u64 *)&(machine->reg_8));
    u64 arg9 = (*(u64 *)&(machine->reg_9));
    u64 arg10 = (*(u64 *)&(machine->reg_10));
    u64 arg11 = (*(u64 *)&(machine->reg_11));
    u64 arg12 = (*(u64 *)&(machine->reg_12));
    u64 arg13 = (*(u64 *)&(machine->reg_13));
    machine->reg_0 =
        (u64)printf(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13);
  } break;
  case LIBC_fprintf: {
    FILE *arg0 = (*(FILE **)&(machine->reg_0));
    const char *restrict arg1 = (*(const char *restrict *)&(machine->reg_1));
    u64 arg2 = (*(u64 *)&(machine->reg_2));
    u64 arg3 = (*(u64 *)&(machine->reg_3));
    u64 arg4 = (*(u64 *)&(machine->reg_4));
    u64 arg5 = (*(u64 *)&(machine->reg_5));
    u64 arg6 = (*(u64 *)&(machine->reg_6));
    u64 arg7 = (*(u64 *)&(machine->reg_7));
    u64 arg8 = (*(u64 *)&(machine->reg_8));
    u64 arg9 = (*(u64 *)&(machine->reg_9));
    u64 arg10 = (*(u64 *)&(machine->reg_10));
    u64 arg11 = (*(u64 *)&(machine->reg_11));
    u64 arg12 = (*(u64 *)&(machine->reg_12));
    u64 arg13 = (*(u64 *)&(machine->reg_13));
    machine->reg_0 =
        (u64)fprintf(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13);
  } break;
  case LIBC_scanf: {
    const char *restrict arg0 = (*(const char *restrict *)&(machine->reg_0));
    u64 arg1 = (*(u64 *)&(machine->reg_1));
    u64 arg2 = (*(u64 *)&(machine->reg_2));
    u64 arg3 = (*(u64 *)&(machine->reg_3));
    u64 arg4 = (*(u64 *)&(machine->reg_4));
    u64 arg5 = (*(u64 *)&(machine->reg_5));
    u64 arg6 = (*(u64 *)&(machine->reg_6));
    u64 arg7 = (*(u64 *)&(machine->reg_7));
    u64 arg8 = (*(u64 *)&(machine->reg_8));
    u64 arg9 = (*(u64 *)&(machine->reg_9));
    u64 arg10 = (*(u64 *)&(machine->reg_10));
    u64 arg11 = (*(u64 *)&(machine->reg_11));
    u64 arg12 = (*(u64 *)&(machine->reg_12));
    u64 arg13 = (*(u64 *)&(machine->reg_13));
    machine->reg_0 = (u64)scanf(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13);
  } break;
  case LIBC_fscanf: {
    FILE *arg0 = (*(FILE **)&(machine->reg_0));
    const char *restrict arg1 = (*(const char *restrict *)&(machine->reg_1));
    u64 arg2 = (*(u64 *)&(machine->reg_2));
    u64 arg3 = (*(u64 *)&(machine->reg_3));
    u64 arg4 = (*(u64 *)&(machine->reg_4));
    u64 arg5 = (*(u64 *)&(machine->reg_5));
    u64 arg6 = (*(u64 *)&(machine->reg_6));
    u64 arg7 = (*(u64 *)&(machine->reg_7));
    u64 arg8 = (*(u64 *)&(machine->reg_8));
    u64 arg9 = (*(u64 *)&(machine->reg_9));
    u64 arg10 = (*(u64 *)&(machine->reg_10));
    u64 arg11 = (*(u64 *)&(machine->reg_11));
    u64 arg12 = (*(u64 *)&(machine->reg_12));
    u64 arg13 = (*(u64 *)&(machine->reg_13));
    machine->reg_0 =
        (u64)fscanf(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13);
  } break;
  case LIBC_puts: {
    const char *arg0 = (*(const char **)&(machine->reg_0));
    int result = puts(arg0);
    machine->reg_0 = (u64)result;
  } break;
  case LIBC_fputs: {
    const char *arg0 = (*(const char **)&(machine->reg_0));
    FILE *arg1 = (*(FILE **)&(machine->reg_1));
    machine->reg_0 = (u64)fputs(arg0, arg1);
  } break;
  case LIBC_snprintf: {
    char *restrict arg0 = (*(char *restrict *)&(machine->reg_0));
    usize arg1 = (*(usize *)&(machine->reg_1));
    const char *restrict arg2 = (*(const char *restrict *)&(machine->reg_2));
    u64 arg3 = (*(u64 *)&(machine->reg_3));
    u64 arg4 = (*(u64 *)&(machine->reg_4));
    u64 arg5 = (*(u64 *)&(machine->reg_5));
    u64 arg6 = (*(u64 *)&(machine->reg_6));
    u64 arg7 = (*(u64 *)&(machine->reg_7));
    u64 arg8 = (*(u64 *)&(machine->reg_8));
    u64 arg9 = (*(u64 *)&(machine->reg_9));
    u64 arg10 = (*(u64 *)&(machine->reg_10));
    u64 arg11 = (*(u64 *)&(machine->reg_11));
    u64 arg12 = (*(u64 *)&(machine->reg_12));
    u64 arg13 = (*(u64 *)&(machine->reg_13));
    machine->reg_0 =
        (u64)snprintf(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13);
  } break;
  case LIBC_fopen: {
    const char *restrict arg0 = (*(const char *restrict *)&(machine->reg_0));
    const char *restrict arg1 = (*(const char *restrict *)&(machine->reg_1));
    machine->reg_0 = (u64)fopen(arg0, arg1);
  } break;
  case LIBC_fclose: {
    FILE *arg0 = (*(FILE **)&(machine->reg_0));
    machine->reg_0 = (u64)fclose(arg0);
  } break;
  case LIBC_memcpy: {
    void *arg0 = (*(void **)&(machine->reg_0));
    void *arg1 = (*(void **)&(machine->reg_1));
    size_t arg2 = (*(size_t *)&(machine->reg_2));
    machine->reg_0 = (u64)memcpy(arg0, arg1, arg2);
  } break;
  case LIBC_memmove: {
    void *arg0 = (*(void **)&(machine->reg_0));
    void *arg1 = (*(void **)&(machine->reg_1));
    size_t arg2 = (*(size_t *)&(machine->reg_2));
    machine->reg_0 = (u64)memmove(arg0, arg1, arg2);
  } break;
  case LIBC_memset: {
    void *arg0 = (*(void **)&(machine->reg_0));
    int arg1 = (*(int *)&(machine->reg_1));
    size_t arg2 = (*(size_t *)&(machine->reg_2));
    machine->reg_0 = (u64)memset(arg0, arg1, arg2);
  } break;
  case LIBC_bzero: {
    void *arg0 = (*(void **)&(machine->reg_0));
    size_t arg1 = (*(size_t *)&(machine->reg_1));
    machine->reg_0 = (u64)bzero(arg0, arg1);
  } break;
  case LIBC_strlen: {
    const char *arg0 = (*(const char **)&(machine->reg_0));
    machine->reg_0 = (u64)strlen(arg0);
  } break;
  case LIBC_strcpy: {
    char *arg0 = (*(char **)&(machine->reg_0));
    const char *arg1 = (*(const char **)&(machine->reg_1));
    machine->reg_0 = (u64)strcpy(arg0, arg1);
  } break;
  case LIBC_strcat: {
    char *arg0 = (*(char **)&(machine->reg_0));
    const char *arg1 = (*(const char **)&(machine->reg_1));
    machine->reg_0 = (u64)strcat(arg0, arg1);
  } break;
  case LIBC_strcmp: {
    const char *arg0 = (*(const char **)&(machine->reg_0));
    const char *arg1 = (*(const char **)&(machine->reg_1));
    machine->reg_0 = (u64)strcmp(arg0, arg1);
  } break;
  }
  return true;
}

/// Returns `true` if should continue, `false` if should stop.
static inline bool machine_next(Machine *machine) {
  MACHINE_CHECK_PC_OVERFLOW(machine, 4);
  const u8 inst[4] = {
      machine->vmem[machine->pc + 0],
      machine->vmem[machine->pc + 1],
      machine->vmem[machine->pc + 2],
      machine->vmem[machine->pc + 3],
  };
  machine->pc += 4;
  const u8 opcode = inst[0] & 0b11111100;
  const u8 oplen = inst[0] & 0b00000011;
  switch (opcode) {
  case OPCODE_BRK:
    printf("BRK Interrupt @ 0x%04X\n", machine->pc - 4);
    return false;
  case OPCODE_CBRK: {
    u8 cond_flag = GET_FLAGS(inst);
    u8 rev = cond_flag & 0b10000000;
    bool cond = (u64)(cond_flag & 0b011111111) & machine->reg_status.numeric;
    if (rev)
      cond = !cond;
    if (cond) {
      printf("CBRK Interrupt @ 0x%04X\n", machine->pc - 4);
      return false;
    }
  } break;
  case OPCODE_NOP:
    break;
  case OPCODE_LOAD_IMM: {
    machine->reg_status.numeric = 0;
    u64 *dest_reg = machine_reg(machine, GET_OPERAND0(inst));
    MACHINE_CHECK_PC_OVERFLOW(machine, 8);
    u64 imm = machine_fetch_data_qword(machine);
    u64 imm_masked = mask_val_and_set_flag_n(machine, imm, oplen);
    machine->reg_status.flag_z = imm_masked == 0;
    *dest_reg = imm_masked;
  } break;
  case OPCODE_LOAD_DIR: {
    machine->reg_status.numeric = 0;
    u64 *dest_reg = machine_reg(machine, GET_OPERAND0(inst));
    u64 addr = *machine_reg(machine, GET_OPERAND1(inst));
    void *src = solve_addr(machine, GET_FLAGS(inst) & 0b00000001, addr);
    TRY_NULL(src);
    *dest_reg = 0;
    memcpy(dest_reg, src, oplen_to_size(oplen)); // use memcpy because address may be unaligned
    mask_val_and_set_flag_n(machine, *dest_reg, oplen);
    machine->reg_status.flag_z = src == 0;
  } break;
  case OPCODE_LOAD_IND: {
    machine->reg_status.numeric = 0;
    MACHINE_CHECK_PC_OVERFLOW(machine, 8);
    u64 src_addr_offset = machine_fetch_data_qword(machine);
    u64 src_addr_base = *machine_reg(machine, GET_OPERAND1(inst));
    u64 src_addr = src_addr_base + src_addr_offset;
    void *src = solve_addr(machine, GET_FLAGS(inst) & 0b00000001, src_addr);
    TRY_NULL(src);
    u64 *dest_reg = machine_reg(machine, GET_OPERAND0(inst));
    *dest_reg = 0;
    memcpy(dest_reg, src, oplen_to_size(oplen)); // use memcpy because address may be unaligned
    mask_val_and_set_flag_n(machine, *dest_reg, oplen);
    machine->reg_status.flag_z = src == 0;
  } break;
  case OPCODE_STORE_IMM: {
    machine->reg_status.numeric = 0;
    MACHINE_CHECK_PC_OVERFLOW(machine, 8);
    u64 dest_addr = machine_fetch_data_qword(machine);
    void *dest = solve_addr(machine, GET_FLAGS(inst) & 0b00000001, dest_addr);
    TRY_NULL(dest);
    u64 src = mask_val_and_set_flag_n(machine, *machine_reg(machine, GET_OPERAND0(inst)), oplen);
    memcpy(dest, &src, oplen_to_size(oplen));
    mask_val_and_set_flag_n(machine, src, oplen);
    machine->reg_status.flag_z = src == 0;
  } break;
  case OPCODE_STORE_DIR: {
    machine->reg_status.numeric = 0;
    u64 dest_addr = *machine_reg(machine, GET_OPERAND1(inst));
    void *dest = solve_addr(machine, GET_FLAGS(inst) & 0b00000001, dest_addr);
    TRY_NULL(dest);
    u64 src = mask_val_and_set_flag_n(machine, *machine_reg(machine, GET_OPERAND0(inst)), oplen);
    memcpy(dest, &src, oplen_to_size(oplen));
    mask_val_and_set_flag_n(machine, src, oplen);
    machine->reg_status.flag_z = src == 0;
  } break;
  case OPCODE_STORE_IND: {
    machine->reg_status.numeric = 0;
    u64 dest_addr_base = *machine_reg(machine, GET_OPERAND0(inst));
    MACHINE_CHECK_PC_OVERFLOW(machine, 8);
    u64 dest_addr_offset = machine_fetch_data_qword(machine);
    u64 dest_addr = dest_addr_base + dest_addr_offset;
    void *dest = solve_addr(machine, GET_FLAGS(inst) & 0b00000001, dest_addr);
    TRY_NULL(dest);
    u64 src = mask_val_and_set_flag_n(machine, *machine_reg(machine, GET_OPERAND0(inst)), oplen);
    memcpy(dest, &src, oplen_to_size(oplen));
    mask_val_and_set_flag_n(machine, src, oplen);
    machine->reg_status.flag_z = src == 0;
  } break;
  case OPCODE_MOV: {
    machine->reg_status.numeric = 0;
    u64 *dest_reg = machine_reg(machine, GET_OPERAND0(inst));
    u64 src = mask_val_and_set_flag_n(machine, *machine_reg(machine, GET_OPERAND1(inst)), oplen);
    *dest_reg = src;
  } break;
  case OPCODE_CMP: {
    machine->reg_status.numeric = 0;
    u64 lhs = mask_val(*machine_reg(machine, GET_OPERAND0(inst)), oplen);
    u64 rhs = mask_val(*machine_reg(machine, GET_OPERAND1(inst)), oplen);
    machine->reg_status.flag_z = lhs == 0;
    machine->reg_status.flag_e = lhs == rhs;
    machine->reg_status.flag_g = lhs > rhs;
    machine->reg_status.flag_l = lhs < rhs;
  } break;
  case OPCODE_CSEL: {
    u8 cond_flag = GET_FLAGS(inst);
    u8 rev = cond_flag & 0b10000000;
    bool cond = (u64)(cond_flag & 0b011111111) & machine->reg_status.numeric;
    if (rev)
      cond = !cond;
    u64 *dest_reg = machine_reg(machine, GET_OPERAND0(inst));
    u64 *src_reg = machine_reg(machine, cond ? GET_OPERAND1(inst) : GET_OPERAND2(inst));
    u64 src = mask_val(*src_reg, oplen);
    *dest_reg = src;
  } break;
  case OPCODE_B: {
    u8 cond_flag = GET_FLAGS(inst);
    u8 rev = cond_flag & 0b10000000;
    bool cond = (u64)(cond_flag & 0b011111111) & machine->reg_status.numeric;
    if (rev)
      cond = !cond;
    if (cond) {
      TRY_NULL(machine_jump_offset(machine, GET_JUMP_OFFSET(inst)));
    }
  } break;
  case OPCODE_J: {
    TRY_NULL(machine_jump_offset(machine, GET_JUMP_OFFSET(inst)));
  } break;
  case OPCODE_ADD: {
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    u64 rhs = *machine_reg(machine, GET_OPERAND2(inst));
    u64 result;
#define ADD_WITH_TY(TY, SIGNED_TY)                                                                                     \
  {                                                                                                                    \
    TY LHS_ = (TY)lhs;                                                                                                 \
    TY RHS_ = (TY)rhs;                                                                                                 \
    TY RESULT_ = LHS_ + RHS_;                                                                                          \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = (SIGNED_TY)(RESULT_) < 0;                                                             \
    machine->reg_status.flag_c = (RESULT_ < LHS_) | (RESULT_ < RHS_);                                                  \
    machine->reg_status.flag_v = (RESULT_ < LHS_) | (RESULT_ < RHS_);                                                  \
    result = RESULT_;                                                                                                  \
  };
    switch (oplen) {
    case OPLEN_8: {
      ADD_WITH_TY(u64, i64);
    } break;
    case OPLEN_4: {
      ADD_WITH_TY(u32, i64);
    } break;
    case OPLEN_2: {
      ADD_WITH_TY(u16, i64);
    } break;
    case OPLEN_1: {
      ADD_WITH_TY(u8, i64);
    } break;
    default:
      PANIC();
    }
    *dest = result;
  } break;
  case OPCODE_SUB: {
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    u64 rhs = *machine_reg(machine, GET_OPERAND2(inst));
    u64 result;
#define SUB_WITH_TY(TY, SIGNED_TY)                                                                                     \
  {                                                                                                                    \
    TY LHS_ = (TY)lhs;                                                                                                 \
    TY RHS_ = (TY)rhs;                                                                                                 \
    TY RESULT_ = LHS_ - RHS_;                                                                                          \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = (SIGNED_TY)(RESULT_) < 0;                                                             \
    machine->reg_status.flag_c = (RESULT_ < LHS_) | (RESULT_ < RHS_); /*carry flag is reversed for subtraction*/       \
    machine->reg_status.flag_v = (RESULT_ > LHS_) | (RESULT_ > RHS_);                                                  \
    result = RESULT_;                                                                                                  \
  };
    switch (oplen) {
    case OPLEN_8: {
      SUB_WITH_TY(u64, i64);
    } break;
    case OPLEN_4: {
      SUB_WITH_TY(u32, i32);
    } break;
    case OPLEN_2: {
      SUB_WITH_TY(u16, i16);
    } break;
    case OPLEN_1: {
      SUB_WITH_TY(u8, i8);
    } break;
    default:
      PANIC();
    }
    *dest = result;
  } break;
  case OPCODE_MUL: {
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    u64 rhs = *machine_reg(machine, GET_OPERAND2(inst));
    u64 result;
#define MUL_WITH_TY(TY, SIGNED_TY)                                                                                     \
  {                                                                                                                    \
    TY LHS_ = (TY)lhs;                                                                                                 \
    TY RHS_ = (TY)rhs;                                                                                                 \
    TY RESULT_ = LHS_ * RHS_;                                                                                          \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = (SIGNED_TY)(RESULT_) < 0;                                                             \
    machine->reg_status.flag_v = (RESULT_ < LHS_) | (RESULT_ < RHS_);                                                  \
    result = RESULT_;                                                                                                  \
  };
    switch (oplen) {
    case OPLEN_8: {
      MUL_WITH_TY(u64, i64);
    } break;
    case OPLEN_4: {
      MUL_WITH_TY(u32, i32);
    } break;
    case OPLEN_2: {
      MUL_WITH_TY(u16, i16);
    } break;
    case OPLEN_1: {
      MUL_WITH_TY(u8, i8);
    } break;
    default:
      PANIC();
    }
    *dest = result;
  } break;
  case OPCODE_DIV: {
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    u64 rhs = *machine_reg(machine, GET_OPERAND2(inst));
    u64 result;
#define DIV_WITH_TY(TY, SIGNED_TY)                                                                                     \
  {                                                                                                                    \
    TY LHS_ = (TY)lhs;                                                                                                 \
    TY RHS_ = (TY)rhs;                                                                                                 \
    TY RESULT_ = LHS_ / RHS_;                                                                                          \
    if (RHS_ == 0)                                                                                                     \
      fprintf(stderr, "Division by zero @ 0x%04X\n", machine->pc - 4);                                                 \
    return false;                                                                                                      \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = (SIGNED_TY)(RESULT_) < 0;                                                             \
    result = RESULT_;                                                                                                  \
  };
    switch (oplen) {
    case OPLEN_8: {
      DIV_WITH_TY(u64, i64);
    } break;
    case OPLEN_4: {
      DIV_WITH_TY(u32, i32);
    } break;
    case OPLEN_2: {
      DIV_WITH_TY(u16, i16);
    } break;
    case OPLEN_1: {
      DIV_WITH_TY(u8, i8);
    } break;
    default:
      PANIC();
    }
    *dest = result;
  } break;
  case OPCODE_MOD: {
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    u64 rhs = *machine_reg(machine, GET_OPERAND2(inst));
    u64 result;
#define MOD_WITH_TY(TY, SIGNED_TY)                                                                                     \
  {                                                                                                                    \
    TY LHS_ = (TY)lhs;                                                                                                 \
    TY RHS_ = (TY)rhs;                                                                                                 \
    TY RESULT_ = LHS_ % RHS_;                                                                                          \
    if (RHS_ == 0)                                                                                                     \
      fprintf(stderr, "Mod by zero @ 0x%04X\n", machine->pc - 4);                                                      \
    return false;                                                                                                      \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = (SIGNED_TY)(RESULT_) < 0;                                                             \
    result = RESULT_;                                                                                                  \
  };
    switch (oplen) {
    case OPLEN_8: {
      MOD_WITH_TY(u64, i64);
    } break;
    case OPLEN_4: {
      MOD_WITH_TY(u32, i64);
    } break;
    case OPLEN_2: {
      MOD_WITH_TY(u16, i64);
    } break;
    case OPLEN_1: {
      MOD_WITH_TY(u8, i64);
    } break;
    default:
      PANIC();
    }
    *dest = result;
  } break;
  case OPCODE_IADD: {
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    u64 rhs = *machine_reg(machine, GET_OPERAND2(inst));
    u64 result;
#define IADD_WITH_TY(TY)                                                                                               \
  {                                                                                                                    \
    TY LHS_ = (TY)lhs;                                                                                                 \
    TY RHS_ = (TY)rhs;                                                                                                 \
    TY RESULT_ = LHS_ + RHS_;                                                                                          \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = RESULT_ < 0;                                                                          \
    machine->reg_status.flag_c = (RESULT_ < LHS_) | (RESULT_ < RHS_);                                                  \
    machine->reg_status.flag_v = (RESULT_ < LHS_) | (RESULT_ < RHS_);                                                  \
    result = (u64)RESULT_;                                                                                             \
  };
    switch (oplen) {
    case OPLEN_8: {
      IADD_WITH_TY(i64);
    } break;
    case OPLEN_4: {
      IADD_WITH_TY(i32);
    } break;
    case OPLEN_2: {
      IADD_WITH_TY(i16);
    } break;
    case OPLEN_1: {
      IADD_WITH_TY(i8);
    } break;
    default:
      PANIC();
    }
    *dest = result;
  } break;
  case OPCODE_ISUB: {
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    u64 rhs = *machine_reg(machine, GET_OPERAND2(inst));
    u64 result;
#define ISUB_WITH_TY(TY)                                                                                               \
  {                                                                                                                    \
    TY LHS_ = (TY)lhs;                                                                                                 \
    TY RHS_ = (TY)rhs;                                                                                                 \
    TY RESULT_ = LHS_ - RHS_;                                                                                          \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = RESULT_ < 0;                                                                          \
    machine->reg_status.flag_c = (RESULT_ < LHS_) | (RESULT_ < RHS_); /*carry flag is reversed for subtraction*/       \
    machine->reg_status.flag_v = (RESULT_ > LHS_) | (RESULT_ > RHS_);                                                  \
    result = (u64)RESULT_;                                                                                             \
  };
    switch (oplen) {
    case OPLEN_8: {
      ISUB_WITH_TY(i64);
    } break;
    case OPLEN_4: {
      ISUB_WITH_TY(i32);
    } break;
    case OPLEN_2: {
      ISUB_WITH_TY(i16);
    } break;
    case OPLEN_1: {
      ISUB_WITH_TY(i8);
    } break;
    default:
      PANIC();
    }
    *dest = result;
  } break;
  case OPCODE_IMUL: {
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    u64 rhs = *machine_reg(machine, GET_OPERAND2(inst));
    u64 result;
#define IMUL_WITH_TY(TY)                                                                                               \
  {                                                                                                                    \
    TY LHS_ = (TY)lhs;                                                                                                 \
    TY RHS_ = (TY)rhs;                                                                                                 \
    TY RESULT_ = LHS_ * RHS_;                                                                                          \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = RESULT_ < 0;                                                                          \
    machine->reg_status.flag_v = (RESULT_ < LHS_) | (RESULT_ < RHS_);                                                  \
    result = (u64)RESULT_;                                                                                             \
  };
    switch (oplen) {
    case OPLEN_8: {
      IMUL_WITH_TY(i64);
    } break;
    case OPLEN_4: {
      IMUL_WITH_TY(i32);
    } break;
    case OPLEN_2: {
      IMUL_WITH_TY(i16);
    } break;
    case OPLEN_1: {
      IMUL_WITH_TY(i8);
    } break;
    default:
      PANIC();
    }
    *dest = result;
  } break;
  case OPCODE_IDIV: {
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    u64 rhs = *machine_reg(machine, GET_OPERAND2(inst));
    u64 result;
#define IDIV_WITH_TY(TY)                                                                                               \
  {                                                                                                                    \
    TY LHS_ = (TY)lhs;                                                                                                 \
    TY RHS_ = (TY)rhs;                                                                                                 \
    TY RESULT_ = LHS_ / RHS_;                                                                                          \
    if (RHS_ == 0) {                                                                                                   \
      fprintf(stderr, "Division by zero @ %04X\n", machine->pc - 4);                                                   \
      return false;                                                                                                    \
    }                                                                                                                  \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = RESULT_ < 0;                                                                          \
    result = (u64)RESULT_;                                                                                             \
  };
    switch (oplen) {
    case OPLEN_8: {
      IDIV_WITH_TY(i64);
    } break;
    case OPLEN_4: {
      IDIV_WITH_TY(i32);
    } break;
    case OPLEN_2: {
      IDIV_WITH_TY(i16);
    } break;
    case OPLEN_1: {
      IDIV_WITH_TY(i8);
    } break;
    default:
      PANIC();
    }
    *dest = result;
  } break;
  case OPCODE_IMOD: {
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    u64 rhs = *machine_reg(machine, GET_OPERAND2(inst));
    u64 result;
#define IMOD_WITH_TY(TY)                                                                                               \
  {                                                                                                                    \
    TY LHS_ = (TY)lhs;                                                                                                 \
    TY RHS_ = (TY)rhs;                                                                                                 \
    TY RESULT_ = LHS_ % RHS_;                                                                                          \
    if (RHS_ == 0) {                                                                                                   \
      fprintf(stderr, "Mod by zero @ %04X\n", machine->pc - 4);                                                        \
      return false;                                                                                                    \
    }                                                                                                                  \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = RESULT_ < 0;                                                                          \
    result = (u64)RESULT_;                                                                                             \
  };
    switch (oplen) {
    case OPLEN_8: {
      IMOD_WITH_TY(i64);
    } break;
    case OPLEN_4: {
      IMOD_WITH_TY(i32);
    } break;
    case OPLEN_2: {
      IMOD_WITH_TY(i16);
    } break;
    case OPLEN_1: {
      IMOD_WITH_TY(i8);
    } break;
    default:
      PANIC();
    }
    *dest = result;
  } break;
  case OPCODE_FADD: {
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    u64 rhs = *machine_reg(machine, GET_OPERAND2(inst));
    u64 result;
#define FADD_WITH_TY(TY)                                                                                               \
  {                                                                                                                    \
    TY LHS_ = TRANSMUTE(TY, lhs);                                                                                      \
    TY RHS_ = TRANSMUTE(TY, rhs);                                                                                      \
    TY RESULT_ = LHS_ + RHS_;                                                                                          \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = RESULT_ < 0;                                                                          \
    result = TRANSMUTE(u64, RESULT_);                                                                                  \
  };
    switch (oplen) {
    case OPLEN_8: {
      FADD_WITH_TY(f64);
    } break;
    case OPLEN_4: {
      FADD_WITH_TY(f32);
    } break;
    case OPLEN_2:
    case OPLEN_1: {
      fprintf(stderr, "Illegal instruction @ 0x%04X (note: floating point operations must only be qword or dword)\n",
              machine->pc - 4);
      return false;
    } break;
    default:
      PANIC();
    }
    *dest = result;
  } break;
  case OPCODE_FSUB: {
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    u64 rhs = *machine_reg(machine, GET_OPERAND2(inst));
    u64 result;
#define FSUB_WITH_TY(TY)                                                                                               \
  {                                                                                                                    \
    TY LHS_ = TRANSMUTE(TY, lhs);                                                                                      \
    TY RHS_ = TRANSMUTE(TY, rhs);                                                                                      \
    TY RESULT_ = LHS_ - RHS_;                                                                                          \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = RESULT_ < 0;                                                                          \
    result = TRANSMUTE(u64, RESULT_);                                                                                  \
  };
    switch (oplen) {
    case OPLEN_8: {
      FSUB_WITH_TY(f64);
    } break;
    case OPLEN_4: {
      FSUB_WITH_TY(f32);
    } break;
    case OPLEN_2:
    case OPLEN_1: {
      fprintf(stderr, "Illegal instruction @ 0x%04X (note: floating point operations must only be qword or dword)\n",
              machine->pc - 4);
      return false;
    } break;
    default:
      PANIC();
    }
    *dest = result;
  } break;
  case OPCODE_FMUL: {
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    u64 rhs = *machine_reg(machine, GET_OPERAND2(inst));
    u64 result;
#define FMUL_WITH_TY(TY)                                                                                               \
  {                                                                                                                    \
    TY LHS_ = TRANSMUTE(TY, lhs);                                                                                      \
    TY RHS_ = TRANSMUTE(TY, rhs);                                                                                      \
    TY RESULT_ = LHS_ * RHS_;                                                                                          \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = RESULT_ < 0;                                                                          \
    result = TRANSMUTE(u64, RESULT_);                                                                                  \
  };
    switch (oplen) {
    case OPLEN_8: {
      FMUL_WITH_TY(f64);
    } break;
    case OPLEN_4: {
      FMUL_WITH_TY(f32);
    } break;
    case OPLEN_2:
    case OPLEN_1: {
      fprintf(stderr, "Illegal instruction @ 0x%04X (note: floating point operations must only be qword or dword)\n",
              machine->pc - 4);
      return false;
    } break;
    default:
      PANIC();
    }
    *dest = result;
  } break;
  case OPCODE_FDIV: {
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    u64 rhs = *machine_reg(machine, GET_OPERAND2(inst));
    u64 result;
#define FDIV_WITH_TY(TY)                                                                                               \
  {                                                                                                                    \
    TY LHS_ = TRANSMUTE(TY, lhs);                                                                                      \
    TY RHS_ = TRANSMUTE(TY, rhs);                                                                                      \
    TY RESULT_ = LHS_ / RHS_;                                                                                          \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = RESULT_ < 0;                                                                          \
    result = TRANSMUTE(u64, RESULT_);                                                                                  \
  };
    switch (oplen) {
    case OPLEN_8: {
      FDIV_WITH_TY(f64);
    } break;
    case OPLEN_4: {
      FDIV_WITH_TY(f32);
    } break;
    case OPLEN_2:
    case OPLEN_1: {
      fprintf(stderr, "Illegal instruction @ 0x%04X (note: floating point operations must only be qword or dword)\n",
              machine->pc - 4);
      return false;
    } break;
    default:
      PANIC();
    }
    *dest = result;
  } break;
  case OPCODE_FMOD: {
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    u64 rhs = *machine_reg(machine, GET_OPERAND2(inst));
    u64 result;
    switch (oplen) {
    case OPLEN_8: {
      f64 lhs_ = TRANSMUTE(f64, (lhs));
      f64 rhs_ = TRANSMUTE(f64, (rhs));
      f64 result_ = fmod(lhs_, rhs_);
      machine->reg_status.numeric = 0;
      machine->reg_status.flag_z = result_ == 0;
      machine->reg_status.flag_n = result_ < 0;
      result = TRANSMUTE(u64, result_);
    } break;
    case OPLEN_4: {
      f32 lhs_ = TRANSMUTE(f32, (lhs));
      f32 rhs_ = TRANSMUTE(f32, (rhs));
      f32 result_ = fmodf(lhs_, rhs_);
      machine->reg_status.numeric = 0;
      machine->reg_status.flag_z = result_ == 0;
      machine->reg_status.flag_n = result_ < 0;
      result = TRANSMUTE(u64, result_);
    } break;
    case OPLEN_2:
    case OPLEN_1: {
      fprintf(stderr, "Illegal instruction @ 0x%04X (note: floating point operations must only be qword or dword)\n",
              machine->pc - 4);
      return false;
    } break;
    default:
      PANIC();
    }
    *dest = result;
  } break;
  case OPCODE_AND: {
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    u64 rhs = *machine_reg(machine, GET_OPERAND2(inst));
    machine->reg_status.numeric = 0;
    u64 result = lhs & rhs;
    result = mask_val_and_set_flag_n(machine, result, oplen);
    machine->reg_status.flag_z = result == 0;
    *dest = result;
  } break;
  case OPCODE_OR: {
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    u64 rhs = *machine_reg(machine, GET_OPERAND2(inst));
    machine->reg_status.numeric = 0;
    u64 result = lhs | rhs;
    result = mask_val_and_set_flag_n(machine, result, oplen);
    *dest = result;
  } break;
  case OPCODE_XOR: {
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    u64 rhs = *machine_reg(machine, GET_OPERAND2(inst));
    machine->reg_status.numeric = 0;
    u64 result = lhs ^ rhs;
    result = mask_val_and_set_flag_n(machine, result, oplen);
    machine->reg_status.flag_z = result == 0;
    *dest = result;
  } break;
  case OPCODE_NOT: {
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    machine->reg_status.numeric = 0;
    u64 result = ~lhs;
    result = mask_val_and_set_flag_n(machine, result, oplen);
    machine->reg_status.flag_z = result == 0;
    *dest = result;
  } break;
  case OPCODE_MULADD: {
    // dest = lhs * rhs + rhs2
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    u64 rhs = *machine_reg(machine, GET_OPERAND2(inst));
    u64 rhs2 = *machine_reg(machine, GET_OPERAND3(inst));
    u64 result;
#define MULADD_WITH_TY(TY, SIGNED_TY)                                                                                  \
  {                                                                                                                    \
    TY LHS_ = (TY)lhs;                                                                                                 \
    TY RHS_ = (TY)rhs;                                                                                                 \
    TY RHS2_ = (TY)rhs2;                                                                                               \
    TY RESULT_ = LHS_ * RHS_;                                                                                          \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_v = (RESULT_ < LHS_) | (RESULT_ < RHS_);                                                  \
    RESULT_ += RHS2_;                                                                                                  \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = (SIGNED_TY)RESULT_ < 0;                                                               \
    machine->reg_status.flag_v |= (RESULT_ < LHS_) | (RESULT_ < RHS_);                                                 \
    result = RESULT_;                                                                                                  \
  };
    switch (oplen) {
    case OPLEN_8: {
      MULADD_WITH_TY(u64, i64);
    } break;
    case OPLEN_4: {
      MULADD_WITH_TY(u32, i32);
    } break;
    case OPLEN_2: {
      MULADD_WITH_TY(u16, i16);
    } break;
    case OPLEN_1: {
      MULADD_WITH_TY(u8, i8);
    } break;
    default:
      PANIC();
    }
    *dest = result;

  } break;
  case OPCODE_CALL: {
    if (machine->reg_sp + 1 >= PC_INIT) {
      fprintf(stderr, "Stack overflowed @ %04X\n", machine->pc - 4);
      return false;
    }
    memcpy(&machine->vmem[machine->reg_sp], &machine->pc, 2);
    machine->reg_sp += 2;
    TRY_NULL(machine_jump_offset(machine, GET_JUMP_OFFSET(inst)));
  } break;
  case OPCODE_CCALL: {
    u8 cond_flag = GET_FLAGS(inst);
    u8 rev = cond_flag & 0b10000000;
    bool cond = (u64)(cond_flag & 0b011111111) & machine->reg_status.numeric;
    if (rev)
      cond = !cond;
    if (cond) {
      if (machine->reg_sp + 1 >= PC_INIT) {
        fprintf(stderr, "Stack overflowed @ %04X\n", machine->pc - 4);
        return false;
      }
      memcpy(&machine->vmem[machine->reg_sp], &machine->pc, 2);
      machine->reg_sp += 2;
      TRY_NULL(machine_jump_offset(machine, GET_JUMP_OFFSET(inst)));
    }
  } break;
  case OPCODE_RET: {
    if (machine->reg_sp < 2) {
      fprintf(stderr, "Stack underflowed @ %04X\n", machine->pc - 4);
      return false;
    }
    machine->reg_sp -= 2;
    memcpy(&machine->pc, &machine->vmem[machine->reg_sp], 2);
  } break;
#define machine_next_PUSH(SIZE)                                                                                        \
  {                                                                                                                    \
    if (machine->reg_sp + SIZE - 1 >= PC_INIT) {                                                                       \
      fprintf(stderr, "Stack overflowed @ %04X\n", machine->pc - 4);                                                   \
      return false;                                                                                                    \
    }                                                                                                                  \
    memcpy(&machine->vmem[machine->reg_sp], machine_reg(machine, GET_OPERAND0(inst)), SIZE);                           \
    machine->reg_sp += SIZE;                                                                                           \
  }
  case OPCODE_PUSH + OPLEN_8: {
    switch (oplen) {
    case OPLEN_8: {
      machine_next_PUSH(8);
    } break;
    case OPLEN_4: {
      machine_next_PUSH(4);
    } break;
    case OPLEN_2: {
      machine_next_PUSH(2);
    } break;
    case OPLEN_1: {
      machine_next_PUSH(1);
    } break;
    }
  } break;
#define machine_next_POP(TY, SIGNED_TY)                                                                                \
  {                                                                                                                    \
    if (machine->reg_sp < sizeof(TY)) {                                                                                \
      fprintf(stderr, "Stack underflowed @ %04X\n", machine->pc - 4);                                                  \
      return false;                                                                                                    \
    }                                                                                                                  \
    machine->reg_sp -= sizeof(TY);                                                                                     \
    TY value;                                                                                                          \
    memcpy(&value, &machine->vmem[machine->reg_sp], sizeof(TY));                                                       \
    *machine_reg(machine, GET_OPERAND0(inst)) = value;                                                                 \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = value == 0;                                                                           \
    machine->reg_status.flag_n = (SIGNED_TY)value < 0;                                                                 \
  }
  case OPCODE_POP: {
    switch (oplen) {
    case OPLEN_8: {
      machine_next_POP(u64, i64);
    } break;
    case OPLEN_4: {
      machine_next_POP(u32, i32);
    } break;
    case OPLEN_2: {
      machine_next_POP(u16, i16);
    } break;
    case OPLEN_1: {
      machine_next_POP(u8, i8);
    } break;
    }
  } break;
  case OPCODE_LIBC_CALL: {
    return machine_libc_call(machine, GET_FLAGS(inst));
  } break;
  case OPCODE_NATIVE_CALL: {
    TODO();
  } break;
  case OPCODE_BREAKPOINT: {
    if (machine->breakpoint_callback != NULL) {
      (machine->breakpoint_callback)(machine);
    }
  } break;
  default:
    fprintf(stderr, "Illegal instruction @ 0x%04X (note: illegal opcode 0x%02X)\n", machine->pc - 4, inst[1]);
    return false;
  }
  return true;
}
