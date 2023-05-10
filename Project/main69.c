#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include <setjmp.h>

enum Type {
    TYPE_LAMBDA = L'λ',
    TYPE_APPLICATION = L'α'
};

typedef struct Expression Expression;
struct Expression {
    int value;
    Expression *left, *right;
};

/* TODO: Ensure allocations and deallocations are proper - maybe track them */
/* TODO: Handle failed allocations */

void Expression_delete(Expression **self)
{
    assert(self != NULL);

    if (*self == NULL)
        return;

    switch ((*self)->value) {
        case TYPE_LAMBDA:
        case TYPE_APPLICATION:
            Expression_delete(&(*self)->left);
            Expression_delete(&(*self)->right);
            free(*self);
            break;
        default:
            if ('a' <= (*self)->value && (*self)->value <= 'z') {
                free(*self);
                return;
            }

            assert(0 && "Unreachable!");
    }
}

void skipwhite(const char **itr)
{
    assert(itr != NULL && *itr != NULL);

    for (; **itr == ' ' || **itr == '\t' || **itr == '\n' || **itr == '\r'; ++*itr);
}

bool Expression_parse_variable(Expression *self, const char **itr)
{
    assert(self != NULL);
    assert(itr != NULL && *itr != NULL);

    if (!('a' <= **itr && **itr <= 'z'))
        return false;

    self->value = **itr;
    self->left = self->right = NULL;

    ++*itr;

    return true;
}

bool Expression_parse_expression(Expression *self, const char **itr);

bool Expression_parse_group(Expression *self, const char **itr)
{
    assert(self != NULL);
    assert(itr != NULL && *itr != NULL);

    const char *start = *itr;

    if (**itr != '(')
        return false;

    ++*itr;

    skipwhite(itr);

    if (!Expression_parse_expression(self, itr)) {
        *itr = start;
        return false;
    }

    skipwhite(itr);

    if (**itr != ')') {
        *itr = start;
        Expression_delete(&self->left);
        Expression_delete(&self->right);
        return false;
    }

    return true;
}

bool Expression_parse_atom(Expression *self, const char **itr)
{
    assert(self != NULL);
    assert(itr != NULL && *itr != NULL);

    if (Expression_parse_variable(self, itr))
        return true;

    if (Expression_parse_group(self, itr))
        return true;

    return false;
}

/* TODO: Condense this function */
bool Expression_parse_application_high(Expression *self, const char **itr)
{
    assert(self != NULL);
    assert(itr != NULL && *itr != NULL);

    Expression templeft;

    if (!Expression_parse_atom(&templeft, itr))
        return false;

    skipwhite(itr);

    Expression tempright;

    if (!Expression_parse_atom(&tempright, itr)) {
        *self = templeft;
        return true;
    }
    
    self->left = malloc(sizeof *self->left); /* TODO: Ensure this allocation is successful */
    self->right = malloc(sizeof *self->right); /* TODO: Ensure this allocation is successful */

    *self->left = templeft;
    *self->right = tempright;

    self->value = TYPE_APPLICATION;

    while (true) {
        Expression tempright;

        skipwhite(itr);

        if (!Expression_parse_atom(&tempright, itr))
            break;

        Expression temp;
        temp.left = malloc(sizeof *temp.left); /* TODO: Ensure this allocation is successful */
        *temp.left = *self;
        temp.right = malloc(sizeof *temp.right); /* TODO: Ensure this allocation is successful */
        *temp.right = tempright;

        *self = temp;

        self->value = TYPE_APPLICATION;
    }

    return true;
}

bool Expression_parse_lambda(Expression *self, const char **itr)
{
    assert(self != NULL);
    assert(itr != NULL && *itr != NULL);

    const char *start = *itr;

    Expression tempvariable;

    if (!Expression_parse_variable(&tempvariable, itr)) {
        Expression tempexpression;

        if (Expression_parse_expression(&tempexpression, itr))
            return true;

        return false;
    }

    return true;
}

bool Expression_parse_expression(Expression *self, const char **itr)
{
    assert(self != NULL);
    assert(itr != NULL && *itr != NULL);

    if (Expression_parse_lambda(self, itr))
        return true;

    return false;
}

void Expression_print(Expression *self)
{
    assert(self != NULL);

    switch (self->value) {
        case TYPE_LAMBDA:
            Expression_print(self->left);
            fputs(" -> ", stdout);
            Expression_print(self->right);
            break;
        case TYPE_APPLICATION:
            putchar('(');
            Expression_print(self->left);
            putchar(' ');
            Expression_print(self->right);
            putchar(')');
            break;
        default:
            if ('a' <= self->value && self->value <= 'z') {
                putchar(self->value);
                break;
            }

            assert(0 && "Unreachable!");
    }
}

void Expression_println(Expression *self)
{
    Expression_print(self);
    puts("");
}

int main(void)
{
    const char *input = "a b c d (e f)", *itr = input;

    skipwhite(&itr);

    Expression *expr = malloc(sizeof *expr);

    if (!Expression_parse_expression(expr, &itr)) {
        fputs("FATAL ERROR: Failed parse!", stderr);
        return EXIT_FAILURE;
    }

    Expression_println(expr);

    Expression_delete(&expr);
    
    return 0;
}
