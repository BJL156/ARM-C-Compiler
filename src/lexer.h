#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  TOKEN_INT,
  TOKEN_CHAR,
  TOKEN_VOID,
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_WHILE,
  TOKEN_FOR,
  TOKEN_RETURN,
  TOKEN_IDENTIFIER,
  TOKEN_INT_LITERAL,
  TOKEN_CHAR_LITERAL,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_LBRACE,
  TOKEN_RBRACE,
  TOKEN_LBRACKET,
  TOKEN_RBRACKET,
  TOKEN_SEMICOLON,
  TOKEN_COMMA,
  TOKEN_ASSIGN,
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_STAR,
  TOKEN_SLASH,
  TOKEN_PERCENT,
  TOKEN_EQ,
  TOKEN_NEQ,
  TOKEN_LT,
  TOKEN_LE,
  TOKEN_GT,
  TOKEN_GE,
  TOKEN_AND_AND,
  TOKEN_OR_OR,
  TOKEN_NOT,
  TOKEN_AMP,
  TOKEN_EOF,
  TOKEN_UNKNOWN
} TokenType;

typedef struct {
  TokenType type;
  union {
    char str[64];
    int64_t imm;
  };
  int line;
  int col;
} Token;

typedef struct {
  const char *src;
  int pos;
  int line;
  int col;
} Lexer;

void lexer_init(Lexer *lexer, const char *src);
Token next_token(Lexer *lexer);

#endif
