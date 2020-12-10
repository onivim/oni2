{
	open Snippet_parser

	exception Error
}
let alpha = ['a' - 'z' 'A' - 'Z' ' ' '\t']+

let number = ['0'-'9']+

rule token = parse
| '{' { LB }
| '}' { RB }
| '$' { DOLLAR }
| ':' { COLON }
| alpha as text
 { TEXT (text) }
| number as num
	{ NUMBER (int_of_string(num)) }
| eof { EOF }
| _ { raise Error }
