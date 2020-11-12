%token <Matcher_internal.modifier> MODIFIER
%token <Matcher_internal.keyPress> BINDING
%token ALLKEYSRELEASED
%token LT GT
%token EOF

%start <Matcher_internal.t> main

%start <Matcher_internal.keyList> keys

%%

main:
| ALLKEYSRELEASED { Matcher_internal.AllKeysReleased }
| phrase = list(expr) EOF { Matcher_internal.Sequence(phrase) }

keys:
| keys = list(expr) EOF  { keys }

expr:
| LT e = keydown_binding GT { e }
| s = keydown_binding { s }

keydown_binding:
| modifiers = list(MODIFIER); binding = BINDING { (binding, modifiers) }

