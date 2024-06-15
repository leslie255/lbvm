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
  ProgramLoadResult result = check_header(&state);
  if (result != ProgramLoadOk)
    return result;
  while (!state.finished) {
    ProgramLoadResult result = read_block(&state);
    if (result != ProgramLoadOk)
      return result;
  }
  return ProgramLoadOk;
}

static inline ProgramLoadResult check_header(ProgramLoadState *state) {
  // Check header.
  static const u8 expected_header[] = "LBVMProgram";
  u8 found_header[sizeof(expected_header)] = {0};
  fread(found_header, sizeof(expected_header), 1, state->f);
  if (memcmp(expected_header, found_header, sizeof(expected_header)) != 0) {
    return ProgramLoadErrorInvalidFileHeader;
  }
  return ProgramLoadOk;
}

static inline ProgramLoadResult read_block(ProgramLoadState *state) {
  u8 magic_number = 0;
  fread(&magic_number, 1, 1, state->f);
  if (magic_number != 0xAA)
    return ProgramLoadErrorInvalidBlockHeader;
  u32 start_address = ({
    u8 bytes[4] = {0};
    usize x = fread(bytes, 4, 1, state->f);
    if (x != 4)
      return ProgramLoadErrorInvalidBlockHeader;
    u32_from_le_bytes(bytes);
  });
  u16 length = ({
    u8 bytes[2] = {0};
    usize x = fread(bytes, 2, 1, state->f);
    if (x != 2)
      return ProgramLoadErrorInvalidBlockHeader;
    u16_from_le_bytes(bytes);
  });
  u8 *bytes = xalloc(u8, length);
  usize x = fread(bytes, length, 1, state->f);
  if (x != length)
    return ProgramLoadErrorEofInBlock;
  return write_bytes(state, bytes, start_address, length);
}

static inline ProgramLoadResult write_bytes(ProgramLoadState *state, const u8 *bytes, u32 start_address, u16 length) {
  switch (start_address) {
  case 0x00000 ... 0x0FFFF:
    if (start_address + length > 0x0FFFF)
      return ProgramLoadErrorOutOfBound;
    memcpy(&state->machine->vmem_stack[start_address], bytes, length);
    return ProgramLoadOk;
  case 0x10000 ... 0x1FFFF:
    if (start_address + length > 0x1FFFF)
      return ProgramLoadErrorOutOfBound;
    memcpy(&state->machine->vmem_text[start_address], bytes, length);
    return ProgramLoadOk;
  case 0x20000 ... 0x2FFFF:
    if (start_address + length > 0x2FFFF)
      return ProgramLoadErrorOutOfBound;
    memcpy(&state->machine->vmem_data[start_address], bytes, length);
    return ProgramLoadOk;
  default:
    return ProgramLoadErrorOutOfBound;
  }
}
