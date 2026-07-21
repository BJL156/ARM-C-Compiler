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

typedef enum {
  TYPE_INT,
  TYPE_CHAR,
  TYPE_VOID
} Type;

typedef struct {
  Type base;
  int pointer_depth;
} TypeInfo;

typedef enum {
  STMT_EXPR,
  STMT_VAR_DECL,
  STMT_IF,
  STMT_WHILE,
  STMT_FOR,
  STMT_RETURN,
  STMT_BLOCK
} StmtType;

typedef struct Stmt Stmt;

typedef struct {
  Stmt **items;
  int count;
  int capacity;
} StmtList;

struct Stmt {
  StmtType type;
  union {
    struct { Expr *expr; } expr_stmt;
    struct { TypeInfo var_type; char name[64]; Expr *init; } var_decl;
    struct { Expr *condition; Stmt *then_branch; Stmt *else_branch; } if_stmt;
    struct { Expr *condition; Stmt *body; } while_stmt;
    struct { Stmt *init; Expr *condition; Expr *increment; Stmt *body; } for_stmt;
    struct { Expr *value; } return_stmt;
    StmtList block;
  };
};

typedef struct {
  TypeInfo type;
  char name[64];
} Param;

typedef struct {
  Param *items;
  int count;
  int capacity;
} ParamList;

typedef struct {
  TypeInfo return_type;
  char name[64];
  ParamList params;
  Stmt *body;
} Function;

typedef struct {
  Function *items;
  int count;
  int capacity;
} Program;

#endif
