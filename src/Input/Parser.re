module Regexes = {
  let control = Str.regexp_case_fold(".*c-.*");
  let alt = Str.regexp_case_fold(".*a-.*");
  let command = Str.regexp_case_fold(".*d-.*");
  let shift = Str.regexp_case_fold(".*s-.*");
  let compound = Str.regexp_case_fold({|<\(.-\)+\(.*\)>|});
  let bracketized = Str.regexp_case_fold({|<\(.*\)>|});
};

let bracketize = key =>
  if (String.length(key) > 1) {
    "<" ++ key ++ ">";
  } else {
    key;
  };

/*
 [toFriendlyName(key)] takes a vim-style key description,
 like ["<C-v>"], and returns a friendly representation, like ["Ctrl+v"]
 */
let rec toFriendlyName =
  fun
  | " " => "Space"
  | "<BS>" => "Backspace"
  | "<ESC>" => "Escape"
  | "<CR>" => "Enter"
  | key when String.length(key) == 0 => ""
  | key when String.length(key) == 1 => key
  | key => {
      let isCompound = Str.string_match(Regexes.compound, key, 0);

      if (isCompound) {
        let buffer = Buffer.create(String.length(key));
        let significand =
          Str.matched_group(2, key) |> bracketize |> toFriendlyName;

        let hasControl = Str.string_match(Regexes.control, key, 0);
        let hasShift = Str.string_match(Regexes.shift, key, 0);
        let hasAlt = Str.string_match(Regexes.alt, key, 0);
        let hasCommand = Str.string_match(Regexes.command, key, 0);

        if (hasCommand) {
          switch (Revery.Environment.os) {
          | Windows => Buffer.add_string(buffer, "Win + ")
          | Mac => Buffer.add_string(buffer, "Cmd + ")
          | _ => Buffer.add_string(buffer, "Super + ")
          };
        };
        if (hasControl) {
          Buffer.add_string(buffer, "Ctrl + ");
        };
        if (hasAlt) {
          Buffer.add_string(buffer, "Alt + ");
        };
        if (hasShift) {
          Buffer.add_string(buffer, "Shift + ");
        };

        Buffer.add_string(buffer, significand);

        Buffer.contents(buffer);
      } else {
        let isBracketized = Str.string_match(Regexes.bracketized, key, 0);

        if (isBracketized) {
          Str.matched_group(1, key)
          |> String.lowercase_ascii
          |> String.capitalize_ascii;
        } else {
          key;
        };
      };
    };
