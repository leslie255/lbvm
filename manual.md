# LBVM Manual

Note that LBVM is not designed with memory security or safety in mind, hence some of the design choices.

LBVM is little endian.

## Registers

LBVM has 16 registers of 64 bits, encoded as 0~15.

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

Oplen is irrelevant for some operations, for those oplen must be `00`.

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

Despite of that, the status register is 64 bits, for convenience sake.

Any instruction that sets a status flag clears all other irrelevant bits to zero.

Instructions that uses the status flags (e.g. `b`, `csel`) uses its 8 `flags` bits as the status flags.
The 7 least significant bits can be masked together for boolean OR logic, while the most significant bit is used as negation.

For example, the byte `0b10110000` (`0b10000000 & G & E`) encodes the condition of `!(greater | equal)`.

## Memory

LBVM has a virtual memory `vmem` of 64kB. Memory of the host machine can also be accessed, although restrictions may be applied by the emulator for debug reasons.

The load/store instructions takes in a `vmem` flag that determines whether the virtual or the host memory is accessed (`0` for vmem, `1` for real memory).

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

For this reason `store_dir` and `load_dir` is a small instruction while the other load/store instructions are big instructions.

## Instruction set

| Name        | Oplen relevant?  | Status affected | Encoding Fomat | Operands                          |
|-------------|----------------- |-----------------|----------------|-----------------------------------|
| brk         | No               | -               | Small          | `[-][-][-][-][-]`                 |
| cbrk        | No               | -               | Small          | `[-][-][-][-][cond]`              |
| nop         | No               | -               | Small          | `[-][-][-][-][-]`                 |
| load_imm    | Yes              | NZ              | Big            | `[dest][-][-][vmem][data]`        |
| load_dir    | Yes              | NZ              | Big            | `[dest][addr][vmem][-]`           |
| load_ind    | Yes              | NZ              | Big            | `[dest][addr][-][vmem][offset]`   |
| store_imm   | Yes              | NZ              | Big            | `[-][src][-][-][vmem][addr]`      |
| store_dir   | Yes              | NZ              | Small          | `[addr][src][-][-][vmem]`         |
| store_ind   | Yes              | NZ              | Big            | `[addr][src][-][-][vmem][offset]` |
| mov         | Yes              | NZ              | Small          | `[dest][src][-][-][-]`            |
| cmp         | Yes              | NZCVEGL         | Small          | `[lhs][rhs][-][-][-]`             |
| csel        | Yes              | -               | Small          | `[dest][lhs][rhs][-][cond]`       |
| b           | No               | -               | Jump/Branch    | `[offset][cond]`                  |
| j           | No               | -               | Jump/Branch    | `[offset][-]`                     |
| add         | Yes              | NZCV            | Small          | `[dest][lhs][rhs][][]`            |
| sub         | Yes              | NZCV            | Small          | `[dest][lhs][rhs][][]`            |
| mul         | Yes              | NZV             | Small          | `[dest][lhs][rhs][][]`            |
| div         | Yes              | NZ              | Small          | `[dest][lhs][rhs][][]`            |
| mod         | Yes              | NZ              | Small          | `[dest][lhs][rhs][][]`            |
| iadd        | Yes              | NZCV            | Small          | `[dest][lhs][rhs][][]`            |
| isub        | Yes              | NZCV            | Small          | `[dest][lhs][rhs][][]`            |
| imul        | Yes              | NZV             | Small          | `[dest][lhs][rhs][][]`            |
| idiv        | Yes              | NZ              | Small          | `[dest][lhs][rhs][][]`            |
| imod        | Yes              | NZ              | Small          | `[dest][lhs][rhs][][]`            |
| fadd        | Only qword/dword | NZ              | Small          | `[dest][lhs][rhs][][]`            |
| fsub        | Only qword/dword | NZ              | Small          | `[dest][lhs][rhs][][]`            |
| fmul        | Only qword/dword | NZ              | Small          | `[dest][lhs][rhs][][]`            |
| fdiv        | Only qword/dword | NZ              | Small          | `[dest][lhs][rhs][][]`            |
| fmod        | Only qword/dword | NZ              | Small          | `[dest][lhs][rhs][][]`            |
| and         | Yes              | NZ              | Small          | `[dest][lhs][rhs][][]`            |
| or          | Yes              | NZ              | Small          | `[dest][lhs][rhs][][]`            |
| xor         | Yes              | NZ              | Small          | `[dest][lhs][rhs][][]`            |
| not         | Yes              | NZ              | Small          | `[dest][lhs][][][]`               |
| muladd      | Yes              | NZV             | Small          | `[dest][lhs][rhs][rhs2][]`        |
| call        | No               | -               | Jump/Branch    | `[offset][-]`                     |
| ccall       | No               | -               | Jump/Branch    | `[offset][cond]`                  |
| ret         | No               | -               | Small          | `[-][-][-][-][-]`                 |
| push        | Yes              | -               | Small          | `[src][-][-][-][-]`               |
| pop         | Yes              | NZ              | Small          | `[dest][-][-][-][-]`              |
| libc_call   | No               | -               | Small          | `[libc_callcode][-][-][-][-]`     |
| native_call | Big              | -               | Small          | `[-][-][-][-][-][addr]`           |
| breakpoint  | No               | -               | Small          | `[-][-][-][-][-]`                 |

## LibC callcodes

TODO
