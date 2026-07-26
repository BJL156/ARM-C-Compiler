#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

#include <stdio.h>

typedef struct {
  char name[64];
  int offset;
} Symbol;

typedef struct {
  FILE *out;
  Symbol symbols[64];
  int symbol_count;
  int next_offset;
  int label_count;
  char return_label[64];
} CodeGen;

void codegen_init(CodeGen *codegen, FILE *out);
void codegen_program(CodeGen *codegen, Program *program);

#endif
