{
	open Snippet_parser

	exception Error
}

let number = ['0'-'9']+

let var = ['_' 'a'-'z' 'A'-'Z'] ['_' 'a'-'z' 'A'-'Z' '0'-'9']*

rule token = parse
| '\r' '\n' { NEWLINE }
| '\n' { NEWLINE }
| '\\' '{' { TEXT("{") }
| '\\' '}' { TEXT("}") }
| '\\' '$' { TEXT("$") }
| '\\' ':' { TEXT(":") }
| '\\' '|' { TEXT("|") }
| '\\' ',' { TEXT(",") }
| '\\' { TEXT("\\") }
| '{' { LB }
| '}' { RB }
| '$' { DOLLAR }
| ':' { COLON }
| '|' { PIPE }
| ',' { COMMA }
| var as variable
	{ VARIABLE (variable) }
| number as num
	{ NUMBER (int_of_string(num)) }
| _ as char
 { TEXT (String.make 1 char) }
| eof { EOF }
| _ { raise Error }
