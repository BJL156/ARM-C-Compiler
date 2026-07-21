#include "lexer.h"

#include "ctype.h"
#include "string.h"

void lexer_init(Lexer *lexer, const char *src) {
  lexer->src = src;
  lexer->pos = 0;
  lexer->line = 1;
  lexer->col = 1;
}

char peek(Lexer *lexer) {
  return lexer->src[lexer->pos];
}

char peek_next(Lexer *lexer) {
  return lexer->src[lexer->pos + 1];
}

char advance(Lexer *lexer) {
  char c = lexer->src[lexer->pos++];
  if (c == '\n') {
    lexer->line++;
    lexer->col = 1;
  } else {
    lexer->col++;
  }

  return c;
}

void skip_whitespace(Lexer *lexer) {
  for (;;) {
    while (peek(lexer) == ' ' || peek(lexer) == '\t' || peek(lexer) == '\n') {
      advance(lexer);
    }

    if (peek(lexer) == '/' && peek_next(lexer) == '/') {
      while (peek(lexer) != '\n' && peek(lexer) != '\0') {
        advance(lexer);
      }
    } else if (peek(lexer) == '/' && peek_next(lexer) == '*') {
      advance(lexer);
      advance(lexer);
      while (!(peek(lexer) == '*' && peek_next(lexer) == '/') && peek(lexer) != '\0') {
        advance(lexer);
      }

      if (peek(lexer) != '\0') {
        advance(lexer);
        advance(lexer);
      }
    } else {
      break;
    }
  }
}

Token scan_int_literal(Lexer *lexer, Token *token) {
  int64_t value = 0;

  if (peek(lexer) == '0' && (peek_next(lexer) == 'x' || peek_next(lexer) == 'X')) {
    advance(lexer);
    advance(lexer);

    while (isxdigit(peek(lexer))) {
      char digit = advance(lexer);
      value = value * 16 + (isdigit(digit) ? digit - '0' : tolower(digit) - 'a' + 10);
    }
  } else {
    while (isdigit(peek(lexer))) {
      char digit = advance(lexer);
      value = value * 10 + (digit - '0');
    }
  }

  token->type = TOKEN_INT_LITERAL;
  token->imm = value;
  return *token;
}

Token scan_char_literal(Lexer *lexer, Token *token) {
  advance(lexer);

  char value;
  if (peek(lexer) == '\\') {
    advance(lexer);
    char escape_code = advance(lexer);
    switch (escape_code) {
      case 'n':  { value = '\n'; break; }
      case 't':  { value = '\t'; break; }
      case '\\': { value = '\\'; break; }
      case '\'': { value = '\''; break; }
      case '0':  { value = '\0'; break; }
      default:   { value = escape_code; break; }
    }
  } else {
    value = advance(lexer);
  }

  advance(lexer);

  token->type = TOKEN_CHAR_LITERAL;
  token->imm = value;
  return *token;
}

typedef struct {
  const char *name;
  TokenType type;
} Keyword;

static const Keyword keywords[] = {
  { "int",    TOKEN_INT    },
  { "char",   TOKEN_CHAR   },
  { "void",   TOKEN_VOID   },
  { "if",     TOKEN_IF     },
  { "else",   TOKEN_ELSE   },
  { "while",  TOKEN_WHILE  },
  { "for",    TOKEN_FOR    },
  { "return", TOKEN_RETURN }
};

TokenType lookup_keyword(const char *str) {
  int keywords_count = sizeof(keywords) / sizeof(keywords[0]);
  for (int i = 0; i < keywords_count; i++) {
    if (strcmp(str, keywords[i].name) == 0) {
      return keywords[i].type;
    }
  }

  return TOKEN_IDENTIFIER;
}

Token scan_identifier(Lexer *lexer, Token *token) {
  int i = 0;
  while (isalnum(peek(lexer)) || peek(lexer) == '_') {
    token->str[i++] = advance(lexer);
  }
  token->str[i] = '\0';

  token->type = lookup_keyword(token->str);
  return *token;
}

Token scan_operator(Lexer *lexer, Token *token) {
  char c = advance(lexer);
  switch (c) {
    case '(': { token->type = TOKEN_LPAREN;    break; }
    case ')': { token->type = TOKEN_RPAREN;    break; }
    case '{': { token->type = TOKEN_LBRACE;    break; }
    case '}': { token->type = TOKEN_RBRACE;    break; }
    case '[': { token->type = TOKEN_LBRACKET;  break; }
    case ']': { token->type = TOKEN_RBRACKET;  break; }
    case ';': { token->type = TOKEN_SEMICOLON; break; }
    case ',': { token->type = TOKEN_COMMA;     break; }
    case '+': { token->type = TOKEN_PLUS;      break; }
    case '-': { token->type = TOKEN_MINUS;     break; }
    case '*': { token->type = TOKEN_STAR;      break; }
    case '/': { token->type = TOKEN_SLASH;     break; }
    case '%': { token->type = TOKEN_PERCENT;   break; }
    case '=': {
      if (peek(lexer) == '=') {
        advance(lexer);
        token->type = TOKEN_EQ;
      } else {
        token->type = TOKEN_ASSIGN;
      }

      break;
    }
    case '!': {
      if (peek(lexer) == '=') {
        advance(lexer);
        token->type = TOKEN_NEQ;
      } else {
        token->type = TOKEN_NOT;
      }

      break;
    }
    case '<': {
      if (peek(lexer) == '=') {
        advance(lexer);
        token->type = TOKEN_LE;
      } else {
        token->type = TOKEN_LT;
      }

      break;
    }
    case '>': {
      if (peek(lexer) == '=') {
        advance(lexer);
        token->type = TOKEN_GE;
      } else {
        token->type = TOKEN_GT;
      }

      break;
    }
    case '&': {
      if (peek(lexer) == '&') {
        advance(lexer);
        token->type = TOKEN_AND_AND;
      } else {
        token->type = TOKEN_AMP;
      }

      break;
    }
    case '|': {
      if (peek(lexer) == '|') {
        advance(lexer);
        token->type = TOKEN_OR_OR;
      } else {
        token->type = TOKEN_UNKNOWN;
        token->str[0] = c;
        token->str[1] = '\0';
      }

      break;
    }
    default: {
      token->type = TOKEN_UNKNOWN;
      token->str[0] = c;
      token->str[1] = '\0';
      break;
    }
  }

  return *token;
}

Token scan_eof(Token *token) {
  token->type = TOKEN_EOF;
  return *token;
}

Token next_token(Lexer *lexer) {
  skip_whitespace(lexer);

  Token token;
  memset(&token, 0, sizeof(token));
  token.line = lexer->line;
  token.col = lexer->col;

  char c = peek(lexer);
  if (c == '\0')  return scan_eof(&token);
  if (isdigit(c)) return scan_int_literal(lexer, &token);
  if (c == '\'')  return scan_char_literal(lexer, &token);
  if (isalpha(c) || c == '_') return scan_identifier(lexer, &token);

  return scan_operator(lexer, &token);
}
