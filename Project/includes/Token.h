#ifndef TOKEN_H
#define TOKEN_H

#define TOKEN_IDENTIFIER_MAX_SIZE 32
#define TOKEN_VARIABLE_MAX_SIZE 32

#include <stddef.h>

typedef struct Position {
    size_t ln, col;
} Position;

typedef char Identifier[TOKEN_IDENTIFIER_MAX_SIZE + 1];
typedef char Variable[TOKEN_VARIABLE_MAX_SIZE + 1];

enum TokenType {
    TOKEN_TYPE_LROUND,
    TOKEN_TYPE_RROUND,
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_VARIABLE,
    TOKEN_TYPE_DOT,
    TOKEN_TYPE_EOF,
    TOKEN_TYPE_WALRUS,
    TOKEN_TYPE_AT,
    TOKEN_TYPE_ARROW
};

typedef struct Token {
    Position pos;
    union {
        Identifier identifier;
        Variable variable;
    };
    enum TokenType type;
} Token;

#endif
