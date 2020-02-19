%token <string> VAR
%token AND OR
%token LPAREN RPAREN
%token NOT
%token EOF

%left OR
%left AND
%nonassoc NOT

%start <Types.expr> main

%{ open Types %}

%%

main:
| phrase = expr EOF { phrase }

expr:
| s = VAR
    { Variable(s) }
| LPAREN e = expr RPAREN
    { e }
| e1 = expr AND e2 = expr
    { And(e1, e2) }
| e1 = expr OR e2 = expr
    { Or(e1, e2) }
| NOT e = expr %prec NOT
    { Not(e) }
