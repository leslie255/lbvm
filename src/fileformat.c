#include "fileformat.h"
#include "endian.h"

void print_program_load_result(ProgramLoadResult r) {
  switch (r) {
  case ProgramLoadOk:
    printf("ProgramLoadOk");
    break;
  case ProgramLoadErrorInvalidFileHeader:
    printf("ProgramLoadErrorInvalidFileHeader");
    break;
  case ProgramLoadErrorInvalidBlockHeader:
    printf("ProgramLoadErrorInvalidBlockHeader");
    break;
  case ProgramLoadErrorEofInBlock:
    printf("ProgramLoadErrorEofInBlock");
    break;
  case ProgramLoadErrorOutOfBound:
    printf("ProgramLoadErrorOutOfBound");
    break;
  }
}

typedef struct ProgramLoadState {
  Machine *restrict machine;
  FILE *f;
  bool finished;
} ProgramLoadState;

static inline ProgramLoadResult check_header(ProgramLoadState *state);

static inline ProgramLoadResult read_block(ProgramLoadState *state);

static inline ProgramLoadResult write_bytes(ProgramLoadState *state, const u8 *bytes, u32 start_address, u16 length);

ProgramLoadResult load_machine_state_from_file(Machine *restrict machine, FILE *f) {
  ProgramLoadState state = {
      .machine = machine,
      .f = f,
      .finished = false,
  };
  ProgramLoadResult check_header_result = check_header(&state);
  if (check_header_result != ProgramLoadOk)
    return check_header_result;
  while (!state.finished) {
    ProgramLoadResult read_block_result = read_block(&state);
    if (read_block_result != ProgramLoadOk)
      return read_block_result;
  }
  return ProgramLoadOk;
}

static inline ProgramLoadResult check_header(ProgramLoadState *state) {
  static const u8 expected_header[11] = "LBVMProgram";
  u8 found_header[11] = {0};
  fread(&found_header, 1, 11, state->f);
  if (memcmp(expected_header, found_header, sizeof(expected_header)) != 0)
    return ProgramLoadErrorInvalidFileHeader;
  return ProgramLoadOk;
}

static inline ProgramLoadResult read_block(ProgramLoadState *state) {
  u8 block_header[7] = {0};
  usize len = dbg_println(fread(&block_header, 1, 7, state->f));
  if (len == 0) {
    state->finished = true;
    return ProgramLoadOk;
  }
  if (len != 7)
    return ProgramLoadErrorInvalidBlockHeader;
  u8 magic_number = block_header[0];
  u32 start_address = u32_from_le_bytes(&block_header[1]);
  u16 length = u16_from_le_bytes(&block_header[5]);
  dbg_println_hex(magic_number);
  dbg_println_hex(start_address);
  dbg_println(length);
  if (magic_number != 0xAA)
    return ProgramLoadErrorInvalidBlockHeader;
  if (length == 0)
    return ProgramLoadOk;
  u8 *bytes = xalloc(u8, length);
  if (fread(bytes, 1, length, state->f) != length)
    return ProgramLoadErrorEofInBlock;
  for (usize i = 0; i < length; ++i) {
    printf("%02X ", bytes[i]);
  }
  printf("\n");
  return write_bytes(state, bytes, start_address, length);
}

static inline ProgramLoadResult write_bytes(ProgramLoadState *state, const u8 *bytes, u32 start_address, u16 length) {
  if ((start_address + length) / 0x10000 != start_address / 0x10000)
    return ProgramLoadErrorOutOfBound;
  u8 *restrict p;
  switch (start_address) {
  case 0x00000 ... 0x0FFFF: {
    p = &state->machine->vmem_stack[start_address - 0x00000];
  } break;
  case 0x10000 ... 0x1FFFF: {
    p = &state->machine->vmem_text[start_address - 0x10000];
  } break;
  case 0x20000 ... 0x2FFFF: {
    p = &state->machine->vmem_data[start_address - 0x20000];
  } break;
  default:
    return ProgramLoadErrorOutOfBound;
  }
  memcpy(p, bytes, length);
  return ProgramLoadOk;
}
