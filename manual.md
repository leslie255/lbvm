# LBVM Manual

Note that LBVM is not designed with memory security or safety in mind, hence some of the design choices.

LBVM is little endian.

## Registers

LBVM has 16 registers of 64 bits, encoded as 0~15.

All registers are callee-saved.

| Name   | Encoding | Special function? |
|--------|----------|-------------------|
| r0     | `0`      | No                |
| r1     | `1`      | No                |
| r2     | `2`      | No                |
| r3     | `3`      | No                |
| r4     | `4`      | No                |
| r5     | `5`      | No                |
| r6     | `6`      | No                |
| r7     | `7`      | No                |
| r8     | `8`      | No                |
| r9     | `9`      | No                |
| r10    | `10`     | No                |
| r11    | `11`     | No                |
| r12    | `12`     | No                |
| r13    | `13`     | No                |
| status | `14`     | Status register   |
| sp     | `15`     | Stack pointer     |

## General format for instructions

Small Instruction (4 bytes):

```
byte0: [opcode:6][oplen:2]
byte1: [reg1:4][reg0:4]
byte2: [reg3:4][reg2:4]
byte3: [flags:8]
```

Jump/Branch Instruction (4 bytes):

```
byte0:    [opcode:6][-]
byte1~2:  [offset:16]
byte3:    [flags:8]
```

Big Instruction (12 bytes):

```
byte0:    [opcode:6][oplen:2]
byte1:    [reg1:4][reg0:4]
byte2:    [reg3:4][reg2:4]
byte3:    [flags:8]
byte4~11: [data:64]
```

## Opcode & oplen

The first byte of instructions consists of a 6 bit opcode and a 2 bit oplen.

Oplen is the length associated with an operation:

| Name  | Size | Encoding            |
|-------|------|---------------------|
| qword | 8    | `0b00`              |
| dword | 4    | `0b01`              |
| word  | 2    | `0b10`              |
| byte  | 1    | `0b11`              |

Oplen is irrelevant for some operations, for those any oplen is allowed.

Floating point arithmetics only allow `qword` and `dword`, using them with `word` or `byte` halts machine.

## Status register and status flags

LBVM has 7 status flags, stored in the 7 least significant bits of the status register.

| Abbreviation | Meaning       | Encoding (Binary) |
|--------------|---------------|-------------------|
| `N`          | **N**egative  | `0b00000001`      |
| `Z`          | **Z**ero      | `0b00000010`      |
| `C`          | **C**arry     | `0b00000100`      |
| `V`          | O**v**erflow  | `0b00001000`      |
| `E`          | **E**qual     | `0b00010000`      |
| `G`          | **G**reater   | `0b00100000`      |
| `L`          | **L**ess-than | `0b01000000`      |

Despite of this, the status register is 64 bits for convenience sake.

Any instruction that sets a status flag clears all other irrelevant bits to zero.

Instructions that uses the status flags (e.g. `b`, `csel`) uses its 8 `flags` bits as the status flags.
The 7 least significant bits can be masked together for boolean OR logic, while the most significant bit is used as negation.

For example, the byte `0b10110000` (`0b10000000 & G & E`) encodes the condition of `!(greater | equal)`.

The status register can be directly addressed to using its encoding of `15` in similar fashion to other registers.
But upon using status-affecting instructions to change the status register (e.g. `load_imm` a value into status register),
the status register would be immediately overwrote within the same instruction.

## Memory

LBVM has a virtual memory `vmem` of 192kB, with 3 segments of 64kB.

The first segment (`vmem` address 0 ~ 0xFFFF) is used as the stack.
The second segment (`vmem` address 0x10000 ~ 0x1FFFF) is used as the text segment.
The third segment (`vmem` address 0x20000 ~ 0x2FFFF) is used as the data segment.

Their is no way for the machine to execute code outside of the text segment.

Memory of the host machine can also be accessed, depending on the `vmem` flag.

The load/store instructions takes in a `vmem` flag as the least significan bit in their `flags` byte.
The `vmem` flag determines whether the virtual or the host memory is accessed (`0` for vmem, `1` for real memory).

Pointers have sizes of 64 bits.

Access of `vmem` with out-of-bound pointers results in halting of machine.

Stack overflow/underflow is checked.

PC overflow/underflow is checked.

## Addressing modes

LBVM has three addressing modes for load/store instructions.

- `imm`: Immediate
- `dir`: Direct
- `ind`: Indirect (loads value on address of `reg + offset`, where `offset` is a 64-bit immediate)

Note that for `store` instructions, the addressing mode refers to the address of the destination address, unlike `load`, for which the addressing mode determines the location of the source value.

For this reason `store_dir` and `load_dir` are small instructions while the other load/store instructions are big instructions.

## Instruction set

| Name          | Opcode | Oplen relevant?  | Status affected | Encoding Fomat | Encoding (without first byte)     |
|---------------|--------|------------------|-----------------|----------------|-----------------------------------|
| `brk`         | 0      | No               | -               | Small          | `[-][-][-][-][-]`                 |
| `cbrk`        | 1      | No               | -               | Small          | `[-][-][-][-][cond]`              |
| `nop`         | 2      | No               | -               | Small          | `[-][-][-][-][-]`                 |
| `load_imm`    | 3      | Yes              | NZ              | Big            | `[dest][-][-][vmem][data]`        |
| `load_dir`    | 4      | Yes              | NZ              | Small          | `[dest][addr][-][-][vmem]`        |
| `load_ind`    | 5      | Yes              | NZ              | Big            | `[dest][addr][-][vmem][offset]`   |
| `store_imm`   | 6      | Yes              | NZ              | Big            | `[-][src][-][-][vmem][addr]`      |
| `store_dir`   | 7      | Yes              | NZ              | Small          | `[addr][src][-][-][vmem]`         |
| `store_ind`   | 8      | Yes              | NZ              | Big            | `[addr][src][-][-][vmem][offset]` |
| `mov`         | 9      | Yes              | NZ              | Small          | `[dest][src][-][-][-]`            |
| `cmp`         | 10     | Yes              | NZCVEGL         | Small          | `[lhs][rhs][-][-][-]`             |
| `fcmp`        | 11     | Yes              | NZCVEGL         | Small          | `[lhs][rhs][-][-][-]`             |
| `csel`        | 12     | Yes              | -               | Small          | `[dest][lhs][rhs][-][cond]`       |
| `b`           | 13     | No               | -               | Jump/Branch    | `[offset][cond]`                  |
| `j`           | 14     | No               | -               | Jump/Branch    | `[offset][-]`                     |
| `add`         | 15     | Yes              | NZCV            | Small          | `[dest][lhs][rhs][-][-]`          |
| `sub`         | 16     | Yes              | NZCV            | Small          | `[dest][lhs][rhs][-][-]`          |
| `mul`         | 17     | Yes              | NZV             | Small          | `[dest][lhs][rhs][-][-]`          |
| `div`         | 18     | Yes              | NZ              | Small          | `[dest][lhs][rhs][-][-]`          |
| `mod`         | 19     | Yes              | NZ              | Small          | `[dest][lhs][rhs][-][-]`          |
| `iadd`        | 20     | Yes              | NZCV            | Small          | `[dest][lhs][rhs][-][-]`          |
| `isub`        | 21     | Yes              | NZCV            | Small          | `[dest][lhs][rhs][-][-]`          |
| `imul`        | 22     | Yes              | NZV             | Small          | `[dest][lhs][rhs][-][-]`          |
| `idiv`        | 23     | Yes              | NZ              | Small          | `[dest][lhs][rhs][-][-]`          |
| `imod`        | 24     | Yes              | NZ              | Small          | `[dest][lhs][rhs][-][-]`          |
| `fadd`        | 25     | Only qword/dword | NZ              | Small          | `[dest][lhs][rhs][-][-]`          |
| `fsub`        | 26     | Only qword/dword | NZ              | Small          | `[dest][lhs][rhs][-][-]`          |
| `fmul`        | 27     | Only qword/dword | NZ              | Small          | `[dest][lhs][rhs][-][-]`          |
| `fdiv`        | 28     | Only qword/dword | NZ              | Small          | `[dest][lhs][rhs][-][-]`          |
| `fmod`        | 29     | Only qword/dword | NZ              | Small          | `[dest][lhs][rhs][-][-]`          |
| `ineg`        | 30     | Yes              | NZ              | Small          | `[dest][lhs][-][-][-]`            |
| `fneg`        | 31     | Only qword/dword | NZ              | Small          | `[dest][lhs][-][-][-]`            |
| `shl`         | 32     | No               | NZ              | Small          | `[dest][lhs][rhs][-][-]`          |
| `shr`         | 33     | No               | NZ              | Small          | `[dest][lhs][rhs][-][-]`          |
| `and`         | 34     | Yes              | NZ              | Small          | `[dest][lhs][rhs][-][-]`          |
| `or`          | 35     | Yes              | NZ              | Small          | `[dest][lhs][rhs][-][-]`          |
| `xor`         | 36     | Yes              | NZ              | Small          | `[dest][lhs][rhs][-][-]`          |
| `not`         | 37     | Yes              | NZ              | Small          | `[dest][lhs][-][-][-]`            |
| `muladd`      | 38     | Yes              | NZV             | Small          | `[dest][lhs][rhs][rhs2][-]`       |
| `call`        | 39     | No               | -               | Jump/Branch    | `[offset][-]`                     |
| `ccall`       | 40     | No               | -               | Jump/Branch    | `[offset][cond]`                  |
| `ret`         | 41     | No               | -               | Small          | `[-][-][-][-][-]`                 |
| `push`        | 42     | Yes              | -               | Small          | `[src][-][-][-][-]`               |
| `pop`         | 43     | Yes              | NZ              | Small          | `[dest][-][-][-][-]`              |
| `libc_call`   | 44     | No               | -               | Small          | `[-][-][-][-][libc_callcode]`     |
| `native_call` | 45     | No               | -               | Big            | TODO                              |
| `vtoreal`     | 46     | No               | -               | Small          | `[dest][src][-][-][-]`            |
| `breakpoint`  | 63     | No               | -               | Small          | `[-][-][-][-][-]`                 |

Note that because all registers are callee-saved, value of status register might change after `call`, `ccall`, `libc_call`, `native_call`, even though the instruction itself does not touch the status register.

## LibC callcodes

LBVM uses a 8-bit callcode for calling libc functions. It does not cover all the libc functions, but the more common ones.

| Name       | Callcode |
|------------|----------|
| `exit`     | 255      |
| `malloc`   | 1        |
| `realloc`  | 2        |
| `free`     | 3        |
| `fwrite`   | 4        |
| `fread`    | 5        |
| `printf`   | 6        |
| `fprintf`  | 7        |
| `scanf`    | 8        |
| `fscanf`   | 9        |
| `puts`     | 10       |
| `fputs`    | 11       |
| `snprintf` | 12       |
| `fopen`    | 13       |
| `fclose`   | 14       |
| `memcpy`   | 15       |
| `memmove`  | 16       |
| `memset`   | 17       |
| `bzero`    | 18       |
| `strlen`   | 19       |
| `strcpy`   | 20       |
| `strcat`   | 21       |
| `strcmp`   | 22       |
