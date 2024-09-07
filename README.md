# Authors
Cole Carley

Aiden Kirk

Nicholas Langley


# GRAMMAR

### Keywords

`let`: variable declaration

`print`: print to screen

### Statements

Stmt -> DeclStmt | ExprStmt;

DeclStmt -> "let" Expr ";"

ExprStmt -> Expr ";"

### Expressions

Expr -> Factor

Factor -> (Term ("+" | "/" ) Factor) | Term

Term -> (Unary ("*" | "/" ) Term) | Unary

Unary -> ("-" Unary) | Primary 

Primary -> \<int\> | \<identifier\>
