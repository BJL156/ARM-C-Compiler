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

## Example
### Input (`examples/add.c`)
```c
int add(int a, int b) {
  return a + b;
}

int main() {
  return add(5, 6);
}
```
### Output
```bash
$ ./build/compiler ./examples/add.s ./build/add
$ cat ./build/add
.global _start
_start:
        bl main
        mov x8, #93
        svc #0
add:
        stp x29, x30, [sp, #-16]!
        mov x29, sp
        sub sp, sp, #16
        str x0, [x29, #-8]
        str x1, [x29, #-16]
        ldr x0, [x29, #-8]
        str x0, [sp, #-16]!
        ldr x0, [x29, #-16]
        ldr x1, [sp], #16
        add x0, x1, x0
        b .Lend0
.Lend0:
        add sp, sp, #16
        ldp x29, x30, [sp], #16
        ret
main:
        stp x29, x30, [sp, #-16]!
        mov x29, sp
        sub sp, sp, #0
        mov x0, #5
        str x0, [sp, #-16]!
        mov x0, #6
        str x0, [sp, #-16]!
        ldr x1, [sp], #16
        ldr x0, [sp], #16
        bl add
        b .Lend1
.Lend1:
        add sp, sp, #0
        ldp x29, x30, [sp], #16
        ret
```
The program can then be assembled using: [ARM Assembler](https://github.com/BJL156/ARM-Assembler).
