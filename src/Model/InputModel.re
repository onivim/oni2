let splitAt = (index, str) => {
  switch (index) {
  | 0 => ("", str)
  | _ =>
    let before =
      try(Str.string_before(str, index)) {
      | _ => str
      };
    let after =
      try(Str.string_after(str, index)) {
      | _ => ""
      };
    (before, after);
  };
};

let getSafeStringBounds = (str, index, delta) => {
  let newIndex = index + delta;
  let currentLength = String.length(str);
  newIndex > currentLength ? currentLength : newIndex < 0 ? 0 : newIndex;
};

let removeBefore = (~count=1, index, text) => {
  let (before, after) = splitAt(index, text);
  let nextPosition = getSafeStringBounds(before, index, - count);
  let newText = Str.string_before(before, nextPosition) ++ after;
  (newText, nextPosition);
};

let removeAfter = (~count=1, index, text) => {
  let (before, after) = splitAt(index, text);
  let newText =
    before ++ Str.last_chars(after, max(0, String.length(after) - count));
  (newText, index);
};

let add = (~at as index, insert, text) => {
  let (before, after) = splitAt(index, text);
  (before ++ insert ++ after, index + String.length(insert));
};

let handleInput = (~text, ~cursorPosition) =>
  fun
  | "<LEFT>" => (text, max(0, cursorPosition - 1))

  | "<RIGHT>" => (text, min(String.length(text), cursorPosition + 1))

  | "<BS>" => removeBefore(cursorPosition, text)

  | "<DEL>" => removeAfter(cursorPosition, text)

  | key when String.length(key) == 1 => add(~at=cursorPosition, key, text)

  | _ => (text, cursorPosition);
