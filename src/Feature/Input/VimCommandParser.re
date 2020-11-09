open Oni_Core;
open Utility;

type t =
  | KeySequence(string)
  | ExCommand(string);

let parse = str => {
  let trimmed = str |> String.trim |> String.lowercase_ascii;

  if (StringEx.startsWith(~prefix=":<c-u>", trimmed)
      && StringEx.endsWith(~postfix="<cr>", trimmed)) {
    let command = String.sub(str, 6, String.length(str) - 10);
    ExCommand(command);
  } else {
    KeySequence(str);
  };
};
