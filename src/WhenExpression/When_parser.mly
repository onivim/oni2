%token <string> IDENT
%token <string> LITERAL
%token AND OR
%token LPAREN RPAREN
%token NOT
%token EOF
%token EQ
%token NEQ

%left OR
%left AND
%nonassoc NOT

%start <Types.expr> main

%{ open Types %}

%%

main:
| phrase = expr EOF { phrase }

expr:
| s = IDENT
    { Variable s }
| LPAREN e = expr RPAREN
    { e }
| e1 = expr AND e2 = expr
    { And (e1, e2) }
| e1 = expr OR e2 = expr
    { Or (e1, e2) }
| NOT e = expr %prec NOT
    { Not e }
| var = IDENT EQ value = LITERAL
    { Eq (var, value) }
| var = IDENT NEQ value = LITERAL
    { Neq (var, value) }
