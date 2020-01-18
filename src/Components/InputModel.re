open Oni_Core;
open Utility;

let slice = (~start=0, ~stop=?, str) => {
  let length = String.length(str);
  let start = IntEx.clamp(~lo=0, ~hi=length, start);
  let stop =
    switch (stop) {
    | Some(index) => IntEx.clamp(~lo=0, ~hi=length, index)
    | None => length
    };

  String.sub(str, start, stop - start);
};

let removeBefore = (~count=1, index, text) => (
  slice(text, ~stop=index - count) ++ slice(text, ~start=index),
  max(0, index - count),
);

let removeAfter = (~count=1, index, text) => (
  slice(text, ~stop=index) ++ slice(text, ~start=index + count),
  index,
);

let add = (~at as index, insert, text) => (
  slice(text, ~stop=index) ++ insert ++ slice(text, ~start=index),
  index + String.length(insert),
);

let handleInput = (~text, ~cursorPosition) =>
  fun
  | "<LEFT>" => (text, max(0, cursorPosition - 1))
  | "<RIGHT>" => (text, min(String.length(text), cursorPosition + 1))
  | "<BS>" => removeBefore(cursorPosition, text)
  | "<DEL>" => removeAfter(cursorPosition, text)
  | "<HOME>" => (text, 0)
  | "<END>" => (text, String.length(text))
  | key when String.length(key) == 1 => add(~at=cursorPosition, key, text)
  | _ => (text, cursorPosition);
