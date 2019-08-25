let cMinus = Str.regexp_case_fold(".*c-.*");
let aMinus = Str.regexp_case_fold(".*a-.*");
let dMinus = Str.regexp_case_fold(".*d-.*");
let sMinus = Str.regexp_case_fold(".*s-.*");

/*
 [toFriendlyName(key)] takes a vim-style key description,
 like ["<C-v>"], and returns a friendly representation, like ["Ctrl+v"]
 */
let toFriendlyName = (v: string) => {
  let len = String.length(v);

  switch (v) {
  | "<ESC>" => Some("Escape")
  | "<TAB>" => Some("Tab")
  | "<C-TAB>" => Some("Control + Tab")
  | "<CR>" => Some("Enter")
  | _ =>
    if (len == 0) {
      None;
    } else if (len == 1) {
      Some(v);
    } else {
      let firstCharacter = v.[0];
      let lastCharacter = v.[len - 1];

      if (firstCharacter == '<' && lastCharacter == '>') {
        let hasControl = Str.string_match(cMinus, v, 0);
        let hasShift = Str.string_match(sMinus, v, 0);
        let hasAlt = Str.string_match(aMinus, v, 0);
        let hasCommand = Str.string_match(dMinus, v, 0);

        let character = v.[len - 2];

        let ret = hasCommand ? "Command + " : "";
        let ret = ret ++ (hasControl ? "Control + " : "");
        let ret = ret ++ (hasAlt ? "Alt + " : "");

        let character =
          !hasShift ? Char.lowercase_ascii(character) : character;

        Some(ret ++ String.make(1, character));
      } else {
        None;
      };
    }
  };
};
