module Regexes = {
  let control = Str.regexp_case_fold(".*c-.*");
  let alt = Str.regexp_case_fold(".*a-.*");
  let command = Str.regexp_case_fold(".*d-.*");
  let shift = Str.regexp_case_fold(".*s-.*");
  let compound = Str.regexp_case_fold({|<\(.-\)+\(.*\)>|});
};

/*
 [toFriendlyName(key)] takes a vim-style key description,
 like ["<C-v>"], and returns a friendly representation, like ["Ctrl+v"]
 */
let toFriendlyName =
  fun
  | " " => Some("Space")
  | "<BS>" => Some("Backspace")
  | "<ESC>" => Some("Escape")
  | "<TAB>" => Some("Tab")
  | "<CR>" => Some("Enter")
  | key when String.length(key) == 0 => None
  | key when String.length(key) == 1 => Some(key)
  | key => {
      let isCompound = Str.string_match(Regexes.compound, key, 0);

      if (isCompound) {
        let buffer = Buffer.create(String.length(key));
        let significand =
          Str.matched_group(2, key)
          |> String.lowercase_ascii
          |> String.capitalize_ascii;

        let hasControl = Str.string_match(Regexes.control, key, 0);
        let hasShift = Str.string_match(Regexes.shift, key, 0);
        let hasAlt = Str.string_match(Regexes.alt, key, 0);
        let hasCommand = Str.string_match(Regexes.command, key, 0);

        if (hasCommand) {
          Buffer.add_string(buffer, "Command + ");
        };
        if (hasControl) {
          Buffer.add_string(buffer, "Control + ");
        };
        if (hasAlt) {
          Buffer.add_string(buffer, "Alt + ");
        };
        if (hasShift) {
          Buffer.add_string(buffer, "Shift + ");
        };

        Buffer.add_string(buffer, significand);

        Some(Buffer.contents(buffer));
      } else {
        None;
      };
    };
