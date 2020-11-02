[@deriving show]
type t = {
  scancode: int,
  keycode: int,
  modifiers: Modifiers.t,
};

let parse = (~getKeycode, ~getScancode, str) => {
  let parse = lexbuf =>
    switch (Matcher_parser.keys(Matcher_lexer.token, lexbuf)) {
    | exception Matcher_lexer.Error => Error("Error parsing binding: " ++ str)
    | exception (Matcher_lexer.UnrecognizedModifier(m)) =>
      Error("Unrecognized modifier:" ++ m ++ " in: " ++ str)
    | exception Matcher_parser.Error =>
      Error("Error parsing binding: " ++ str)
    | v => Ok(v)
    };

  let flatMap = (f, r) => Result.bind(r, f);

  let finish = r => {
    let f = ((key, mods)) => {
      switch (getKeycode(key), getScancode(key)) {
      | (Some(keycode), Some(scancode)) =>
        Ok({
          modifiers: Matcher_internal.Helpers.internalModsToMods(mods),
          scancode,
          keycode,
        })
      | _ => Error("Unrecognized key: " ++ Key.toString(key))
      };
    };

    let bindings = r |> List.map(f);

    bindings |> Base.Result.all;
  };

  str
  |> String.lowercase_ascii
  |> Lexing.from_string
  |> parse
  |> flatMap(finish);
};

let toString = (~meta="Meta", ~keyCodeToString, {keycode, modifiers, _}) => {
  let buffer = Buffer.create(16);
  let separator = " + ";

  let keyString = keyCodeToString(keycode);

  let onlyShiftPressed =
    modifiers.shift && !modifiers.control && !modifiers.meta && !modifiers.alt;

  let keyString =
    String.length(keyString) == 1 && !onlyShiftPressed
      ? String.lowercase_ascii(keyString) : keyString;

  if (modifiers.meta) {
    Buffer.add_string(buffer, meta ++ separator);
  };

  if (modifiers.control) {
    Buffer.add_string(buffer, "Ctrl" ++ separator);
  };

  if (modifiers.altGr) {
    Buffer.add_string(buffer, "AltGr" ++ separator);
  } else if (modifiers.alt) {
    Buffer.add_string(buffer, "Alt" ++ separator);
  };

  if ((modifiers.meta || modifiers.control || modifiers.alt) && modifiers.shift) {
    Buffer.add_string(buffer, "Shift" ++ separator);
  };

  Buffer.add_string(buffer, keyString);

  Buffer.contents(buffer);
};
