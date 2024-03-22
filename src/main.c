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

void write_jump_inst(u8 **writer, u8 opcode, u16 offset, u8 flags) {
  **writer = opcode;
  *writer += 1;
  memcpy(*writer, &offset, 2);
  *writer += 2;
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

  static const char s[] = "hello, world\n";

  u8 memory[VMEM_SIZE] = {0};
  u8 *writer = &memory[PC_INIT];
  write_big_inst(&writer, OPCODE_LOAD_IMM, REG_0, 0, 0, 0, 0, (u64)&s);
  write_small_inst(&writer, OPCODE_LIBC_CALL, 0, 0, 0, 0, LIBC_printf);
  Machine machine = {0};
  machine.breakpoint_callback = &breakpoint_callback;
  machine.vmem = memory;
  machine.pc = PC_INIT;
  while (machine_next(&machine))
    ;
}
