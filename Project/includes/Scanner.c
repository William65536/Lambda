#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "Token.h"
#include "TokenList.h"
#include "Scanner.h"

typedef struct Scanner {
    Position pos;
    const char *input;
    const char *itr;
    TokenList *tokens;
} Scanner;

static Scanner *Scanner_new(const char *input)
{
    assert(input != NULL);

    Scanner *ret = malloc(sizeof *ret);

    if (ret == NULL)
        return NULL;

    ret->input = input;
    ret->itr = ret->input;

    ret->pos = (Position) { .ln = 0, .col = 0 };
    ret->tokens = TokenList_new(20); /* TODO: Make the initial capacity based off input's size */

    if (ret->tokens == NULL) {
        free(ret);
        return NULL;
    }

    return ret;
}

static char Scanner_incr(Scanner *self)
{
    assert(self != NULL);
    assert(self->itr != NULL);

    if (*self->itr == '\0')
        return *self->itr;

    if (*self->itr == '\r') {
        return *self->itr++;
    }

    if (*self->itr == '\n') {
        self->pos.col = 0;
        self->pos.ln++;
        return *self->itr++;
    }

    self->pos.col++;
    return *self->itr++;
}

static void Scanner_skipwhite(Scanner *self)
{
    assert(self != NULL);
    assert(self->itr != NULL);

    for (; *self->itr == ' ' || *self->itr == '\t' || *self->itr == '\n' || *self->itr == '\r'; Scanner_incr(self));
}

static bool isloweralpha(char c)
{
    return 'a' <= c && c <= 'z';
}

static bool isupperalpha(char c)
{
    return 'A' <= c && c <= 'Z';
}

static bool isdigit(char c)
{
    return '0' <= c && c <= '9';
}

#include <stdio.h>

ScannerError Scanner_scan(const char *input, TokenList **ret)
{
    assert(input != NULL);

    Scanner *scanner = Scanner_new(input); /* TODO: `scanner` does not need to be allocated on the heap */

    if (scanner == NULL)
        return (ScannerError) { .code = SCANNER_ERROR_FAILED_ALLOCATION };

    enum ScannerErrorCode errorcode = SCANNER_ERROR_NONE;

    /* TODO: Enable this to throw multiple errors */
    while (errorcode == SCANNER_ERROR_NONE) {
        Scanner_skipwhite(scanner);

        Token token;

        switch (*scanner->itr) {
            case '#':
                for (; *scanner->itr != '\0' && *scanner->itr != '\n'; Scanner_incr(scanner));

                continue;
            case '\0':
                token.type = TOKEN_TYPE_EOF;
                token.pos = scanner->pos;
                Scanner_incr(scanner);
                break;
            case '(':
                token.type = TOKEN_TYPE_LROUND;
                token.pos = scanner->pos;
                Scanner_incr(scanner);
                break;
            case ')':
                token.type = TOKEN_TYPE_RROUND;
                token.pos = scanner->pos;
                Scanner_incr(scanner);
                break;
            case '.':
                token.type = TOKEN_TYPE_DOT;
                token.pos = scanner->pos;
                Scanner_incr(scanner);
                break;
            case '@':
                token.type = TOKEN_TYPE_AT;
                token.pos = scanner->pos;
                Scanner_incr(scanner);
                break;
            case '-':
                Scanner_incr(scanner);

                if (*scanner->itr != '>') {
                    errorcode = SCANNER_ERROR_UNEXPECTED_TOKEN;
                    continue;
                }

                token.type = TOKEN_TYPE_ARROW;
                token.pos = scanner->pos;
                Scanner_incr(scanner);
                break;
            case ':':
                Scanner_incr(scanner);

                if (*scanner->itr != '=') {
                    errorcode = SCANNER_ERROR_UNEXPECTED_TOKEN;
                    continue;
                }

                token.type = TOKEN_TYPE_WALRUS;
                token.pos = scanner->pos;
                Scanner_incr(scanner);
                break;
            default:
                if (isloweralpha(*scanner->itr)) {
                    token.type = TOKEN_TYPE_VARIABLE;
                    token.pos = scanner->pos;

                    size_t i;
                    for (i = 0; i < TOKEN_VARIABLE_MAX_SIZE && (isloweralpha(*scanner->itr) || isupperalpha(*scanner->itr) || *scanner->itr == '_'); i++, Scanner_incr(scanner))
                        token.variable[i] = *scanner->itr;

                    token.variable[i] = '\0';

                    for (; isloweralpha(*scanner->itr) || isupperalpha(*scanner->itr) || isdigit(*scanner->itr) || *scanner->itr == '_'; Scanner_incr(scanner));

                    break;
                }

                if (isupperalpha(*scanner->itr)) {
                    token.type = TOKEN_TYPE_IDENTIFIER;
                    token.pos = scanner->pos;

                    size_t i;
                    for (i = 0; i < TOKEN_IDENTIFIER_MAX_SIZE && (isloweralpha(*scanner->itr) || isupperalpha(*scanner->itr) || *scanner->itr == '_'); i++, Scanner_incr(scanner))
                        token.identifier[i] = *scanner->itr;

                    token.identifier[i] = '\0';

                    for (; isloweralpha(*scanner->itr) || isupperalpha(*scanner->itr) || isdigit(*scanner->itr) || *scanner->itr == '_'; Scanner_incr(scanner));

                    break;
                }

                errorcode = SCANNER_ERROR_UNRECOGNIZED_TOKEN;

                continue;
        }

        if (!TokenList_push(&scanner->tokens, token)) {
            errorcode = SCANNER_ERROR_FAILED_ALLOCATION;
            continue;
        }

        if (token.type == TOKEN_TYPE_EOF)
            break;
    }

    Position pos = scanner->pos;
    char c = *scanner->itr;

    if (errorcode != SCANNER_ERROR_NONE) {
        TokenList_delete(&scanner->tokens);
        *ret = NULL;
    } else {
        *ret = scanner->tokens;
    }

    free(scanner);

    return (ScannerError) { .code = errorcode, .pos = pos, .c = c };
}
