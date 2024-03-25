#pragma once

#define VMEM_SIZE 65536 // 64 kB
#define PC_INIT 32768

#define OPLEN_8 0b00000000
#define OPLEN_4 0b00000001
#define OPLEN_2 0b00000010
#define OPLEN_1 0b00000011

#define REG_0      0
#define REG_1      1
#define REG_2      2
#define REG_3      3
#define REG_4      4
#define REG_5      5
#define REG_6      6
#define REG_7      7
#define REG_8      8
#define REG_9      9
#define REG_10     10
#define REG_11     11
#define REG_12     12
#define REG_13     13
#define REG_STATUS 14
#define REG_SP     15

#define LIBC_exit       255
#define LIBC_malloc     1
#define LIBC_realloc    2
#define LIBC_free       3
#define LIBC_fwrite     4
#define LIBC_fread      5
#define LIBC_printf     6
#define LIBC_fprintf    7
#define LIBC_scanf      8
#define LIBC_fscanf     9
#define LIBC_puts       10
#define LIBC_fputs      11
#define LIBC_snprintf   12
#define LIBC_fopen      13
#define LIBC_fclose     14
#define LIBC_memcpy     15
#define LIBC_memmove    16
#define LIBC_memset     17
#define LIBC_bzero      18
#define LIBC_strlen     19
#define LIBC_strcpy     20
#define LIBC_strcat     21
#define LIBC_strcmp     22

#define CONDFLAG_N     0b00000001
#define CONDFLAG_Z     0b00000010
#define CONDFLAG_C     0b00000100
#define CONDFLAG_V     0b00001000
#define CONDFLAG_E     0b00010000
#define CONDFLAG_G     0b00100000
#define CONDFLAG_L     0b01000000
#define CONDFLAG_GE    (CONDFLAG_G  | CONDFLAG_E)
#define CONDFLAG_LE    (CONDFLAG_L  | CONDFLAG_E)
#define CONDFLAG_NN    (CONDFLAG_N  | 0b10000000)
#define CONDFLAG_NZ    (CONDFLAG_Z  | 0b10000000)
#define CONDFLAG_NC    (CONDFLAG_C  | 0b10000000)
#define CONDFLAG_NV    (CONDFLAG_V  | 0b10000000)
#define CONDFLAG_NE    (CONDFLAG_E  | 0b10000000)
#define CONDFLAG_NG    (CONDFLAG_G  | 0b10000000)
#define CONDFLAG_NL    (CONDFLAG_L  | 0b10000000)
#define CONDFLAG_NGE   (CONDFLAG_GE | 0b10000000)
#define CONDFLAG_NLE   (CONDFLAG_LE | 0b10000000)

#define OPCODE(N) (((N) << 2) & 0b11111100)

#define OPCODE_BRK         OPCODE(0)   // INTERRUPT
#define OPCODE_CBRK        OPCODE(1)
#define OPCODE_NOP         OPCODE(2)   // NOP
#define OPCODE_LOAD_IMM    OPCODE(3)   // MEMORY
#define OPCODE_LOAD_DIR    OPCODE(4)
#define OPCODE_LOAD_IND    OPCODE(5)
#define OPCODE_STORE_IMM   OPCODE(6)
#define OPCODE_STORE_DIR   OPCODE(7)
#define OPCODE_STORE_IND   OPCODE(8)
#define OPCODE_MOV         OPCODE(9)
#define OPCODE_CMP         OPCODE(10)   // BRANCHING
#define OPCODE_CSEL        OPCODE(11)
#define OPCODE_B           OPCODE(12)
#define OPCODE_J           OPCODE(13)
#define OPCODE_ADD         OPCODE(14)  // ARITHMETICS
#define OPCODE_SUB         OPCODE(15)
#define OPCODE_MUL         OPCODE(16)
#define OPCODE_DIV         OPCODE(17)
#define OPCODE_MOD         OPCODE(18)
#define OPCODE_IADD        OPCODE(19)
#define OPCODE_ISUB        OPCODE(20)
#define OPCODE_IMUL        OPCODE(21)
#define OPCODE_IDIV        OPCODE(22)
#define OPCODE_IMOD        OPCODE(23)
#define OPCODE_FADD        OPCODE(24)
#define OPCODE_FSUB        OPCODE(25)
#define OPCODE_FMUL        OPCODE(26)
#define OPCODE_FDIV        OPCODE(27)
#define OPCODE_FMOD        OPCODE(28)
#define OPCODE_AND         OPCODE(29)
#define OPCODE_OR          OPCODE(30)
#define OPCODE_XOR         OPCODE(31)
#define OPCODE_NOT         OPCODE(32)
#define OPCODE_MULADD      OPCODE(33)
#define OPCODE_CALL        OPCODE(34)  // FUNCTIONS/STACK
#define OPCODE_CCALL       OPCODE(35)
#define OPCODE_RET         OPCODE(36)
#define OPCODE_PUSH        OPCODE(37)
#define OPCODE_POP         OPCODE(38)
#define OPCODE_LIBC_CALL   OPCODE(39)  // VIRTUAL
#define OPCODE_NATIVE_CALL OPCODE(40)
#define OPCODE_BREAKPOINT  0b11111100
