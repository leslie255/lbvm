# Leslie255's Bytecode Virtual Machine

I made this VM so I can submit my C coursework in the form of a bunch of bytecode and a `machine.h` file.
It is not made as a "useful" bytecode VM hence some of the design choices.

See [manual.md](manual.md) for design of the Bytecode VM.

Currently everything except `native_call` instruction works (so you can't call a local native function in the bytecode, but you can call libc functions).

Assembler is available at: [leslie255/lbvm_asm](https://github.com/leslie255/lbvm_asm).

## Usage

There is a `run.py` file for quickly running an assembly file. For more detail, see:

```bash
python3 run.py --help
```

Note that this requires cloning the git submodule of `lbvm_asm`.

## LICENSE

This project is licensed under GPLv3.
