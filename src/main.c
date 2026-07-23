#include "ast.h"
#include "lexer.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

void print_indent(int depth) {
  for (int i = 0; i < depth; i++) {
    printf("  ");
  }
}

void print_expr(Expr *expr, int depth) {
  if (!expr) {
    print_indent(depth);
    printf("(null)\n");
    return;
  }

  print_indent(depth);
  switch (expr->type) {
    case EXPR_INT_LITERAL: {
      printf("IntLiteral: %lld\n", (long long)expr->literal);
      break;
    }
    case EXPR_CHAR_LITERAL: {
      printf("CharLiteral: %lld\n", (long long)expr->literal);
      break;
    }
    case EXPR_IDENTIFIER: {
      printf("Identifier: %s\n", expr->name);
      break;
    }
    case EXPR_BINARY: {
      printf("Binary (op=%d)\n", expr->binary.op);
      print_expr(expr->binary.left, depth + 1);
      print_expr(expr->binary.right, depth + 1);
      break;
    }
    case EXPR_UNARY: {
      printf("Unary (op=%d)\n", expr->unary.op);
      print_expr(expr->unary.operand, depth + 1);
      break;
    }
    case EXPR_ASSIGN: {
      printf("Assign\n");
      print_indent(depth + 1);
      printf("Target:\n");
      print_expr(expr->assign.target, depth + 2);
      print_indent(depth + 1);
      printf("Value:\n");
      print_expr(expr->assign.value, depth + 2);
      break;
    }
    case EXPR_CALL: {
      printf("Call: %s\n", expr->call.name);
      for (int i = 0; i < expr->call.args.count; i++) {
        print_expr(expr->call.args.items[i], depth + 1);
      }
      break;
    }
    default: {
      printf("UnknownExpr (type=%d)\n", expr->type);
      break;
    }
  }
}

void print_stmt(Stmt *stmt, int depth) {
  if (!stmt) {
    print_indent(depth);
    printf("(null)\n");
    return;
  }

  print_indent(depth);
  switch (stmt->type) {
    case STMT_EXPR: {
      printf("ExprStmt\n");
      print_expr(stmt->expr_stmt.expr, depth + 1);
      break;
    }
    case STMT_VAR_DECL: {
      printf("VarDecl: %s\n", stmt->var_decl.name);
      if (stmt->var_decl.init) {
        print_indent(depth + 1);
        printf("Init:\n");
        print_expr(stmt->var_decl.init, depth + 2);
      }
      break;
    }
    case STMT_IF: {
      printf("If\n");
      print_indent(depth + 1);
      printf("Condition:\n");
      print_expr(stmt->if_stmt.condition, depth + 2);
      print_indent(depth + 1);
      printf("Then:\n");
      print_stmt(stmt->if_stmt.then_branch, depth + 2);
      if (stmt->if_stmt.else_branch) {
        print_indent(depth + 1);
        printf("Else:\n");
        print_stmt(stmt->if_stmt.else_branch, depth + 2);
      }
      break;
    }
    case STMT_FOR: {
      printf("For\n");
      if (stmt->for_stmt.init) {
        print_indent(depth + 1);
        printf("Init:\n");
        print_stmt(stmt->for_stmt.init, depth + 2);
      }
      if (stmt->for_stmt.condition) {
        print_indent(depth + 1);
        printf("Condition:\n");
        print_expr(stmt->for_stmt.condition, depth + 2);
      }
      if (stmt->for_stmt.increment) {
        print_indent(depth + 1);
        printf("Increment:\n");
        print_expr(stmt->for_stmt.increment, depth + 2);
      }
      print_indent(depth + 1);
      printf("Body:\n");
      print_stmt(stmt->for_stmt.body, depth + 2);
      break;
    }
    case STMT_WHILE: {
      printf("While\n");
      print_indent(depth + 1);
      printf("Condition:\n");
      print_expr(stmt->while_stmt.condition, depth + 2);
      print_indent(depth + 1);
      printf("Body:\n");
      print_stmt(stmt->while_stmt.body, depth + 2);
      break;
    }
    case STMT_RETURN: {
      printf("Return\n");
      if (stmt->return_stmt.value) {
        print_expr(stmt->return_stmt.value, depth + 1);
      }
      break;
    }
    case STMT_BLOCK: {
      printf("Block\n");
      for (int i = 0; i < stmt->block.count; i++) {
        print_stmt(stmt->block.items[i], depth + 1);
      }
      break;
    }
    default: {
      printf("UnknownStmt (type=%d)\n", stmt->type);
      break;
    }
  }
}

void print_function(Function *fn) {
  printf("Function: %s\n", fn->name);
  for (int i = 0; i < fn->params.count; i++) {
    print_indent(1);
    printf("Param: %s\n", fn->params.items[i].name);
  }
  print_stmt(fn->body, 1);
}

void print_program(Program *program) {
  for (int i = 0; i < program->count; i++) {
    print_function(&program->items[i]);
  }
}

static char *read_file(const char *filepath) {
  FILE *file = fopen(filepath, "r");
  if (!file) {
    fprintf(stderr, "Error: Failed to open \"%s\".\n", filepath);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  rewind(file);
  char *buffer = malloc(size + 1);
  fread(buffer, 1, size, file);
  buffer[size] = '\0';

  fclose(file);
  return buffer;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <input.c> <output>\n", argv[0]);
    return 1;
  }

  char *src = read_file(argv[1]);
  if (!src) {
    return 1;
  }

  Lexer lexer;
  lexer_init(&lexer, src);

  Parser parser;
  parser_init(&parser, &lexer);

  Program program = parse_program(&parser);
  
  print_program(&program);

  FILE *output = fopen(argv[2], "wb");
  if (!output) {
    fprintf(stderr, "Error: Failed to open \"%s\".\n", argv[2]);
    return 1;
  }

  fclose(output);

  return 0;
}
