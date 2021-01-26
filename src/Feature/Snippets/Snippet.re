open Oni_Core;
open Utility;
open Snippet_internal;

[@deriving show]
type raw = list(Snippet_internal.t);

[@deriving show]
type t = list(Snippet_internal.t);

let parse: string => result(raw, string) =
  str => {
    let str = str ++ "\n";
    let parse = lexbuf =>
      switch (Snippet_parser.main(Snippet_lexer.token, lexbuf)) {
      | exception Snippet_lexer.Error =>
        Error("Error parsing binding: " ++ str)
      | exception Snippet_parser.Error => Error("Error parsing")
      | v => Ok(v)
      };

    str
    |> Lexing.from_string
    |> parse
    |> Result.map(List.map(Snippet_internal.normalize));
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
   });

let resolve =
    (
      ~prefix: string,
      ~postfix: string,
      ~indentationSettings: Oni_Core.IndentationSettings.t,
      snippet,
    ) => {
  ignore(indentationSettings);

  let lines = List.length(snippet);

  // Keep track of the leading whitespace, so we can add it on subsequent lines
  let leadingWhitespace = StringEx.leadingWhitespace(prefix);

  snippet
  |> List.mapi((idx, line) => {
       let isFirst = idx == 0;
       let isLast = idx == lines - 1;

       // Add prefix
       let line' =
         if (isFirst) {
           [Text(prefix), ...line];
         } else if (leadingWhitespace != "") {
           [Text(leadingWhitespace), ...line];
         } else {
           line;
         };

       // Add postfix
       let line'' =
         if (isLast) {
           let revLine' = line' |> List.rev;
           [Text(postfix), ...revLine'] |> List.rev;
         } else {
           line';
         };

       line'';
     });
};

let%test_module "resolve" =
  (module
   {
     let useTabs =
       IndentationSettings.(create(~mode=Tabs, ~size=4, ~tabSize=4, ()));
     let _useSpaces2 =
       IndentationSettings.(create(~mode=Spaces, ~size=2, ~tabSize=2, ()));
     let%test "adds prefix and postfix in single line" = {
       let raw = parse("abc") |> Result.get_ok;

       let resolved =
         resolve(
           ~prefix="PREFIX",
           ~postfix="POSTFIX",
           ~indentationSettings=useTabs,
           raw,
         );

       resolved == [[Text("PREFIX"), Text("abc"), Text("POSTFIX")]];
     };

     let%test "adds prefix and postfix when multi-line" = {
       let raw = parse("abc\ndef") |> Result.get_ok;

       let resolved =
         resolve(
           ~prefix="PREFIX",
           ~postfix="POSTFIX",
           ~indentationSettings=useTabs,
           raw,
         );

       resolved
       == [
            [Text("PREFIX"), Text("abc")],
            [Text("def"), Text("POSTFIX")],
          ];
     };
     let%test "persists whitespace across lines" = {
       let raw = parse("abc\ndef\nghi") |> Result.get_ok;

       let resolved =
         resolve(
           ~prefix="  PREFIX",
           ~postfix="POSTFIX",
           ~indentationSettings=useTabs,
           raw,
         );

       resolved
       == [
            [Text("  PREFIX"), Text("abc")],
            [Text("  "), Text("def")],
            [Text("  "), Text("ghi"), Text("POSTFIX")],
          ];
     };
   });

let toLines = (resolvedSnippet: t) => {
  let rec lineToString = (acc, line: list(Snippet_internal.segment)) =>
    switch (line) {
    | [] => acc
    | [hd, ...tail] =>
      let acc' = acc ++ segmentToString(hd);
      lineToString(acc', tail);
    }
  and segmentToString = segment =>
    switch (segment) {
    | Text(str) => str
    | Placeholder({contents, _}) => lineToString("", contents)
    | Choice({choices, _}) =>
      List.nth_opt(choices, 0) |> Option.value(~default="")
    | Variable({name, default}) => default |> Option.value(~default=name)
    };

  resolvedSnippet |> List.map(lineToString("")) |> Array.of_list;
};
