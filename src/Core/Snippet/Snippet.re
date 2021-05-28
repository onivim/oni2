include Snippet_internal;

let parse: string => result(t, string) =
  str => {
    let str = str ++ "\n";
    let parse = lexbuf =>
      switch (Snippet_parser.main(Snippet_lexer.token, lexbuf)) {
      | exception Snippet_lexer.Error =>
        Error("Error parsing binding: " ++ str)
      | exception Snippet_parser.Error =>
        Error("Error parsing snippet: " ++ str)
      | v => Ok(v)
      };

    str |> Lexing.from_string |> parse |> Result.map(List.map(normalize));
  };

let%test_module "parse" =
  (module
   {
     let%test "simple text" = {
       parse("abc") == Ok([[Text("abc")]]);
     };

     let%test "simple text, multiple lines" = {
       parse("abc\ndef") == Ok([[Text("abc")], [Text("def")]]);
     };
     let%test "simple text, extra line" = {
       parse("abc\n") == Ok([[Text("abc")], []]);
     };

     let%test "simple text, multiple lines, CRLF" = {
       parse("abc\r\ndef") == Ok([[Text("abc")], [Text("def")]]);
     };

     let%test "simple tabstop" = {
       parse("$0") == Ok([[Placeholder({index: 0, contents: []})]]);
     };

     let%test "bracket tabstop" = {
       parse("${1}") == Ok([[Placeholder({index: 1, contents: []})]]);
     };

     let%test "multiple tabstops" = {
       parse("$0 ${1}")
       == Ok([
            [
              Placeholder({index: 0, contents: []}),
              Text(" "),
              Placeholder({index: 1, contents: []}),
            ],
          ]);
     };

     let%test "single-item choice" = {
       parse("${1|a|}") == Ok([[Choice({index: 1, choices: ["a"]})]]);
     };

     let%test "multiple choices" = {
       parse("${1|a,b|}")
       == Ok([[Choice({index: 1, choices: ["a", "b"]})]]);
     };

     let%test "placeholder with text" = {
       parse("${1:abc}")
       == Ok([[Placeholder({index: 1, contents: [Text("abc")]})]]);
     };

     let%test "placeholder with nested placeholder" = {
       parse("${1:abc $2}")
       == Ok([
            [
              Placeholder({
                index: 1,
                contents: [
                  Text("abc "),
                  Placeholder({index: 2, contents: []}),
                ],
              }),
            ],
          ]);
     };

     let%test "placeholder with nested placeholder with text" = {
       parse("${1:abc ${2:placeholder}}")
       == Ok([
            [
              Placeholder({
                index: 1,
                contents: [
                  Text("abc "),
                  Placeholder({index: 2, contents: [Text("placeholder")]}),
                ],
              }),
            ],
          ]);
     };

     let%test "escaped placeholder" = {
       parse("\\$0") == Ok([[Text("$0")]]);
     };

     let%test "text around placeholder" = {
       parse("a $0 b")
       == Ok([
            [
              Text("a "),
              Placeholder({index: 0, contents: []}),
              Text(" b"),
            ],
          ]);
     };

     let%test "simple variable" = {
       parse("$TM_FILENAME")
       == Ok([[Variable({name: "TM_FILENAME", default: None})]]);
     };

     let%test "variable with default" = {
       parse("${TM_FILENAME:test}")
       == Ok([[Variable({name: "TM_FILENAME", default: Some("test")})]]);
     };

     let%test "curly braces" = {
       parse("{ $0 }")
       == Ok([
            [
              Text("{ "),
              Placeholder({index: 0, contents: []}),
              Text(" }"),
            ],
          ]);
     };

     let%test "placeholder with spaces" = {
       parse("${1:int a}")
       == Ok([[Placeholder({index: 1, contents: [Text("int a")]})]]);
     };

     let%test "comma in snippet" = {
       parse("a, b") == Ok([[Text("a, b")]]);
     };

     let%test "pipe in snippet" = {
       parse("a|b") == Ok([[Text("a|b")]]);
     };

     let%test "colon in snippet" = {
       parse("a:b") == Ok([[Text("a:b")]]);
     };

     // Test case to exercise a failure to parse snippets like:
     // https://github.com/Tom-xacademy/xa-js-snippets/blob/39bc330b9167635d44b0573e06cc1e10ccf8e891/snippets/snippets.json#L104
     let%test "non-placeholder special case" = {
       parse("${ data }") == Ok([[Text("${ data }")]]);
     };

     let%test "sass snippet bug" = {
       parse(
         "@media ${1:screen} ${2:and} ( ${3|max-width: ,min-width: ,height: |} )",
       )
       |> Result.is_ok;
     };

     let%test "sprintf parse bug (comma in placeholder)" = {
       parse("sprintf(${1:char *,})") |> Result.is_ok;
     };
     let%test "colon parse bug" = {
       parse("print(${1:items:Any..})") |> Result.is_ok;
     };
   });
