#include "parser.h"
#include "ast.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void parser_init(Parser *parser, Lexer *lexer) {
  memset(parser, 0, sizeof(*parser));
  parser->lexer = lexer;
  parser->current = next_token(lexer);
}

static void advance(Parser *parser) {
  parser->previous = parser->current;
  parser->current = next_token(parser->lexer);
}

bool check(Parser *parser, TokenType type) {
  return parser->current.type == type;
}

bool match(Parser *parser, TokenType type) {
  if (check(parser, type)) {
    advance(parser);
    return true;
  }

  return false;
}

Token expect(Parser *parser, TokenType type, const char *msg) {
  if (match(parser, type)) {
    return parser->previous;
  }

  fprintf(stderr, "Error: %s at line: %d.\n", msg, parser->current.line);
  exit(1);
}

Expr *expr_new(ExprType type) {
  Expr *expr = malloc(sizeof(Expr));
  memset(expr, 0, sizeof(*expr));
  expr->type = type;

  return expr;
}

void exprlist_append(ExprList *exprlist, Expr *expr) {
  if (exprlist->count == exprlist->capacity) {
    exprlist->capacity = exprlist->capacity == 0 ? 8 : exprlist->capacity * 2;
    exprlist->items = realloc(exprlist->items, exprlist->capacity * sizeof(Expr *));
  }
  exprlist->items[exprlist->count++] = expr;
}

Expr *parse_expression(Parser *parser);

Expr *parse_primary(Parser *parser) {
  if (check(parser, TOKEN_INT_LITERAL)) {
    Token token = expect(parser, TOKEN_INT_LITERAL, "expected integer literal");
    Expr *expr = expr_new(EXPR_INT_LITERAL);
    expr->literal = token.imm;

    return expr;
  }

  if (check(parser, TOKEN_CHAR_LITERAL)) {
    Token token = expect(parser, TOKEN_CHAR_LITERAL, "expected character literal");
    Expr *expr = expr_new(EXPR_CHAR_LITERAL);
    expr->literal = token.imm;

    return expr;
  }

  if (check(parser, TOKEN_IDENTIFIER)) {
    Token token = expect(parser, TOKEN_IDENTIFIER, "expected identifier");

    if (check(parser, TOKEN_LPAREN)) {
      expect(parser, TOKEN_LPAREN, "expected '('");
      Expr *expr = expr_new(EXPR_CALL);
      strncpy(expr->call.name, token.str, sizeof(expr->call.name) - 1);

      if (!check(parser, TOKEN_RPAREN)) {
        Expr *arg = parse_expression(parser);
        exprlist_append(&expr->call.args, arg);
        while (match(parser, TOKEN_COMMA)) {
          arg = parse_expression(parser);
          exprlist_append(&expr->call.args, arg);
        }
      }

      expect(parser, TOKEN_RPAREN, "expected ')'");
      return expr;
    }

    Expr *expr = expr_new(EXPR_IDENTIFIER);
    strncpy(expr->name, token.str, sizeof(expr->name) - 1);
    return expr;
  }

  if (check(parser, TOKEN_LPAREN)) {
    expect(parser, TOKEN_LPAREN, "expected '('");
    Expr *expr = parse_expression(parser);
    expect(parser, TOKEN_RPAREN, "expected ')' after expression");

    return expr;
  }

  fprintf(stderr, "Error: unexpected token at line: %d.\n", parser->current.line);
  exit(1);
}

Expr *parse_unary(Parser *parser) {
  if (check(parser, TOKEN_MINUS) || check(parser, TOKEN_NOT) ||
      check(parser, TOKEN_STAR) || check(parser, TOKEN_AMP)) {
    advance(parser);
    Token op = parser->previous;
    Expr *operand = parse_unary(parser);
    Expr *expr = expr_new(EXPR_UNARY);
    expr->unary.op = op.type;
    expr->unary.operand = operand;

    return expr;
  }

  return parse_primary(parser);
}

Expr *parse_factor(Parser *parser) {
  Expr *left = parse_unary(parser);

  while (check(parser, TOKEN_STAR) || check(parser, TOKEN_SLASH) || check(parser, TOKEN_PERCENT)) {
    advance(parser);
    Token op = parser->previous;
    Expr *right = parse_unary(parser);

    Expr *expr = expr_new(EXPR_BINARY);
    expr->binary.op = op.type;
    expr->binary.left = left;
    expr->binary.right = right;
    left = expr;
  }

  return left;
}

Expr *parse_term(Parser *parser) {
  Expr *left = parse_factor(parser);

  while (check(parser, TOKEN_PLUS) || check(parser, TOKEN_MINUS)) {
    advance(parser);
    Token op = parser->previous;
    Expr *right = parse_factor(parser);

    Expr *expr = expr_new(EXPR_BINARY);
    expr->binary.op = op.type;
    expr->binary.left = left;
    expr->binary.right = right;
    left = expr;
  }

  return left;
}

Expr *parse_comparison(Parser *parser) {
  Expr *left = parse_term(parser);

  while (check(parser, TOKEN_LT) || check(parser, TOKEN_LE) || check(parser, TOKEN_GT) || check(parser, TOKEN_GE)) {
    advance(parser);
    Token op = parser->previous;
    Expr *right = parse_term(parser);

    Expr *expr = expr_new(EXPR_BINARY);
    expr->binary.op = op.type;
    expr->binary.left = left;
    expr->binary.right = right;
    left = expr;
  }

  return left;
}

Expr *parse_equality(Parser *parser) {
  Expr *left = parse_comparison(parser);

  while (check(parser, TOKEN_EQ) || check(parser, TOKEN_NEQ)) {
    advance(parser);
    Token op = parser->previous;
    Expr *right = parse_comparison(parser);

    Expr *expr = expr_new(EXPR_BINARY);
    expr->binary.op = op.type;
    expr->binary.left = left;
    expr->binary.right = right;
    left = expr;
  }

  return left;
}

Expr *parse_logical_and(Parser *parser) {
  Expr *left = parse_equality(parser);

  while (check(parser, TOKEN_AND_AND)) {
    advance(parser);
    Token op = parser->previous;
    Expr *right = parse_equality(parser);

    Expr *expr = expr_new(EXPR_BINARY);
    expr->binary.op = op.type;
    expr->binary.left = left;
    expr->binary.right = right;
    left = expr;
  }

  return left;
}

Expr *parse_logical_or(Parser *parser) {
  Expr *left = parse_logical_and(parser);

  while (check(parser, TOKEN_OR_OR)) {
    advance(parser);
    Token op = parser->previous;
    Expr *right = parse_logical_and(parser);

    Expr *expr = expr_new(EXPR_BINARY);
    expr->binary.op = op.type;
    expr->binary.left = left;
    expr->binary.right = right;
    left = expr;
  }

  return left;
}

Expr *parse_assignment(Parser *parser) {
  Expr *left = parse_logical_or(parser);

  if (check(parser, TOKEN_ASSIGN)) {
    advance(parser);
    Expr *value = parse_assignment(parser);

    if (left->type != EXPR_IDENTIFIER) {
      fprintf(stderr, "Error: invalid assigment target at line: %d.\n", parser->previous.line);
      exit(1);
    }

    Expr *expr = expr_new(EXPR_ASSIGN);
    expr->assign.target = left;
    expr->assign.value = value;
    return expr;
  }

  return left;
}

Expr *parse_expression(Parser *parser) {
  return parse_assignment(parser);
}

Stmt *stmt_new(StmtType type) {
  Stmt *stmt = malloc(sizeof(Stmt));
  memset(stmt, 0, sizeof(*stmt));
  stmt->type = type;

  return stmt;
}

void stmtlist_append(StmtList *stmtlist, Stmt *stmt) {
  if (stmtlist->count == stmtlist->capacity) {
    stmtlist->capacity = stmtlist->capacity == 0 ? 8 : stmtlist->capacity * 2;
    stmtlist->items = realloc(stmtlist->items, stmtlist->capacity * sizeof(Stmt *));
  }
  stmtlist->items[stmtlist->count++] = stmt;
}

Stmt *parse_return(Parser *parser) {
  expect(parser, TOKEN_RETURN, "expected `return`");

  Stmt *stmt = stmt_new(STMT_RETURN);

  if (!check(parser, TOKEN_SEMICOLON)) {
    stmt->return_stmt.value = parse_expression(parser);
  }

  expect(parser, TOKEN_SEMICOLON, "expected ';' after return statement");

  return stmt;
}

Stmt *parse_expr_statement(Parser *parser) {
  Expr *expr = parse_expression(parser);
  Stmt *stmt = stmt_new(STMT_EXPR);
  stmt->expr_stmt.expr = expr;
  
  expect(parser, TOKEN_SEMICOLON, "expected ';' after expression");

  return stmt;
}

TypeInfo parse_type(Parser *parser) {
  TypeInfo info;
  memset(&info, 0, sizeof(info));

  if (match(parser, TOKEN_INT)) {
    info.base = TYPE_INT;
  } else if (match(parser, TOKEN_CHAR)) {
    info.base = TYPE_CHAR;
  } else if (match(parser, TOKEN_VOID)) {
    info.base = TYPE_VOID;
  } else {
    fprintf(stderr, "Error: expected type at line: %d.\n", parser->current.line);
    exit(1);
  }

  while (match(parser, TOKEN_STAR)) {
    info.pointer_depth++;
  }

  return info;
}

Stmt *parse_var_decl(Parser *parser) {
  TypeInfo type = parse_type(parser);
  Token name = expect(parser, TOKEN_IDENTIFIER, "expected variable name");

  Stmt *stmt = stmt_new(STMT_VAR_DECL);
  stmt->var_decl.var_type = type;
  strncpy(stmt->var_decl.name, name.str, sizeof(stmt->var_decl.name) - 1);

  if (match(parser, TOKEN_ASSIGN)) {
    stmt->var_decl.init = parse_expression(parser);
  }

  expect(parser, TOKEN_SEMICOLON, "expected ';' after variable declaration");

  return stmt;
}

Stmt *parse_statement(Parser *parser);

Stmt *parse_block(Parser *parser) {
  expect(parser, TOKEN_LBRACE, "expected '{'");

  Stmt *stmt = stmt_new(STMT_BLOCK);

  while (!check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_EOF)) {
    Stmt *inner = parse_statement(parser);
    stmtlist_append(&stmt->block, inner);
  }

  expect(parser, TOKEN_RBRACE, "expected '}'");

  return stmt;
}

Stmt *parse_if(Parser *parser) {
  expect(parser, TOKEN_IF, "expected 'if'");
  expect(parser, TOKEN_LPAREN, "expected '(' after 'if'");
  Expr *condition = parse_expression(parser);
  expect(parser, TOKEN_RPAREN, "expected ')' after if condition");

  Stmt *stmt = stmt_new(STMT_IF);
  stmt->if_stmt.condition = condition;
  stmt->if_stmt.then_branch = parse_statement(parser);
  
  if (match(parser, TOKEN_ELSE)) {
    stmt->if_stmt.else_branch = parse_statement(parser);
  }

  return stmt;
}

Stmt *parse_while(Parser *parser) {
  expect(parser, TOKEN_WHILE, "expected 'while'");
  expect(parser, TOKEN_LPAREN, "expected '(' after 'while'");
  Expr *condition = parse_expression(parser);
  expect(parser, TOKEN_RPAREN, "expected ')' after while condition");

  Stmt *stmt = stmt_new(STMT_WHILE);
  stmt->while_stmt.condition = condition;
  stmt->while_stmt.body = parse_statement(parser);

  return stmt;
}

Stmt *parse_for(Parser *parser) {
  expect(parser, TOKEN_FOR, "expected 'for'");
  expect(parser, TOKEN_LPAREN, "expected '(' after 'for'");

  Stmt *init = NULL;
  if (check(parser, TOKEN_SEMICOLON)) {
    advance(parser);
  } else if (check(parser, TOKEN_INT) || check(parser, TOKEN_CHAR) || check(parser, TOKEN_VOID)) {
    init = parse_var_decl(parser);
  } else {
    init = parse_expr_statement(parser);
  }

  Expr *condition = NULL;
  if (!check(parser, TOKEN_SEMICOLON)) {
    condition = parse_expression(parser);
  }

  expect(parser, TOKEN_SEMICOLON, "expected ';' after for condition");

  Expr *increment = NULL;
  if (!check(parser, TOKEN_RPAREN)) {
    increment = parse_expression(parser);
  }

  expect(parser, TOKEN_RPAREN, "expected ')' after 'for'");

  Stmt *stmt = stmt_new(STMT_FOR);
  stmt->for_stmt.body = parse_statement(parser);
  stmt->for_stmt.init = init;
  stmt->for_stmt.condition = condition;
  stmt->for_stmt.increment = increment;

  return stmt;
}

Stmt *parse_statement(Parser *parser) {
  if (check(parser, TOKEN_LBRACE)) return parse_block(parser);
  if (check(parser, TOKEN_IF))     return parse_if(parser);
  if (check(parser, TOKEN_WHILE))  return parse_while(parser);
  if (check(parser, TOKEN_FOR))    return parse_for(parser);
  if (check(parser, TOKEN_RETURN)) return parse_return(parser);
  if (check(parser, TOKEN_INT) || check(parser, TOKEN_CHAR) || check(parser, TOKEN_VOID)) {
    return parse_var_decl(parser);
  }

  return parse_expr_statement(parser);
}

void paramlist_append(ParamList *paramlist, Param param) {
  if (paramlist->count == paramlist->capacity) {
    paramlist->capacity = paramlist->capacity == 0 ? 8 : paramlist->capacity * 2;
    paramlist->items = realloc(paramlist->items, paramlist->capacity * sizeof(Param));
  }
  paramlist->items[paramlist->count++] = param;
}

Function parse_function(Parser *parser) {
  TypeInfo return_type = parse_type(parser);
  Token name = expect(parser, TOKEN_IDENTIFIER, "expected function name");

  expect(parser, TOKEN_LPAREN, "expected '(' after function name");

  ParamList params;
  memset(&params, 0, sizeof(params));

  if (!check(parser, TOKEN_RPAREN)) {
    TypeInfo param_type = parse_type(parser);
    Token param_name = expect(parser, TOKEN_IDENTIFIER, "expected parameter name");

    Param param;
    memset(&param, 0, sizeof(param));
    param.type = param_type;
    strncpy(param.name, param_name.str, sizeof(param.name) - 1);
    paramlist_append(&params, param);

    while (match(parser, TOKEN_COMMA)) {
      param_type = parse_type(parser);
      param_name = expect(parser, TOKEN_IDENTIFIER, "expected parameter name");
      memset(&param, 0, sizeof(param));
      param.type = param_type;
      strncpy(param.name, param_name.str, sizeof(param.name) - 1);
      paramlist_append(&params, param);
    }
  }

  expect(parser, TOKEN_RPAREN, "expected ')' after parameters");

  Function fn;
  memset(&fn, 0, sizeof(fn));
  fn.return_type = return_type;
  strncpy(fn.name, name.str, sizeof(fn.name) - 1);
  fn.params = params;
  fn.body = parse_block(parser);

  return fn;
}

void program_append(Program *program, Function fn) {
  if (program->count == program->capacity) {
    program->capacity = program->capacity == 0 ? 8 : program->capacity * 2;
    program->items = realloc(program->items, program->capacity * sizeof(Function));
  }
  program->items[program->count++] = fn;
}

Program parse_program(Parser *parser) {
  Program program;
  memset(&program, 0, sizeof(program));

  while (!check(parser, TOKEN_EOF)) {
    Function fn = parse_function(parser);
    program_append(&program, fn);
  }

  return program;
}
