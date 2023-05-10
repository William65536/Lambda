#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include "Token.h"
#include "TokenList.h"

struct TokenList {
    size_t size, cap;
    Token tokens[];
};


TokenList *TokenList_new(size_t initcap)
{
    assert(initcap > 0);

    TokenList *ret = malloc(sizeof *ret + initcap * sizeof *ret->tokens);

    if (ret == NULL)
        return NULL;

    ret->cap = initcap;
    ret->size = 0;

    return ret;
}

void TokenList_delete(TokenList **self)
{
    assert(self != NULL && *self != NULL);

    free(*self);
    *self = NULL;
}

size_t TokenList_size(const TokenList *self)
{
    assert(self != NULL);

    return self->size;
}

Token TokenList_get(const TokenList *self, size_t i)
{
    assert(self != NULL);
    assert(i < self->size);

    return self->tokens[i];
}

const Token *TokenList_begin(const TokenList *self)
{
    assert(self != NULL);
    assert(self->size > 0);

    return self->tokens;
}

const Token *TokenList_end(const TokenList *self)
{
    assert(self != NULL);
    assert(self->size > 0);

    return self->tokens + self->size;
}

bool TokenList_resize(TokenList **self)
{
    assert(self != NULL && *self != NULL);

    TokenList *temp = realloc(*self, sizeof **self + 2 * (*self)->cap * sizeof *(*self)->tokens);

    if (temp == NULL)
        return false;

    *self = temp;
    (*self)->cap *= 2;

    return true;
}

bool TokenList_push(TokenList **self, Token token)
{
    assert(self != NULL);
    assert((*self)->cap > 0);

    if ((*self)->size + 1 > (*self)->cap)
        if (!TokenList_resize(self))
            return false;

    (*self)->tokens[(*self)->size] = token;
    (*self)->size++;

    return true;
}

static void Token_print(Token self)
{
    switch (self.type) {
        case TOKEN_TYPE_LROUND:
            fputs("<LPAREN `(`>", stdout);
            break;
        case TOKEN_TYPE_RROUND:
            fputs("<LPAREN `)`>", stdout);
            break;
        case TOKEN_TYPE_IDENTIFIER:
            printf("<IDENTIFIER `%s`>", self.identifier);
            break;
        case TOKEN_TYPE_VARIABLE:
            printf("<VARIABLE `%s`>", self.variable);
            break;
        case TOKEN_TYPE_DOT:
            fputs("<LPAREN `.`>", stdout);
            break;
        case TOKEN_TYPE_EOF:
            fputs("<EOF>", stdout);
            break;
        case TOKEN_TYPE_WALRUS:
            fputs("<ASSIGN `:=`>", stdout);
            break;
        case TOKEN_TYPE_AT:
            fputs("<APPLY `@`>", stdout);
            break;
        case TOKEN_TYPE_ARROW:
            fputs("<ARROW `->`>", stdout);
            break;
        default:
            assert(0 && "Unreachable");
    }
}

void TokenList_println(const TokenList *self)
{
    assert(self != NULL);

    printf("(%zu) [ ", self->size);

    for (size_t i = 0; i < self->size; i++) {
        Token_print(self->tokens[i]);
        putchar(' ');
    }

    putchar(']');
    puts("");
}
