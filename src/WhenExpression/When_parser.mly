%token <string> VAR
%token AND OR
%token LPAREN RPAREN
%token NOT
%token EOF

%left OR
%left AND
%nonassoc NOT

%start <Expression.t> main

%%

main:
| phrase = expr EOF { phrase }

expr:
| s = VAR
    { Expression.Variable(s) }
| LPAREN e = expr RPAREN
    { e }
| e1 = expr AND e2 = expr
    { Expression.And(e1, e2) }
| e1 = expr OR e2 = expr
    { Expression.Or(e1, e2) }
| NOT e = expr %prec NOT
    { Expression.Not(e) }
