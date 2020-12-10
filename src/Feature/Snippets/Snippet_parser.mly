%token DOLLAR
%token EOF
%token LB
%token RB
%token COLON
%token PIPE
%token <int> NUMBER
%token <string> TEXT

%start <Snippet_internal.t> main

%%

main:
| phrase = list(expr) EOF { phrase }

expr:
| DOLLAR; num = NUMBER { Placeholder({index = num; contents = [] }) }
| DOLLAR; LB; num = NUMBER; RB{ Placeholder({ index = num; contents = []}) }
| DOLLAR; LB; num = NUMBER; COLON; e = list(expr); RB{ Placeholder({ index = num; contents = e}) }
| text = TEXT { Text(text) }
