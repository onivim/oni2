type t = {
  scancode: int,
  keycode: int,
  modifiers: Modifiers.t,
};

let toString = (~meta="Meta", ~keyCodeToString, {keycode, modifiers, _}) => {
  let buffer = Buffer.create(16);
  let separator = " + ";

  let keyString = keyCodeToString(keycode);

  let keyString =
    modifiers.shift ? String.uppercase_ascii(keyString) : keyString;

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

  Buffer.add_string(buffer, keyString);

  Buffer.contents(buffer);
};
