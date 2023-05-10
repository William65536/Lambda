#ifndef TOKENLIST_H
#define TOKENLIST_H

#include <stddef.h>
#include <stdbool.h>
#include "Token.h"

typedef struct TokenList TokenList;

TokenList *TokenList_new(size_t initcap);

void TokenList_delete(TokenList **self);

size_t TokenList_size(const TokenList *self);

Token TokenList_get(const TokenList *self, size_t i);

const Token *TokenList_begin(const TokenList *self);

const Token *TokenList_end(const TokenList *self);

bool TokenList_push(TokenList **self, Token token);

void TokenList_println(const TokenList *self);

#endif
