open When_lexer;

let parse = (str) => {
  let lexbuf = Lexing.from_string(str/*"abc && (def || !ghi)"*/);
  switch (When_parser.main(When_lexer.token, lexbuf)) {
  | exception When_lexer.Error => print_endline ("lexer error")
  | exception When_parser.Error => print_endline ("Error");
  | v => 
    print_endline("parsed ok: " ++ Expression.show(v)); 
  };
};
