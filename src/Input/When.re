open When_lexer;

let parse = (str) => {
  let lexbuf = Lexing.from_string(str/*"abc && (def || !ghi)"*/);
  switch (When_parser.main(When_lexer.token, lexbuf)) {
  | exception When_lexer.Error => Error("Error parsing when binding: " ++ str);
  | exception When_parser.Error => Error("Error parsing when binding: " ++ str);
  | v => Ok(v)
  };
};
