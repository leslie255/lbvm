#include "common.h"
#include "debug_utils.h"

#include "machine.h"

typedef enum ProgramLoadResult {
  ProgramLoadOk,
  ProgramLoadErrorInvalidFileHeader,
  ProgramLoadErrorInvalidBlockHeader,
  ProgramLoadErrorEofInBlock,
  ProgramLoadErrorOutOfBound,
} ProgramLoadResult;

void print_program_load_result(ProgramLoadResult r);

ProgramLoadResult load_machine_state_from_file(Machine *restrict machine, FILE *f);
