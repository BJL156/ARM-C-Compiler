#include "parser.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void parser_init(Parser *parser, Lexer *lexer) {
  memset(parser, 0, sizeof(*parser));
  parser->lexer = lexer;
  parser->current = next_token(lexer);
}

void advance(Parser *parser) {
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

Program parse_program(Parser *parser) {

}
