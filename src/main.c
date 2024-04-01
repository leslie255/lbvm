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

void print_char_with_escape(char c) {
  switch (c) {
  case '\0':
    printf("'\\0'");
    break;
  case '\a':
    printf("'\\a'");
    break;
  case '\b':
    printf("'\\b'");
    break;
  case '\e':
    printf("'\\e'");
    break;
  case '\f':
    printf("'\\f'");
    break;
  case '\n':
    printf("'\\n'");
    break;
  case '\r':
    printf("'\\r'");
    break;
  case '\t':
    printf("'\\t'");
    break;
  case '\v':
    printf("'\\v'");
    break;
  case '\\':
    printf("'\\\\'");
    break;
  case '\'':
    printf("'\\\''");
    break;
  case '\"':
    printf("'\\\"'");
    break;
  default:
    if (c >= 32 && c < 127) {
      printf("'%c'", c);
    } else {
      printf("_");
    }
  }
}

void breakpoint_callback(Machine *machine) {
  printf("--- BREAKPOINT ---\n");
  printf("pc:\t0x%04X\n", machine->pc);
  printf("r0:\t0x%016llX (%llu, %lf, ", machine->reg_0, machine->reg_0, TRANSMUTE(f64, machine->reg_0));
  print_char_with_escape((char)machine->reg_0);
  printf(")\n");
  printf("r1:\t0x%016llX (%llu, %lf, ", machine->reg_1, machine->reg_1, TRANSMUTE(f64, machine->reg_1));
  print_char_with_escape((char)machine->reg_1);
  printf(")\n");
  printf("r2:\t0x%016llX (%llu, %lf, ", machine->reg_2, machine->reg_2, TRANSMUTE(f64, machine->reg_2));
  print_char_with_escape((char)machine->reg_2);
  printf(")\n");
  printf("r3:\t0x%016llX (%llu, %lf, ", machine->reg_3, machine->reg_3, TRANSMUTE(f64, machine->reg_3));
  print_char_with_escape((char)machine->reg_3);
  printf(")\n");
  printf("r4:\t0x%016llX (%llu, %lf, ", machine->reg_4, machine->reg_4, TRANSMUTE(f64, machine->reg_4));
  print_char_with_escape((char)machine->reg_4);
  printf(")\n");
  printf("r5:\t0x%016llX (%llu, %lf, ", machine->reg_5, machine->reg_5, TRANSMUTE(f64, machine->reg_5));
  print_char_with_escape((char)machine->reg_5);
  printf(")\n");
  printf("r6:\t0x%016llX (%llu, %lf, ", machine->reg_6, machine->reg_6, TRANSMUTE(f64, machine->reg_6));
  print_char_with_escape((char)machine->reg_6);
  printf(")\n");
  printf("r7:\t0x%016llX (%llu, %lf, ", machine->reg_7, machine->reg_7, TRANSMUTE(f64, machine->reg_7));
  print_char_with_escape((char)machine->reg_7);
  printf(")\n");
  printf("r8:\t0x%016llX (%llu, %lf, ", machine->reg_8, machine->reg_8, TRANSMUTE(f64, machine->reg_8));
  print_char_with_escape((char)machine->reg_8);
  printf(")\n");
  printf("r9:\t0x%016llX (%llu, %lf, ", machine->reg_9, machine->reg_9, TRANSMUTE(f64, machine->reg_9));
  print_char_with_escape((char)machine->reg_9);
  printf(")\n");
  printf("r10:\t0x%016llX (%llu, %lf, ", machine->reg_10, machine->reg_10, TRANSMUTE(f64, machine->reg_10));
  print_char_with_escape((char)machine->reg_10);
  printf(")\n");
  printf("r11:\t0x%016llX (%llu, %lf, ", machine->reg_11, machine->reg_11, TRANSMUTE(f64, machine->reg_11));
  print_char_with_escape((char)machine->reg_11);
  printf(")\n");
  printf("r12:\t0x%016llX (%llu, %lf, ", machine->reg_12, machine->reg_12, TRANSMUTE(f64, machine->reg_12));
  print_char_with_escape((char)machine->reg_12);
  printf(")\n");
  printf("r13:\t0x%016llX (%llu, %lf, ", machine->reg_13, machine->reg_13, TRANSMUTE(f64, machine->reg_13));
  print_char_with_escape((char)machine->reg_13);
  printf(")\n");
  printf("status:\t%c %c %c %c %c %c %c (0x%016llX)\n", machine->reg_status.flag_n ? 'N' : 'n',
         machine->reg_status.flag_z ? 'Z' : 'z', machine->reg_status.flag_c ? 'C' : 'c',
         machine->reg_status.flag_v ? 'V' : 'v', machine->reg_status.flag_e ? 'E' : 'e',
         machine->reg_status.flag_g ? 'G' : 'g', machine->reg_status.flag_l ? 'L' : 'l', machine->reg_status.numeric);
  printf("sp:\t0x%016llX (%llu)\n", machine->reg_sp, machine->reg_sp);
  for (size_t i = 0; i < 8; i++) {
    for (size_t j = 0; j < 8; j++) {
      printf("%02X ", machine->vmem_stack[i * 8 + j]);
    }
    printf("\n");
  }
  getchar();
}

i32 main() {
  lbvm_check_platform_compatibility();

  static const char vowel[] = "Vowel";
  static const char not_vowel[] = "Not vowel";
  static const char my_email[] = "zili.luo@student.manchester.ac.uk";

  int n;
  printf("Enter an integer: ");
  scanf("%d", &n);

  static const u8 code[] = {
      60, 208, 12,  0,  19,  0, 0,  1,   15, 4,  0,  0,   16,  0,  0,   0,   0,  0,   0,   0,  15,  3,   0,  0,   97,
      0,  0,   0,   0,  0,   0, 0,  43,  48, 0,  0,  39,  225, 0,  0,   139, 17, 4,   0,   15, 3,   0,   0,  101, 0,
      0,  0,   0,   0,  0,   0, 43, 48,  0,  0,  39, 226, 0,   0,  139, 34,  4,  0,   143, 17, 2,   0,   15, 3,   0,
      0,  105, 0,   0,  0,   0, 0,  0,   0,  43, 48, 0,   0,   39, 226, 0,   0,  139, 34,  4,  0,   143, 17, 2,   0,
      15, 3,   0,   0,  111, 0, 0,  0,   0,  0,  0,  0,   43,  48, 0,   0,   39, 226, 0,   0,  139, 34,  4,  0,   143,
      17, 2,   0,   15, 3,   0, 0,  117, 0,  0,  0,  0,   0,   0,  0,   43,  48, 0,   0,   39, 226, 0,   0,  139, 34,
      4,  0,   143, 17, 2,   0, 43, 17,  0,  0,  48, 144, 10,  2,  176, 0,   0,  6,   0,   0,  0,   0,
  };

  u8 vmem_text[VMEM_SEG_SIZE] = {0};
  u8 vmem_data[VMEM_SEG_SIZE] = {0};
  u8 vmem_stack[VMEM_SEG_SIZE] = {0};
  memcpy(vmem_text, code, sizeof(code));
  Machine machine = {0};
  machine.config_silent = true;
  machine.breakpoint_callback = &breakpoint_callback;
  machine.vmem_stack = vmem_stack;
  machine.vmem_text = vmem_text;
  machine.vmem_data = vmem_data;
  machine.reg_13 = (u64)&my_email[0];
  machine.reg_12 = (u64)n;
  machine.reg_11 = (u64)sizeof(my_email);
  machine.reg_10 = (u64)&vowel[0];
  machine.reg_9 = (u64)&not_vowel[0];
  while (machine_next(&machine))
    ;
}
