# Lambda

## Grammar

```
expression = lambda | application | primary
lambda = "^" { VARIABLE }* "." expression
application = primary { expression }*
primary = VARIABLE | "(" expression ")"

```