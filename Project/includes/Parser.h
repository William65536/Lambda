#ifndef PARSER_H
#define PARSER_H

#include "TokenList.h"

enum ParserErrorCode {
    PARSER_ERROR_NONE,
    PARSER_ERROR_FAILED_ALLOCATION,
    PARSER_ERROR_FAILED_PARSE
};

enum ASTNodeType {
    ASTNODE_TYPE_IDENTIFIER,
    ASTNODE_TYPE_VARIABLE,
    ASTNODE_TYPE_LAMDBA,
    ASTNODE_TYPE_APPLICATION,
    ASTNODE_TYPE_ASSIGNMENT,
    ASTNODE_TYPE_EMPTY,
    ASTNODE_TYPE_PROGRAM
};

typedef struct ASTNode ASTNode;

void ASTNode_delete_all(ASTNode **self);

enum ParserErrorCode Parser_parse(const TokenList *tokens, ASTNode **ret);

void ASTNode_println(const ASTNode *self);

#endif
