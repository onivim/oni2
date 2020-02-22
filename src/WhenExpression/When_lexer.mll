{
  open When_parser

  exception Error
}

let whitespace = [' ' '\t' '\n']
let ident = ['a'-'z' 'A'-'Z' '.']+
let literal = ['a'-'z' 'A'-'Z' '0'-'9']+

rule token = parse
| whitespace
    { token lexbuf }
| ident as s
    { IDENT s }
| literal as s
    { LITERAL s }
| "&&"
    { AND }
| "||"
    { OR }
| "=="
    { EQ }
| "!="
    { NEQ }
| '!'
    { NOT }
| '('
    { LPAREN }
| ')'
    { RPAREN }
| eof
    { EOF }
| _
    { raise Error }
