# LBVM Implementation Standard

Note that LBVM is not designed with memory security or safety in mind, which is the reason for some of the design choices.

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

## Status register and status flags

## Memory

LBVM has a virtual memory `vmem` of 64kB. Memory of the host machine can also be accessed, although restrictions may be applied by the emulator for debug reasons.

The load/store instructions takes in a `vmem` flag that determines whether the virtual or the host memory is accessed (`0` for vmem, `1` for real memory).

Pointers have sizes of 64 bits.

Access of `vmem` with out-of-bound pointers results in halting of machine.

## Addressing modes

LBVM has three addressing modes for load/store instructions.

- `imm`: Immediate
- `dir`: Direct
- `ind`: Indirect (loads value on address of `reg + offset`, where `offset` is a 64-bit immediate)

## General format for instructions

Small Instruction (4 bytes):

```
byte0: [opcode:6][oplen:2]
byte1: [operand1:4][operand0:4]
byte2: [operand3:4][operand2:4]
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
byte1:    [operand1:4][operand0:4]
byte2:    [operand3:4][operand2:4]
byte3:    [flags:8]
byte4~11: [data:64]
```

`oplen` is the length of operations:

| Length    | Encoding |
|-----------|----------|
| qword (8) | `0`      |
| dword (4) | `1`      |
| word (2)  | `2`      |
| byte (1)  | `3`      |
