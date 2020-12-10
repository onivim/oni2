{
	open Snippet_parser

	exception Error
}

let number = ['0'-'9']+

rule token = parse
| '{' { LB }
| '}' { RB }
| '$' { DOLLAR }
| ':' { COLON }
| '|' { PIPE }
| number as num
	{ NUMBER (int_of_string(num)) }
| _ as char
 { TEXT (String.make 1 char) }
| eof { EOF }
| _ { raise Error }
