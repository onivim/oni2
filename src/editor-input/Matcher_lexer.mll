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
	"d-", MODIFIER (Meta);
	"ctrl+", MODIFIER (Control);
	"shift+", MODIFIER (Shift);
	"alt+", MODIFIER (Alt);
	"meta+", MODIFIER (Meta);
	"win+", MODIFIER (Meta);
	"cmd+", MODIFIER (Meta);
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
	]
}

let white = [' ' '\t']+

let alpha = ['a' - 'z' 'A' - 'Z']
let alphaWithUnderscore = ['a' - 'z' 'A' - 'Z' '_']
let modifier = alpha+ ['-' '+']

let binding = ['a'-'z' 'A'-'Z' '0'-'9' '`' '-' '=' '[' ']' '\\' ';' '\'' ',' '.' '/']

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
| binding as i
 { BINDING (Physical(Character (i))) }
| '<' { LT }
| '>' { GT }
| eof { EOF }
| _ { raise Error }
