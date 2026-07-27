#include "codegen.h"

#include <stdlib.h>
#include <string.h>

void codegen_init(CodeGen *codegen, FILE *out) {
  memset(codegen, 0, sizeof(*codegen));
  codegen->out = out;
}

int count_locals(Stmt *stmt) {
  if (stmt == NULL) {
    return 0;
  }

  switch (stmt->type) {
    case STMT_VAR_DECL: {
      return 1;
    }
    case STMT_BLOCK: {
      int count = 0;
      for (int i = 0; i < stmt->block.count; i++) {
        count += count_locals(stmt->block.items[i]);
      }
      return count;
    }
    case STMT_IF: {
      return count_locals(stmt->if_stmt.then_branch) + count_locals(stmt->if_stmt.else_branch);
    }
    case STMT_WHILE: {
      return count_locals(stmt->while_stmt.body);
    }
    case STMT_FOR: {
      return count_locals(stmt->for_stmt.init) + count_locals(stmt->for_stmt.body);
    }
    default: {
      return 0;
    }
  }
}

int lookup_symbol(CodeGen *codegen, const char *name) {
  for (int i = 0; i < codegen->symbol_count; i++) {
    if (strcmp(name, codegen->symbols[i].name) == 0) {
      return codegen->symbols[i].offset;
    }
  }

  fprintf(stderr, "Error: undeclared variable \"%s\".\n", name);
  exit(1);
}

void codegen_bool_from_flags(CodeGen *codegen, const char *condition) {
  int true_label = codegen->label_count++;
  int end_label = codegen->label_count++;

  fprintf(codegen->out, "\tb.%s .Ltrue%d\n", condition, true_label);
  fprintf(codegen->out, "\tmov x0, #0\n");
  fprintf(codegen->out, "\tb .Lend%d\n", end_label);
  fprintf(codegen->out, ".Ltrue%d:\n", true_label);
  fprintf(codegen->out, "\tmov x0, #1\n");
  fprintf(codegen->out, ".Lend%d:\n", end_label);
}

void codegen_expr(CodeGen *codegen, Expr *expr) {
  switch (expr->type) {
    case EXPR_INT_LITERAL: {
      fprintf(codegen->out, "\tmov x0, #%lld\n", (long long)expr->literal);
      break;
    }
    case EXPR_IDENTIFIER: {
      int offset = lookup_symbol(codegen, expr->name);
      fprintf(codegen->out, "\tldr x0, [x29, #%d]\n", offset);
      break;
    }
    case EXPR_BINARY: {
      codegen_expr(codegen, expr->binary.left);
      fprintf(codegen->out, "\tstr x0, [sp, #-16]!\n");
      codegen_expr(codegen, expr->binary.right);
      fprintf(codegen->out, "\tldr x1, [sp], #16\n");

      switch (expr->binary.op) {
        case TOKEN_PLUS: {
          fprintf(codegen->out, "\tadd x0, x1, x0\n");
          break;
        }
        case TOKEN_MINUS: {
          fprintf(codegen->out, "\tsub x0, x1, x0\n");
          break;
        }
        case TOKEN_STAR: {
          fprintf(codegen->out, "\tmul x0, x1, x0\n");
          break;
        }
        case TOKEN_SLASH: {
          fprintf(codegen->out, "\tsdiv x0, x1, x0\n");
          break;
        }
        case TOKEN_PERCENT: {
          fprintf(codegen->out, "\tsdiv x2, x1, x0\n");
          fprintf(codegen->out, "\tmul x2, x2, x0\n");
          fprintf(codegen->out, "\tsub x0, x1, x2\n");
          break;
        }
        case TOKEN_EQ: {
          fprintf(codegen->out, "\tcmp x1, x0\n");
          codegen_bool_from_flags(codegen, "eq");
          break;
        }
        case TOKEN_NEQ: {
          fprintf(codegen->out, "\tcmp x1, x0\n");
          codegen_bool_from_flags(codegen, "ne");
          break;
        }
        case TOKEN_LT: {
          fprintf(codegen->out, "\tcmp x1, x0\n");
          codegen_bool_from_flags(codegen, "lt");
          break;
        }
        case TOKEN_LE: {
          fprintf(codegen->out, "\tcmp x1, x0\n");
          codegen_bool_from_flags(codegen, "le");
          break;
        }
        case TOKEN_GT: {
          fprintf(codegen->out, "\tcmp x1, x0\n");
          codegen_bool_from_flags(codegen, "gt");
          break;
        }
        case TOKEN_GE: {
          fprintf(codegen->out, "\tcmp x1, x0\n");
          codegen_bool_from_flags(codegen, "ge");
          break;
        }
        default: {
          fprintf(stderr, "Error: codegen not implemented for binary operator: %d.\n", expr->binary.op);
          exit(1);
        }
      }

      break;
    }
    case EXPR_UNARY: {
      codegen_expr(codegen, expr->unary.operand);

      switch (expr->unary.op) {
        case TOKEN_MINUS: {
          fprintf(codegen->out, "\tneg x0, x0\n");
          break;
        }
        case TOKEN_NOT: {
          fprintf(codegen->out, "\tcmp x0, #0\n");
          codegen_bool_from_flags(codegen, "eq");
          break;
        }
        default: {
          fprintf(stderr, "Error: codegen not implemented for unary operator: %d.\n", expr->unary.op);
          exit(1);
        }
      }

      break;
    }
    default: {
      fprintf(stderr, "Error: codegen not implemented for expression type: %d.\n", expr->type);
      exit(1);
    }
  }
}

void codegen_stmt(CodeGen *codegen, Stmt *stmt) {
  switch (stmt->type) {
    case STMT_RETURN: {
      if (stmt->return_stmt.value) {
        codegen_expr(codegen, stmt->return_stmt.value);
      }

      fprintf(codegen->out, "\tb %s\n", codegen->return_label);
      break;
    }
    case STMT_BLOCK: {
      for (int i = 0; i < stmt->block.count; i++) {
        codegen_stmt(codegen, stmt->block.items[i]);
      }
      break;
    }
    default: {
      fprintf(stderr, "Error: codegen not implemented for statement type: %d.\n", stmt->type);
      exit(1);
    }
  }
}

void codegen_function(CodeGen *codegen, Function *fn) {
  codegen->next_offset = 0;
  codegen->symbol_count = 0;
  
  snprintf(codegen->return_label, sizeof(codegen->return_label), ".Lend%d", codegen->label_count++);

  int local_count = count_locals(fn->body);
  int total_vars = fn->params.count + local_count;
  int frame_size = total_vars * 8;

  frame_size = (frame_size + 15) / 16 * 16;

  fprintf(codegen->out, "%s:\n", fn->name);
  fprintf(codegen->out, "\tstp x29, x30, [sp, #-16]!\n");
  fprintf(codegen->out, "\tmov x29, sp\n");
  fprintf(codegen->out, "\tsub sp, sp, #%d\n", frame_size);

  for (int i = 0; i < fn->params.count; i++) {
    Param *param = &fn->params.items[i];

    codegen->next_offset -= 8;
    int offset = codegen->next_offset;

    Symbol symbol;
    strncpy(symbol.name, param->name, sizeof(symbol.name) - 1);
    symbol.offset = offset;
    codegen->symbols[codegen->symbol_count++] = symbol;

    fprintf(codegen->out, "\tstr x%d, [x29, #%d]\n", i, offset);
  }

  codegen_stmt(codegen, fn->body);

  fprintf(codegen->out, "%s:\n", codegen->return_label);

  fprintf(codegen->out, "\tadd sp, sp, #%d\n", frame_size);
  fprintf(codegen->out, "\tldp x29, x30, [sp], #16\n");
  fprintf(codegen->out, "\tret\n");
}

void codegen_program(CodeGen *codegen, Program *program) {
  for (int i = 0; i < program->count; i++) {
    codegen_function(codegen, &program->items[i]);
  }
}
