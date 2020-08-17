%token <Matcher_internal.modifier> MODIFIER
%token <Key.t> BINDING
%token ALLKEYSRELEASED
%token LT GT
%token EXCLAMATION
%token EOF

%start <Matcher_internal.t> main

%%

main:
| ALLKEYSRELEASED { Matcher_internal.AllKeysReleased }
| phrase = list(expr) EOF { Matcher_internal.Sequence(phrase) }

expr:
| EXCLAMATION; LT e = keyup_binding GT { e }
| EXCLAMATION; s = keyup_binding { s }
| LT e = keydown_binding GT { e }
| s = keydown_binding { s }

keyup_binding:
| modifiers = list(MODIFIER); binding = BINDING { (Matcher_internal.Keyup, binding, modifiers) }

keydown_binding:
| modifiers = list(MODIFIER); binding = BINDING { (Matcher_internal.Keydown, binding, modifiers) }

