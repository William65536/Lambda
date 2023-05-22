# Lambda

## References

- [Efficient evaluation of lambda expressions](https://dl.acm.org/doi/pdf/10.1145/96709.96711)

## Errors



## Grammar

```
EOF = "\0"
IDENTIFIER = [A-Z][A-z0-9_]*
VARIABLE = [a-z][A-z0-9_]*

program = { statement }* EOF
statement = { assignment | expression }? "."
assignment = IDENTIFIER ":=" expression
expression = application-low
application-low = lambda { "@" application-low }?
lambda = { VARIABLE "->" }? application-high
application-high = { atom }+
atom = VARIABLE | IDENTIFIER | "(" expression ")"
```
