type t =
  | Sequence(list(KeyPress.t))
  | AllKeysReleased;

type sequence = list(t);

let parse = (~explicitShiftKeyNeeded, str) => {
  let parse = lexbuf =>
    switch (Matcher_parser.main(Matcher_lexer.token, lexbuf)) {
    | exception Matcher_lexer.Error => Error("Error parsing binding: " ++ str)
    | exception (Matcher_lexer.UnrecognizedModifier(m)) =>
      Error("Unrecognized modifier:" ++ m ++ " in: " ++ str)
    | exception Matcher_parser.Error =>
      Error("Error parsing binding: " ++ str)
    | v => Ok(v)
    };

  prerr_endline("Matcher: " ++ str);
  let flatMap = (f, r) => Result.bind(r, f);

  let addShiftKeyToCapital = !explicitShiftKeyNeeded;

  let finish = r => {
    switch (r) {
    | Matcher_internal.AllKeysReleased => Ok(AllKeysReleased)
    | Matcher_internal.Sequence(keys) =>
      keys
      |> KeyPress.combineUnmatchedStrings
      |> List.iter(key =>
           prerr_endline("KEYS: " ++ Matcher_internal.show_keyMatcher(key))
         );
      keys
      |> KeyPress.combineUnmatchedStrings
      |> List.map(KeyPress.ofInternal(~addShiftKeyToCapital))
      |> List.flatten
      |> Base.Result.all
      |> Result.map(keys => Sequence(keys));
    };
  };

  str |> Lexing.from_string |> parse |> flatMap(finish);
};
