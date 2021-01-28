%token DOLLAR
%token EOF
%token LB
%token RB
%token COLON
%token PIPE
%token COMMA
%token <int> NUMBER
%token <string> TEXT
%token <string> VARIABLE
%token NEWLINE

%start <Snippet_internal.t list> main

%%

main:
| lines = list(line); EOF { lines }

line:
| phrase = list(expr); NEWLINE { phrase }

expr:
| e = expr_nested; { e }
| LB; { Text("{") } 
| RB; { Text("}") }

expr_nested:
| DOLLAR; num = NUMBER { Placeholder({index = num; contents = [] }) } 
| DOLLAR; LB; num = NUMBER; RB{ Placeholder({ index = num; contents = []}) }
| DOLLAR; LB; num = NUMBER; COLON; e = list(expr_nested); RB{ Placeholder({ index = num; contents = e}) }
| DOLLAR; LB; num = NUMBER; PIPE; firstChoice = string; additionalChoices = list(additional_choice); PIPE; RB { Snippet_internal.Choice({index = num; choices = [firstChoice] @
additionalChoices }) }
| DOLLAR; var = VARIABLE; { Variable({name = var; default = None }) }
| DOLLAR; LB; var = VARIABLE; COLON; default = string; RB { Variable({name = var; default = Some(default) }) }
| text = TEXT { Text(text) }
| numberAsText = NUMBER { Text(string_of_int(numberAsText)) }
| variableAsText = VARIABLE { Snippet_internal.Text(variableAsText) }


additional_choice:
| COMMA; choice = string; { choice }

string:
| str = list(character) { String.concat "" str }
| numberAsText = NUMBER { string_of_int(numberAsText) }
| variableAsText = VARIABLE { variableAsText }

character:
| char = TEXT { char }
