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

static inline void machine_print_regs(const Machine *machine) {
  printf("pc:\t0x%04X\n", machine->pc);
  printf("r0:\t0x%016llX (%llu)\n", machine->reg_0, machine->reg_0);
  printf("r1:\t0x%016llX (%llu)\n", machine->reg_1, machine->reg_1);
  printf("r2:\t0x%016llX (%llu)\n", machine->reg_2, machine->reg_2);
  printf("r3:\t0x%016llX (%llu)\n", machine->reg_3, machine->reg_3);
  printf("r4:\t0x%016llX (%llu)\n", machine->reg_4, machine->reg_4);
  printf("r5:\t0x%016llX (%llu)\n", machine->reg_5, machine->reg_5);
  printf("r6:\t0x%016llX (%llu)\n", machine->reg_6, machine->reg_6);
  printf("r7:\t0x%016llX (%llu)\n", machine->reg_7, machine->reg_7);
  printf("r8:\t0x%016llX (%llu)\n", machine->reg_8, machine->reg_8);
  printf("r9:\t0x%016llX (%llu)\n", machine->reg_9, machine->reg_9);
  printf("r10:\t0x%016llX (%llu)\n", machine->reg_10, machine->reg_10);
  printf("r11:\t0x%016llX (%llu)\n", machine->reg_11, machine->reg_11);
  printf("r12:\t0x%016llX (%llu)\n", machine->reg_12, machine->reg_12);
  printf("r13:\t0x%016llX (%llu)\n", machine->reg_13, machine->reg_13);
  printf("status:\t%c %c %c %c %c %c %c\n", machine->reg_status.flag_n ? 'N' : 'n',
         machine->reg_status.flag_z ? 'Z' : 'z', machine->reg_status.flag_c ? 'C' : 'c',
         machine->reg_status.flag_v ? 'V' : 'v', machine->reg_status.flag_e ? 'E' : 'e',
         machine->reg_status.flag_g ? 'G' : 'g', machine->reg_status.flag_l ? 'L' : 'l');
  printf("sp:\t0x%016llX (%llu)\n", machine->reg_sp, machine->reg_sp);
}

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

/// Fetch the 8 data bytes on pc for big instructions.
static inline u64 machine_fetch_data_qword(Machine *machine) {
  u64 value;
  memcpy(&value, &machine->vmem[machine->pc], sizeof(u64));
  machine->pc += sizeof(u64);
  return value;
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
      fprintf(stderr, "Out of bound stack access @ 0x%04X\n", machine->pc - 12);
      exit(1);
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
    PANIC();
  }
  __builtin_unreachable();
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
    PANIC();
  }
  __builtin_unreachable();
}

#define GET_OPERAND0(INST) ((INST)[1] & 0b00001111)
#define GET_OPERAND1(INST) (((INST)[1] & 0b11110000) >> 4)
#define GET_OPERAND2(INST) ((INST)[2] & 0b00001111)
#define GET_OPERAND3(INST) (((INST)[2] & 0b11110000) >> 4)
#define GET_FLAGS(INST) ((INST)[3])
#define GET_JUMP_OFFSET(INST) (((u16)((INST)[1])) | (u16)((INST)[2] << 8))

#define IS_NEGATIVE(X)                                                                                                 \
  _Generic((X), u8 : (i8)(X) < 0, u16 : (i16)(X) < 0, u32 : (i32)(X) < 0, u64 : (i64)(X) < 0, usize : (isize)(X) < 0)

//// Returns `true` if should continue, `false` if should stop.
static inline bool machine_next(Machine *machine) {
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
    u64 imm = machine_fetch_data_qword(machine);
    u64 imm_masked = mask_val_and_set_flag_n(machine, imm, oplen);
    machine->reg_status.flag_z = imm_masked == 0;
    *dest_reg = imm_masked;
  } break;
  case OPCODE_LOAD_DIR: {
    machine->reg_status.numeric = 0;
    u64 *dest_reg = machine_reg(machine, GET_OPERAND0(inst));
    u64 addr = machine_fetch_data_qword(machine);
    void *src = solve_addr(machine, GET_FLAGS(inst) & 0b00000001, addr);
    *dest_reg = 0;
    memcpy(dest_reg, src, oplen_to_size(oplen)); // use memcpy because address may be unaligned
    mask_val_and_set_flag_n(machine, *dest_reg, oplen);
    machine->reg_status.flag_z = src == 0;
  } break;
  case OPCODE_LOAD_IND: {
    machine->reg_status.numeric = 0;
    u64 src_addr_offset = machine_fetch_data_qword(machine);
    u64 src_addr_base = *machine_reg(machine, GET_OPERAND1(inst));
    u64 src_addr = src_addr_base + src_addr_offset;
    void *src = solve_addr(machine, GET_FLAGS(inst) & 0b00000001, src_addr);
    u64 *dest_reg = machine_reg(machine, GET_OPERAND0(inst));
    *dest_reg = 0;
    memcpy(dest_reg, src, oplen_to_size(oplen)); // use memcpy because address may be unaligned
    mask_val_and_set_flag_n(machine, *dest_reg, oplen);
    machine->reg_status.flag_z = src == 0;
  } break;
  case OPCODE_STORE_IMM: {
    machine->reg_status.numeric = 0;
    u64 dest_addr = machine_fetch_data_qword(machine);
    void *dest = solve_addr(machine, GET_FLAGS(inst) & 0b00000001, dest_addr);
    u64 src = mask_val_and_set_flag_n(machine, *machine_reg(machine, GET_OPERAND0(inst)), oplen);
    memcpy(dest, &src, oplen_to_size(oplen));
    mask_val_and_set_flag_n(machine, src, oplen);
    machine->reg_status.flag_z = src == 0;
  } break;
  case OPCODE_STORE_DIR: {
    machine->reg_status.numeric = 0;
    u64 dest_addr = *machine_reg(machine, GET_OPERAND1(inst));
    void *dest = solve_addr(machine, GET_FLAGS(inst) & 0b00000001, dest_addr);
    u64 src = mask_val_and_set_flag_n(machine, *machine_reg(machine, GET_OPERAND0(inst)), oplen);
    memcpy(dest, &src, oplen_to_size(oplen));
    mask_val_and_set_flag_n(machine, src, oplen);
    machine->reg_status.flag_z = src == 0;
  } break;
  case OPCODE_STORE_IND: {
    machine->reg_status.numeric = 0;
    u64 dest_addr_base = *machine_reg(machine, GET_OPERAND0(inst));
    u64 dest_addr_offset = machine_fetch_data_qword(machine);
    u64 dest_addr = dest_addr_base + dest_addr_offset;
    void *dest = solve_addr(machine, GET_FLAGS(inst) & 0b00000001, dest_addr);
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
    if (cond)
      machine->pc += GET_JUMP_OFFSET(inst);
  } break;
  case OPCODE_J: {
    machine->pc += GET_JUMP_OFFSET(inst);
  } break;
  case OPCODE_ADD: {
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    u64 rhs = *machine_reg(machine, GET_OPERAND2(inst));
    u64 result;
#define ADD_WITH_TY(TY)                                                                                                \
  {                                                                                                                    \
    TY LHS_ = (TY)lhs;                                                                                                 \
    TY RHS_ = (TY)rhs;                                                                                                 \
    TY RESULT_ = LHS_ + RHS_;                                                                                          \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = IS_NEGATIVE(RESULT_);                                                                 \
    machine->reg_status.flag_c = (RESULT_ < LHS_) | (RESULT_ < RHS_);                                                  \
    machine->reg_status.flag_v = (RESULT_ < LHS_) | (RESULT_ < RHS_);                                                  \
    result = RESULT_;                                                                                                  \
  };
    switch (oplen) {
    case OPLEN_8: {
      ADD_WITH_TY(u64);
    } break;
    case OPLEN_4: {
      ADD_WITH_TY(u32);
    } break;
    case OPLEN_2: {
      ADD_WITH_TY(u16);
    } break;
    case OPLEN_1: {
      ADD_WITH_TY(u8);
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
#define SUB_WITH_TY(TY)                                                                                                \
  {                                                                                                                    \
    TY LHS_ = (TY)lhs;                                                                                                 \
    TY RHS_ = (TY)rhs;                                                                                                 \
    TY RESULT_ = LHS_ - RHS_;                                                                                          \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = IS_NEGATIVE(RESULT_);                                                                 \
    machine->reg_status.flag_c = (RESULT_ < LHS_) | (RESULT_ < RHS_); /*carry flag is reversed for subtraction*/       \
    machine->reg_status.flag_v = (RESULT_ > LHS_) | (RESULT_ > RHS_);                                                  \
    result = RESULT_;                                                                                                  \
  };
    switch (oplen) {
    case OPLEN_8: {
      SUB_WITH_TY(u64);
    } break;
    case OPLEN_4: {
      SUB_WITH_TY(u32);
    } break;
    case OPLEN_2: {
      SUB_WITH_TY(u16);
    } break;
    case OPLEN_1: {
      SUB_WITH_TY(u8);
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
#define MUL_WITH_TY(TY)                                                                                                \
  {                                                                                                                    \
    TY LHS_ = (TY)lhs;                                                                                                 \
    TY RHS_ = (TY)rhs;                                                                                                 \
    TY RESULT_ = LHS_ * RHS_;                                                                                          \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = IS_NEGATIVE(RESULT_);                                                                 \
    machine->reg_status.flag_v = (RESULT_ < LHS_) | (RESULT_ < RHS_);                                                  \
    result = RESULT_;                                                                                                  \
  };
    switch (oplen) {
    case OPLEN_8: {
      MUL_WITH_TY(u64);
    } break;
    case OPLEN_4: {
      MUL_WITH_TY(u32);
    } break;
    case OPLEN_2: {
      MUL_WITH_TY(u16);
    } break;
    case OPLEN_1: {
      MUL_WITH_TY(u8);
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
#define DIV_WITH_TY(TY)                                                                                                \
  {                                                                                                                    \
    TY LHS_ = (TY)lhs;                                                                                                 \
    TY RHS_ = (TY)rhs;                                                                                                 \
    TY RESULT_ = LHS_ / RHS_;                                                                                          \
    if (RHS_ == 0)                                                                                                     \
      fprintf(stderr, "Division by zero @ 0x%04X\n", machine->pc - 4);                                                 \
    return false;                                                                                                      \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = IS_NEGATIVE(RESULT_);                                                                 \
    result = RESULT_;                                                                                                  \
  };
    switch (oplen) {
    case OPLEN_8: {
      DIV_WITH_TY(u64);
    } break;
    case OPLEN_4: {
      DIV_WITH_TY(u32);
    } break;
    case OPLEN_2: {
      DIV_WITH_TY(u16);
    } break;
    case OPLEN_1: {
      DIV_WITH_TY(u8);
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
      IADD_WITH_TY(u16);
    } break;
    case OPLEN_1: {
      IADD_WITH_TY(u8);
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
    if (RHS_ == 0) {                                                                                                   \
      fprintf(stderr, "Division by zero @ %04X\n", machine->pc - 4);                                                   \
      return false;                                                                                                    \
    }                                                                                                                  \
    TY RESULT_ = LHS_ / RHS_;                                                                                          \
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
  case OPCODE_FADD: {
    u64 *dest = machine_reg(machine, GET_OPERAND0(inst));
    u64 lhs = *machine_reg(machine, GET_OPERAND1(inst));
    u64 rhs = *machine_reg(machine, GET_OPERAND2(inst));
    u64 result;
#define FADD_WITH_TY(TY)                                                                                               \
  {                                                                                                                    \
    TY LHS_ = (TY)lhs;                                                                                                 \
    TY RHS_ = (TY)rhs;                                                                                                 \
    TY RESULT_ = LHS_ + RHS_;                                                                                          \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = RESULT_ < 0;                                                                          \
    result = *((u64 *)&RESULT_);                                                                                       \
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
      fprintf(stderr, "Illegal instruction @ 0x%04X (note: floating point operations cannot only be qword or dword)\n",
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
    TY LHS_ = (TY)lhs;                                                                                                 \
    TY RHS_ = (TY)rhs;                                                                                                 \
    TY RESULT_ = LHS_ - RHS_;                                                                                          \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = RESULT_ < 0;                                                                          \
    result = *((u64 *)&RESULT_);                                                                                       \
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
      fprintf(stderr, "Illegal instruction @ 0x%04X (note: floating point operations cannot only be qword or dword)\n",
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
    TY LHS_ = (TY)lhs;                                                                                                 \
    TY RHS_ = (TY)rhs;                                                                                                 \
    TY RESULT_ = LHS_ * RHS_;                                                                                          \
    machine->reg_status.numeric = 0;                                                                                   \
    machine->reg_status.flag_z = RESULT_ == 0;                                                                         \
    machine->reg_status.flag_n = RESULT_ < 0;                                                                          \
    result = *((u64 *)&RESULT_);                                                                                       \
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
      fprintf(stderr, "Illegal instruction @ 0x%04X (note: floating point operations cannot only be qword or dword)\n",
              machine->pc - 4);
      return false;
    } break;
    default:
      PANIC();
    }
    *dest = result;
  } break;
  case OPCODE_FDIV: {
    TODO();
  } break;
  case OPCODE_FMOD: {
    TODO();
  } break;
  case OPCODE_AND: {
    TODO();
  } break;
  case OPCODE_OR: {
    TODO();
  } break;
  case OPCODE_XOR: {
    TODO();
  } break;
  case OPCODE_NOT: {
    TODO();
  } break;
  case OPCODE_MULADD: {
    TODO();
  } break;
  case OPCODE_CALL: {
    if (machine->reg_sp + 1 >= PC_INIT) {
      fprintf(stderr, "Stack overflowed @ %04X\n", machine->pc - 4);
      return false;
    }
    memcpy(&machine->vmem[machine->reg_sp], &machine->pc, 2);
    machine->reg_sp += 2;
    machine->pc += GET_JUMP_OFFSET(inst);
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
      machine->pc += GET_JUMP_OFFSET(inst);
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
    TODO();
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
