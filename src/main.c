#include "common.h"
#include "debug_utils.h"
#include "machine.h"
#include "values.h"

void write_small_inst(u8 **writer, u8 opcode, u8 operand0, u8 operand1, u8 operand2, u8 operand3, u8 flags) {
  const u8 operand0_1 = (u8)(operand0 | (operand1 << 4));
  const u8 operand2_3 = (u8)(operand2 | (operand3 << 4));
  **writer = opcode;
  *writer += 1;
  **writer = operand0_1;
  *writer += 1;
  **writer = operand2_3;
  *writer += 1;
  **writer = flags;
  *writer += 1;
}

void write_big_inst(u8 **writer, u8 opcode, u8 operand0, u8 operand1, u8 operand2, u8 operand3, u8 flags, u64 data) {
  const u8 operand0_1 = (u8)(operand0 | (operand1 << 4));
  const u8 operand2_3 = (u8)(operand2 | (operand3 << 4));
  **writer = opcode;
  *writer += 1;
  **writer = operand0_1;
  *writer += 1;
  **writer = operand2_3;
  *writer += 1;
  **writer = flags;
  *writer += 1;
  memcpy(*writer, &data, 8);
  *writer += 8;
}

void breakpoint_callback(Machine *machine) {
  printf("--- BREAKPOINT ---\n");
  machine_print_regs(machine);
  for (size_t i = 0; i < 8; i++) {
    for (size_t j = 0; j < 8; j++) {
      printf("%02X ", machine->vmem[i * 8 + j]);
    }
    printf("\n");
  }
  getchar();
}

i32 main() {
  lbvm_check_platform_compatibility();

  u8 memory[VMEM_SIZE] = {0};

  u8 *writer = &memory[PC_INIT];
  write_big_inst(&writer, OPCODE_LOAD_IMM + OPLEN_8, REG_1, 0, 0, 0, 0, 0x0123456789ABCDEF);
  write_big_inst(&writer, OPCODE_LOAD_IMM + OPLEN_8, REG_2, 0, 0, 0, 0, 0xF0F0F0F0F0F0F0F0);
  write_big_inst(&writer, OPCODE_LOAD_IMM + OPLEN_8, REG_3, 0, 0, 0, 0, 11);
  write_big_inst(&writer, OPCODE_LOAD_IMM + OPLEN_8, REG_4, 0, 0, 0, 0, 10);
  write_small_inst(&writer, OPCODE_BREAKPOINT, 0, 0, 0, 0, 0);
  write_small_inst(&writer, OPCODE_CMP + OPLEN_8, REG_3, REG_4, 0, 0, 0);
  write_small_inst(&writer, OPCODE_BREAKPOINT, 0, 0, 0, 0, 0);
  write_small_inst(&writer, OPCODE_CSEL + OPLEN_8, REG_0, REG_1, REG_2, 0, CONDFLAG_G);
  write_small_inst(&writer, OPCODE_BREAKPOINT, 0, 0, 0, 0, 0);
  write_small_inst(&writer, OPCODE_PUSH + OPLEN_8, REG_0, 0, 0, 0, 0);
  write_small_inst(&writer, OPCODE_BREAKPOINT, 0, 0, 0, 0, 0);
  write_small_inst(&writer, OPCODE_POP + OPLEN_4, REG_12, 0, 0, 0, 0);
  write_small_inst(&writer, OPCODE_BREAKPOINT, 0, 0, 0, 0, 0);
  write_small_inst(&writer, OPCODE_POP + OPLEN_4, REG_13, 0, 0, 0, 0);
  write_small_inst(&writer, OPCODE_BREAKPOINT, 0, 0, 0, 0, 0);

  Machine machine = {0};
  machine.breakpoint_callback = &breakpoint_callback;
  machine.vmem = memory;
  machine.pc = PC_INIT;
  while (machine_next(&machine))
    ;
}
