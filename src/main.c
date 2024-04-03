#include "common.h"
#include "debug_utils.h"
#include "machine.h"
#include "values.h"

#include "../code.h"

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

i32 main(int argc, char **argv) {
  lbvm_check_platform_compatibility();

  bool dbg = false;
  if (argc >= 2) {
    if (strcmp(argv[1], "--dbg") == 0) {
      dbg = true;
    }
  }

  Machine machine = machine_new(!dbg, breakpoint_callback, NULL);
  machine_load_program(&machine, text_segment, sizeof(text_segment), data_segment, sizeof(data_segment));
  while (machine_next(&machine))
    ;
  if (dbg)
    breakpoint_callback(&machine);
}
