open Oni_Core;
open Utility;

type t =
  | KeySequence(string)
  | ExCommand(string);

module Internal = {
  // Replace the <SID> key with <SNR>123_
  let replaceSID = {
    let sidRegEx = Str.regexp("<SID>");
    (~scriptId: int, str) => {
      Str.global_replace(
        sidRegEx,
        "<SNR>" ++ string_of_int(scriptId) ++ "_",
        str,
      );
    };
  };

  let%test "replaceSID should replace all instances of <SID>" = {
    "<SID><SID>" |> replaceSID(~scriptId=123) == "<SNR>123_<SNR>123_";
  };
};

let parse = (~scriptId: Vim.Mapping.ScriptId.t, str) => {
  let trimmed = str |> String.trim |> String.lowercase_ascii;

  let scriptIdInt = Vim.Mapping.ScriptId.toInt(scriptId);
  if (StringEx.startsWith(~prefix=":<c-u>", trimmed)
      && StringEx.endsWith(~postfix="<cr>", trimmed)) {
    let command =
      String.sub(str, 6, String.length(str) - 10)
      |> Internal.replaceSID(~scriptId=scriptIdInt);
    ExCommand(command);
  } else {
    KeySequence(str);
  };
};
