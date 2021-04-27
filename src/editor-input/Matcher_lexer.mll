{
	open Matcher_parser
	open Matcher_internal
	open Key
	
	exception Error

	exception UnrecognizedModifier of string
	exception UnrecognizedSpecialKey of string

	let specialKeyTable = Hashtbl.create 64
	let modifierTable = Hashtbl.create 64

	let _ = List.iter (fun (name, keyword) ->
	Hashtbl.add modifierTable name keyword) [
	"c-", MODIFIER (Control);
	"s-", MODIFIER (Shift);
	"a-", MODIFIER (Alt);
	(* For Vim-style keybindings, a- and m- are equivalent modifiers *)
	"m-", MODIFIER (Alt);
	"d-", MODIFIER (Super);
	"ctrl+", MODIFIER (Control);
	"shift+", MODIFIER (Shift);
	"alt+", MODIFIER (Alt);
	(* For Code-style bindings, meta+ refers to the 'super' key on Linux *)
	"meta+", MODIFIER (Super);
	"win+", MODIFIER (Super);
	"cmd+", MODIFIER (Super);
	]

	let _ = List.iter(fun (name, keyword) ->
	Hashtbl.add specialKeyTable name keyword) [
	"release", ALLKEYSRELEASED;
	"esc", BINDING (Physical(Escape));
	"escape", BINDING (Physical(Escape));
	"up", BINDING (Physical(Up));
	"down", BINDING (Physical(Down));
	"left", BINDING (Physical(Left));
	"right", BINDING (Physical(Right));
	"tab", BINDING (Physical(Tab));
	"pageup", BINDING (Physical(PageUp));
	"pagedown", BINDING (Physical(PageDown));
	"cr", BINDING (Physical(Return));
	"enter", BINDING (Physical(Return));
	"space", BINDING (Physical(Space));
	"del", BINDING (Physical(Delete));
	"delete", BINDING (Physical(Delete));
	"pause", BINDING (Physical(Pause));
	"pausebreak", BINDING (Physical(Pause));
	"home", BINDING (Physical(Home));
	"end", BINDING (Physical(End));
	"delete", BINDING (Physical(Delete));
	"bs", BINDING (Physical(Backspace));
	"backspace", BINDING (Physical(Backspace));
	"capslock", BINDING (Physical(CapsLock));
	"insert", BINDING(Physical(Insert));
	"numpad_multiply", BINDING(Physical(NumpadMultiply));
	"numpad_add", BINDING(Physical(NumpadAdd));
	"numpad_separator", BINDING(Physical(NumpadSeparator));
	"numpad_subtract", BINDING(Physical(NumpadSubtract));
	"numpad_decimal", BINDING(Physical(NumpadDecimal));
	"numpad_divide", BINDING(Physical(NumpadDivide));
	"numpad_multiply", BINDING(Physical(NumpadMultiply));
	"leader", BINDING(Special(Leader));
	"plug", BINDING(Special(Plug));
	"plus", BINDING(Physical(Character(Uchar.of_char '+')));
	]
}

let white = [' ' '\t']+

let alpha = ['a' - 'z' 'A' - 'Z']
let alphaWithUnderscore = ['a' - 'z' 'A' - 'Z' '_']
let modifier = alpha+ ['-' '+']

let binding = ['a'-'z' 'A'-'Z' '0'-'9' '`' '-' '=' '[' ']' '\\' ';' '\'' ',' '.' '/' '+' ]

let numpadDigit = ['n'] ['u'] ['m'] ['p'] ['a'] ['d'] ['0' '9']

rule token = parse
| modifier as m
 { 
 	let m = String.lowercase_ascii m in
	try Hashtbl.find modifierTable m
	with Not_found ->
		raise (UnrecognizedModifier m)
 }
| numpadDigit as numpadDigit { 
	BINDING (Physical (NumpadDigit(int_of_string(String.make 1 (String.get numpadDigit 6)))))
}
| alphaWithUnderscore+ as sk
{
	let candidate = String.lowercase_ascii sk in
	try Hashtbl.find specialKeyTable candidate
	with Not_found -> 
		BINDING(UnmatchedString(sk))
		
}
| ['f' 'F'] (['0'-'9'] as m) { BINDING ( Physical(Function(int_of_string (String.make 1 m)) ) ) }
| ['f' 'F'] '1' (['0'-'9'] as m) { BINDING ( Physical(Function(int_of_string ("1" ^ (String.make 1 m))) ) ) }
| ['f' 'F'] '1' (['0'-'9'] as m) { BINDING ( Physical (Function(int_of_string ("1" ^ (String.make 1 m))) ) ) }
| white { token lexbuf }
| '<' ['l' 'L'] ['t' 'T'] '>' { BINDING (Physical(Character(Uchar.of_char '<'))) }
| '<' ['g' 'G'] ['t' 'T'] '>' { BINDING (Physical(Character(Uchar.of_char '>'))) }
| binding as i
 { BINDING (Physical(Character (Uchar.of_char i))) }
| '<' { LT }
| '>' { GT }
| eof { EOF }
| _ { BINDING (UnmatchedString(Lexing.lexeme lexbuf)) }
