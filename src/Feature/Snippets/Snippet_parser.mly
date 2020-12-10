%token DOLLAR
%token EOF
%token LB
%token RB
%token COLON
%token PIPE
%token COMMA
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

| DOLLAR; LB; num = NUMBER; PIPE; firstChoice = string; additionalChoices = list(additional_choice); PIPE; RB { Choice({index = num; choices = [firstChoice] @
additionalChoices }) }

| text = TEXT { Text(text) }
| numberAsText = NUMBER { Text(string_of_int(numberAsText)) }

additional_choice:
| COMMA; choice = string; { choice }

string:
| str = list(character) { String.concat "" str }

character:
| char = TEXT { char }
