#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"

typedef struct {
  Lexer *lexer;
  Token current;
  Token previous;
} Parser;

void parser_init(Parser *parser, Lexer *lexer);
Program parse_program(Parser *parser);

#endif
