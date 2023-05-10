#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "includes\TokenList.h"
#include "includes\Scanner.h"
#include "includes\Parser.h"

/* TODO: Ensure that memory allocation is correct */

// typedef struct String {
//     size_t size, cap;
//     char *str_ref;
//     char small[31 + 1];
// } String;

/* Coda programming language */
/* Learn DirectX */

int main(void)
{
    const char *input = "I := x -> x.";

    /* TODO: Use arena allocation to make memory management easier */

    /* Dynamic strings */
    /* Ring buffers */

    TokenList *tokenlist;

    {
        ScannerError scannererror;

        if ((scannererror = Scanner_scan(input, &tokenlist)).code != SCANNER_ERROR_NONE) {
            switch (scannererror.code) {
                case SCANNER_ERROR_UNRECOGNIZED_TOKEN:
                    fprintf(stderr, "LEX ERROR [Ln %zu, Col %zu]: Unrecognized token `%c`\n", scannererror.pos.ln + 1, scannererror.pos.col + 1, scannererror.c);
                    break;
                case SCANNER_ERROR_UNEXPECTED_TOKEN:
                    fprintf(stderr, "LEX ERROR [Ln %zu, Col %zu]: Unexpected token `%c`\n", scannererror.pos.ln + 1, scannererror.pos.col + 1, scannererror.c);
                    break;
                case SCANNER_ERROR_FAILED_ALLOCATION:
                    fprintf(stderr, "SYSTEM ERROR: Failed allocation\n");
                    break;
                default:
                    assert(0 && "Unreachable!");
            }

            return EXIT_FAILURE;
        }
    }

    ASTNode *ast;

    {
        enum ParserErrorCode parsererror;

        if ((parsererror = Parser_parse(tokenlist, &ast)) != PARSER_ERROR_NONE) {
            switch (parsererror) {
                case PARSER_ERROR_FAILED_ALLOCATION:
                    fputs("SYSTEM ERROR: Failed allocation\n", stderr);
                    break;
                case PARSER_ERROR_FAILED_PARSE:
                    fputs("PARSER ERROR: Failed parse\n", stderr);
                    break;
                default:
                    assert(0 && "Unreachable!");
            }

            TokenList_delete(&tokenlist);

            return EXIT_FAILURE;
        }
    }

    ASTNode_println(ast);

    TokenList_delete(&tokenlist);

    ASTNode_delete_all(&ast);

    return 0;
}
