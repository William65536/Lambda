#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <setjmp.h>
#include <string.h>

#include "Token.h"
#include "TokenList.h"
#include "Parser.h"

struct ASTNode {
    enum ASTNodeType type;
    Position pos; /* TODO: Remove this if it is superfluous */
    union {
        Identifier identifier;
        Variable variable;
    };
    ASTNode *left, *right;
};

/* TODO: Implement error-handling */
/* TODO: Make a function that converts a `Token` to an `ASTNode` */

void ASTNode_delete_all(ASTNode **self)
{
    assert(self != NULL);

    if (*self == NULL)
        return;

    ASTNode_delete_all(&(*self)->left);
    ASTNode_delete_all(&(*self)->right);

    free(*self);
    *self = NULL;
}

void ASTNode_delete_all_children(ASTNode self)
{
    ASTNode_delete_all(&self.left);
    ASTNode_delete_all(&self.right);
}

static bool Parser_parse_expression(ASTNode *self, const Token **itr, jmp_buf envbuf);

static bool Parser_parse_expression_group(ASTNode *self, const Token **itr, jmp_buf envbuf)
{
    assert(self != NULL);
    assert(itr != NULL && *itr != NULL);
    assert(envbuf != NULL);

    const Token *start = *itr;

    if ((*itr)->type != TOKEN_TYPE_LROUND)
        return false;

    ++*itr;

    if (!Parser_parse_expression(self, itr, envbuf)) {
        *itr = start;
        return false;
    }

    if ((*itr)->type != TOKEN_TYPE_RROUND) {
        ASTNode_delete_all_children(*self);
        *itr = start;
        return false;
    }

    ++*itr;

    return true;
}

static bool Parser_parse_atom(ASTNode *self, const Token **itr, jmp_buf envbuf)
{
    assert(self != NULL);
    assert(itr != NULL && *itr != NULL);
    assert(envbuf != NULL);

    if ((*itr)->type == TOKEN_TYPE_VARIABLE) {
        strcpy(self->variable, (*itr)->variable);
        self->pos = (*itr)->pos;
        self->type = ASTNODE_TYPE_VARIABLE;
        self->left = self->right = NULL;

        ++*itr;

        return true;
    }

    if ((*itr)->type == TOKEN_TYPE_IDENTIFIER) {
        strcpy(self->identifier, (*itr)->identifier);
        self->pos = (*itr)->pos;
        self->type = ASTNODE_TYPE_IDENTIFIER;
        self->left = self->right = NULL;

        ++*itr;

        return true;
    }

    if (Parser_parse_expression_group(self, itr, envbuf))
        return true;

    return false;
}

static bool Parser_parse_application_high(ASTNode *self, const Token **itr, jmp_buf envbuf)
{
    assert(self != NULL);
    assert(itr != NULL && *itr != NULL);
    assert(envbuf != NULL);

    ASTNode templeft;

    if (!Parser_parse_atom(&templeft, itr, envbuf))
        return false;

    ASTNode tempright;

    if (!Parser_parse_atom(&tempright, itr, envbuf)) {
        *self = templeft;
        return true;
    }

    self->left = malloc(sizeof *self->left);

    if (self->left == NULL)
        longjmp(envbuf, 1);

    self->right = malloc(sizeof *self->right);

    if (self->right == NULL)
        longjmp(envbuf, 1);

    self->pos = self->left->pos; /* TODO: Figure out a good position */
    self->type = ASTNODE_TYPE_APPLICATION;
    *self->left = templeft;
    *self->right = tempright;

    Position currpos = self->right->pos; /* TODO: Figure out a good position */

    while (true) {
        ASTNode tempright;

        if (!Parser_parse_atom(&tempright, itr, envbuf))
            break;

        ASTNode temp;

        temp.left = malloc(sizeof *self->left);

        if (temp.left == NULL)
            longjmp(envbuf, 1);

        temp.right = malloc(sizeof *self->right);

        if (temp.right == NULL) {
            free(temp.left);
            longjmp(envbuf, 1);
        }

        temp.pos = currpos;
        temp.type = ASTNODE_TYPE_APPLICATION;
        *temp.left = *self;
        *temp.right = tempright;

        *self = temp;

        currpos = self->right->pos; /* TODO: Figure out a good position */
    }

    return true;
}

static bool Parser_parse_lambda(ASTNode *self, const Token **itr, jmp_buf envbuf)
{
    assert(self != NULL);
    assert(itr != NULL && *itr != NULL);
    assert(envbuf != NULL);

    const Token *start = *itr;

    Position pos = (*itr)->pos;

    if (Parser_parse_application_high(self, itr, envbuf)) {
        if (self->type != ASTNODE_TYPE_VARIABLE)
            return true;
        else
            *itr = start;
    }

    if ((*itr)->type != TOKEN_TYPE_VARIABLE) {
        *itr = start;
        return false;
    }

    ASTNode tempvariable;
    strcpy(tempvariable.variable, (*itr)->variable);
    tempvariable.pos = (*itr)->pos;
    tempvariable.type = ASTNODE_TYPE_VARIABLE;
    tempvariable.left = tempvariable.right = NULL;

    ++*itr;

    if ((*itr)->type != TOKEN_TYPE_ARROW) {
        *self = tempvariable;
        return true;
    }

    ++*itr;

    ASTNode tempexpression;

    if (!Parser_parse_application_high(&tempexpression, itr, envbuf)) {
        *itr = start;
        return false;
    }

    self->left = malloc(sizeof *self->left);

    if (self->left == NULL) {
        ASTNode_delete_all_children(tempexpression);
        longjmp(envbuf, 1);
    }

    self->right = malloc(sizeof *self->right);

    if (self->right == NULL) {
        free(self->left);
        self->left = NULL;
        ASTNode_delete_all_children(tempexpression);
        longjmp(envbuf, 1);
    }

    self->pos = pos;
    self->type = ASTNODE_TYPE_LAMDBA;
    *self->left = tempvariable;
    *self->right = tempexpression;

    return true;
}

static bool Parser_parse_application_low(ASTNode *self, const Token **itr, jmp_buf envbuf)
{
    assert(self != NULL);
    assert(itr != NULL && *itr != NULL);
    assert(envbuf != NULL);

    const Token *start = *itr;

    const Position pos = (*itr)->pos;

    ASTNode templeft;

    if (!Parser_parse_lambda(&templeft, itr, envbuf))
        return false;

    if ((*itr)->type != TOKEN_TYPE_AT) {
        *self = templeft;
        return true;
    }

    ++*itr;

    ASTNode tempright;

    if (!Parser_parse_application_low(&tempright, itr, envbuf)) {
        ASTNode_delete_all_children(templeft);
        *itr = start;
        return false;
    }

    self->left = malloc(sizeof *self->left);

    if (self->left == NULL) {
        ASTNode_delete_all_children(templeft);
        ASTNode_delete_all_children(tempright);
        longjmp(envbuf, 1);
    }

    self->right = malloc(sizeof *self->right);

    if (self->right == NULL) {
        free(self->left);
        self->left = NULL;
        ASTNode_delete_all_children(templeft);
        ASTNode_delete_all_children(tempright);
        longjmp(envbuf, 1);
    }

    self->pos = pos;
    self->type = ASTNODE_TYPE_APPLICATION;
    *self->left = templeft;
    *self->right = tempright;

    return true;
}

static bool Parser_parse_expression(ASTNode *self, const Token **itr, jmp_buf envbuf)
{
    assert(self != NULL);
    assert(itr != NULL && *itr != NULL);

    return Parser_parse_application_low(self, itr, envbuf);
}

static bool Parser_parse_assignment(ASTNode *self, const Token **itr, jmp_buf envbuf)
{
    assert(self != NULL);
    assert(itr != NULL && *itr != NULL);
    assert(envbuf != NULL);

    const Token *start = *itr;

    Position pos = (*itr)->pos;

    if ((*itr)->type != TOKEN_TYPE_IDENTIFIER)
        return false;

    ASTNode tempidentifier;

    {
        strcpy(tempidentifier.identifier, (*itr)->identifier);
        tempidentifier.pos = (*itr)->pos;
        tempidentifier.type = ASTNODE_TYPE_IDENTIFIER;
        tempidentifier.left = tempidentifier.right = NULL;
    }

    ++*itr;

    if ((*itr)->type != TOKEN_TYPE_WALRUS) {
        *itr = start;
        return false;
    }

    ++*itr;

    ASTNode tempexpression;

    if (!Parser_parse_expression(&tempexpression, itr, envbuf)) {
        ++*itr;
        return false;
    }

    self->left = malloc(sizeof *self->left);

    if (self->left == NULL) {
        ASTNode_delete_all_children(tempexpression);
        longjmp(envbuf, 1);
    }

    self->right = malloc(sizeof *self->right);

    if (self->right == NULL) {
        free(self->left);
        self->left = NULL;
        ASTNode_delete_all_children(tempexpression);
        longjmp(envbuf, 1);
    }

    self->pos = pos;
    self->type = ASTNODE_TYPE_ASSIGNMENT;
    *self->left = tempidentifier;
    *self->right = tempexpression;

    return true;
}

static bool Parser_parse_statement(ASTNode *self, const Token **itr, jmp_buf envbuf)
{
    assert(self != NULL);
    assert(itr != NULL && *itr != NULL);
    assert(envbuf != NULL);

    const Token *start = *itr;

    if (Parser_parse_assignment(self, itr, envbuf))
        goto parsedot;

    if (Parser_parse_expression(self, itr, envbuf))
        goto parsedot;

    self->pos = (*itr)->pos;
    self->type = ASTNODE_TYPE_EMPTY;
    self->left = self->right = NULL;

    parsedot:

    if ((*itr)->type != TOKEN_TYPE_DOT) {
        ASTNode_delete_all_children(*self);
        *itr = start;
        return false;
    }

    ++*itr;

    return true;
}

static bool Parser_parse_program(ASTNode *self, const Token **itr, jmp_buf envbuf)
{
    assert(self != NULL);
    assert(itr != NULL && *itr != NULL);
    assert(envbuf != NULL);

    const Token *start = *itr;

    ASTNode *curr = self;

    Position currpos = (*itr)->pos;

    while (true) {
        ASTNode tempstatement;

        if (!Parser_parse_statement(&tempstatement, itr, envbuf))
            goto parse_eof;

        curr->left = malloc(sizeof *self->left);

        if (curr->left == NULL) {
            ASTNode_delete_all_children(tempstatement);
            longjmp(envbuf, 1);
        }

        curr->right = malloc(sizeof *self->right);

        if (curr->right == NULL) {
            free(curr->left);
            curr->left = NULL;
            ASTNode_delete_all_children(tempstatement);
            longjmp(envbuf, 1);
        }

        curr->pos = currpos;
        curr->type = ASTNODE_TYPE_PROGRAM;
        *curr->left = tempstatement;

        curr = curr->right;

        currpos = (*itr)->pos;
    }

    parse_eof:

    if ((*itr)->type != TOKEN_TYPE_EOF) {
        *itr = start;
        return false;
    }

    curr->pos = currpos;
    curr->type = ASTNODE_TYPE_EMPTY;
    curr->left = curr->right = NULL;

    return true;
}

enum ParserErrorCode Parser_parse(const TokenList *tokens, ASTNode **ret)
{
    assert(tokens != NULL);

    const Token *itr = TokenList_begin(tokens);

    *ret = malloc(sizeof **ret);

    if (*ret == NULL)
        return PARSER_ERROR_FAILED_ALLOCATION;

    jmp_buf envbuf;

    if (setjmp(envbuf) != 0) {
        ASTNode_delete_all(ret);
        return PARSER_ERROR_FAILED_ALLOCATION;
    }

    if (!Parser_parse_program(*ret, &itr, envbuf)) {
        free(*ret);
        return PARSER_ERROR_FAILED_PARSE;
    }

    return PARSER_ERROR_NONE;
}

static void ASTNode_print(const ASTNode *self)
{
    assert(self != NULL);

    switch (self->type) {
        case ASTNODE_TYPE_IDENTIFIER:
            fputs(self->identifier, stdout);
            break;
        case ASTNODE_TYPE_VARIABLE:
            fputs(self->variable, stdout);
            break;
        case ASTNODE_TYPE_LAMDBA:
            putchar('(');
            ASTNode_print(self->left);
            fputs(" -> ", stdout);
            ASTNode_print(self->right);
            putchar(')');
            break;
        case ASTNODE_TYPE_APPLICATION:
            putchar('(');
            ASTNode_print(self->left);
            putchar(' ');
            ASTNode_print(self->right);
            putchar(')');
            break;
        case ASTNODE_TYPE_ASSIGNMENT:
            ASTNode_print(self->left);
            fputs(" := ", stdout);
            ASTNode_print(self->right);
            break;
        case ASTNODE_TYPE_EMPTY:
            break;
        case ASTNODE_TYPE_PROGRAM:
            ASTNode_print(self->left);
            putchar('.');
            puts("");
            ASTNode_print(self->right);
            break;
        default:
            assert(0 && "Unreachable!");
    }
}

void ASTNode_println(const ASTNode *self)
{
    assert(self != NULL);

    ASTNode_print(self);
    puts("");
}
