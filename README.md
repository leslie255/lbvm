# Leslie255's Bytecode Virtual Machine

I made this VM so I can submit my C coursework in the form of a bunch of bytecode and a `machine.h` file.
It is not made as a "useful" bytecode VM hence some of the design choices.

See `manual.md` for design of the Bytecode VM.

Currently everything except `native_call` instruction works (so you can't call a local native function in the bytecode, but you can call libc functions).

An assembler is planned but it won't be written in C.

## LICENSE

This project is licensed under GPLv3.
