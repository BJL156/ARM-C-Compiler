# ARM C Compiler
[![Language](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20WSL-lightgrey.svg)](https://www.linux.org/)
[![Architecture](https://img.shields.io/badge/target-AArch64-red.svg)](https://developer.arm.com/Architectures/AArch64)

A lightweight, zero-dependency C compiler written from scratch in C. It compiles C source code into AArch64 source code. It can then be assembled using an assembler such as GAS or by using the previous project's assembler [ARM C Assembler](https://github.com/BJL156/ARM-Assembler).

This C compiler follows the next stage of my compiler AArch64 toolchain. It continues from my last project: [ARM Assembler](https://github.com/BJL156/ARM-Assembler) by allowing C to be translated into assembly for my assembler to translate directly into an ELF64 executable. 

## Build
> [!NOTE]
> **Needs to be built on Linux. For Windows, use WSL.**

Clone the repository and change into its directory:
```bash
git clone https://github.com/BJL156/ARM-C-Compiler
cd ARM-C-Assembler
```
Then use CMake:
```bash
cmake -B build
cmake --build build
```
The final executable will be written to: `build/compiler`.

## Usage
```bash
./compiler <file.c> <output>
  <file.c>  C source file.
  <output>  Output AArch64 file.
```
