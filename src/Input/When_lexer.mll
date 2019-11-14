{
  open When_parser

  exception Error
}

rule token = parse
| [' ' '\t' '\n']
    { token lexbuf }
| ['a' - 'z' 'A' - 'Z']+ as i
    { VAR (i) }
| '&' '&'
    { AND }
| '|' '|'
    { OR }
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
