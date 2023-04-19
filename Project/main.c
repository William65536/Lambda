#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

typedef enum ErrorCode {
    ERROR_NONE,
    ERROR_FAILED_ALLOCATION,
    ERROR_FAILED_PARSE
} ErrorCode;

typedef struct Expression Expression;
struct Expression {
    enum {
        TYPE_VARIABLE,
        TYPE_LAMBDA,
        TYPE_APPLICATION
    } type;
    union {
        struct {
            char var;
            Expression *next;
        };
        struct {
            Expression *args, *expr;
        };
        struct {
            Expression *left, *right;
        };
    };
};

static void skipwhite(const char **itr)
{
    assert(itr != NULL && *itr != NULL);

    for (; **itr == ' ' || **itr == '\t' || **itr == '\n' || **itr == '\r'; ++*itr);
}

void Expression_delete(Expression **self)
{
    assert(self != NULL && *self != NULL);

    assert(0 && "TODO");
}

void Expression_print(const Expression *self)
{
    assert(self != NULL);

    switch (self->type) {
        case TYPE_VARIABLE:
            putchar(self->var);
            if (self->next != NULL)
                Expression_print(self->next);
            break;
        case TYPE_APPLICATION:
            putchar('(');
            assert(self->left != NULL);
            Expression_print(self->left);
            putchar(' ');
            assert(self->right != NULL);
            Expression_print(self->right);
            putchar(')');
            break;
        case TYPE_LAMBDA:
            putchar('^');
            assert(self->args != NULL);
            Expression_print(self->args);
            putchar('.');
            assert(self->expr != NULL);
            Expression_print(self->expr);
            break;
        default:
            assert(0 && "Unreachable");
    }
}

void Expression_println(const Expression *self)
{
    assert(self != NULL);

    Expression_print(self);
    puts("");
}

static inline bool isloweralpha(char c)
{
    return 'a' <= c && c <= 'z'; /* Assumes ASCII */
}

static ErrorCode Expression_parse_variable(Expression *self, const char **itr)
{
    assert(self != NULL);
    assert(itr != NULL && *itr != NULL);

    if (!isloweralpha(**itr))
        return ERROR_FAILED_PARSE;

    self->type = TYPE_VARIABLE;
    self->var = **itr;
    self->next = NULL;

    ++*itr;

    return ERROR_NONE;
}

static ErrorCode Expression_parse_expression(Expression *self, const char **itr);

static ErrorCode Expression_parse_primary(Expression *self, const char **itr)
{
    assert(self != NULL);
    assert(itr != NULL && *itr != NULL);

    const char *start = *itr;

    if (Expression_parse_variable(self, itr) == ERROR_NONE)
        return ERROR_NONE;

    {
        if (**itr != '(')
            goto exit;

        ++*itr;

        ErrorCode errorcode = Expression_parse_expression(self, itr);

        /* TODO: Do I have to do a memory cleanup on failure? */
        if (errorcode == ERROR_FAILED_PARSE)
            goto exit;

        if (errorcode == ERROR_FAILED_ALLOCATION)
            return ERROR_FAILED_ALLOCATION;

        if (**itr != ')')
            goto exit;

        ++*itr;

        return ERROR_NONE;

        exit:;
    }

    *itr = start;

    return ERROR_FAILED_PARSE;
}

/* static */ ErrorCode Expression_parse_lambda(Expression *self, const char **itr)
{
    assert(self != NULL);
    assert(itr != NULL && *itr != NULL);

    const char *start = *itr;

    if (**itr != '^')
        return ERROR_FAILED_PARSE;

    ++*itr;

    Expression *args = NULL;

    Expression *curr;

    size_t numargs;

    for (numargs = 0; ; numargs++) {
        skipwhite(itr);

        if (args == NULL)
            curr = args = malloc(sizeof *args);

        if (curr == NULL) {
            if (args != NULL)
                Expression_delete(&args);

            return ERROR_FAILED_ALLOCATION;
        }

        // if (Expression_parse_variable)

        curr = curr->next;

        ++*itr;
    }

    // skipwhite(itr);

    // Expression *args = malloc(sizeof *args);

    // if (args == NULL)
    //     return ERROR_FAILED_ALLOCATION;

    // Expression *curr = args;

    // size_t numargs;
    // for (numargs = 0; curr = Expression_parse_variable(curr, itr) == ERROR_NONE; numargs++);

    // if (numargs == 0) {


    //     return ERROR_FAILED_PARSE;
    // }

    return ERROR_NONE;
}

/* static */ ErrorCode Expression_parse_application(Expression *self, const char **itr)
{
    assert(self != NULL);
    assert(itr != NULL && *itr != NULL);

    assert(0 && "TODO");

    return ERROR_FAILED_PARSE;
}

static ErrorCode Expression_parse_expression(Expression *self, const char **itr)
{
    assert(self != NULL);
    assert(itr != NULL && *itr != NULL);

    const char *start = *itr;

    {
        ErrorCode errorcode = Expression_parse_primary(self, itr);

        /* TODO: Should I clean up on failure? */
        if (errorcode == ERROR_FAILED_ALLOCATION)
            return ERROR_FAILED_ALLOCATION;

        if (errorcode == ERROR_NONE)
            return ERROR_NONE;
    }

    {
        ErrorCode errorcode = Expression_lambda_primary(self, itr);

        /* TODO: Should I clean up on failure? */
        if (errorcode == ERROR_FAILED_ALLOCATION)
            return ERROR_FAILED_ALLOCATION;

        if (errorcode == ERROR_NONE)
            return ERROR_NONE;
    }

    {
        ErrorCode errorcode = Expression_lambda_primary(self, itr);

        /* TODO: Should I clean up on failure? */
        if (errorcode == ERROR_FAILED_ALLOCATION)
            return ERROR_FAILED_ALLOCATION;

        if (errorcode == ERROR_NONE)
            return ERROR_NONE;
    }

    *itr = start;

    return ERROR_FAILED_PARSE;
}

int main(void)
{
    const char *input = "((((((v))))))";
    const char *itr = input;

    Expression *expr = malloc(sizeof *expr);

    if (!Expression_parse_expression(expr, &itr)) {
        free(expr);
        fputs("FATAL ERROR: Failed parse!", stderr);
        return 1;
    }

    Expression_println(expr);

    free(expr);
    expr = NULL;

    return 0;
}
