
let parse = (str) => {
  let parse = lexbuf =>
    switch (Snippet_parser.main(Snippet_lexer.token, lexbuf)) {
    | exception Snippet_lexer.Error =>
      Error("Error parsing binding: " ++ str)
    | v => Ok(v)
    };
    // TODO: Combine text
  str |> Lexing.from_string |> parse
  |> Result.map(Snippet_internal.normalize)
}

open Snippet_internal;

  let%test "simple text" = {
    parse("abc") == Ok([Text("abc")]);
  };

  let%test "simple tabstop" = {
    parse("$0") == Ok([Placeholder({index: 0, contents: []})]);
  };

  let%test "bracket tabstop" = {
    parse("${1}") == Ok([Placeholder({index: 1, contents: []})]);
  };

  let%test "multiple tabstops" = {
    parse("$0 ${1}") == Ok([
      Placeholder({index: 0, contents: []}),
      Text(" "),
      Placeholder({index: 1, contents: []})
    ]);
  };

  let%test "placeholder with text" = {
    parse("${1:abc}") == Ok([Placeholder({index: 1, contents: [Text("abc")]})]);
  };

  let%test "placeholder with nested placeholder" = {
    parse("${1:abc $2}") == Ok([Placeholder({index: 1, contents: [Text("abc "),
    Placeholder({index: 2, contents: []})]})]);
  };

  let%test "placeholder with nested placeholder with text" = {
    parse("${1:abc ${2:placeholder}}") == Ok([Placeholder({index: 1, contents: [Text("abc "),
    Placeholder({index: 2, contents: [Text("placeholder")]})]})]);
  };


  // let%test "escaped placeholder" = {
  //   parse("\\$0") == Ok([Text("$0")])
  // };
  let%test "text around placeholder" = {
    parse("a $0 b") == Ok([Text("a "), Placeholder({index: 0, contents: []}), Text(" b")]);
  };
