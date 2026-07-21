#ifndef AST_H
#define AST_H

#include "lexer.h"

#include <stdint.h>

typedef enum {
  EXPR_INT_LITERAL,
  EXPR_CHAR_LITERAL,
  EXPR_IDENTIFIER,
  EXPR_BINARY,
  EXPR_UNARY,
  EXPR_ASSIGN,
  EXPR_CALL
} ExprType;

typedef struct Expr Expr;

typedef struct {
  Expr **items;
  int count;
  int capacity;
} ExprList;

struct Expr {
  ExprType type;
  union {
    int64_t literal;
    char name[64];
    struct { TokenType op; Expr *left; Expr *right; } binary;
    struct { TokenType op; Expr *operand; } unary;
    struct { Expr *target; Expr *value; } assign;
    struct { char name[64]; ExprList args; } call;
  };
};

#endif
