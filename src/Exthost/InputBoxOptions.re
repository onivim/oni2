open Oni_Core;

[@deriving show]
type t = {
  ignoreFocusOut: bool,
  password: bool,
  placeHolder: option(string),
  prompt: option(string),
  value: option(string),
  // TODO
  // valueSelection: (int, int),
};

let decode =
  Json.Decode.(
    obj(({field, _}) =>
      {
        ignoreFocusOut: field.withDefault("ignoreFocusOut", false, bool),
        password: field.withDefault("password", false, bool),
        placeHolder: field.optional("placeHolder", string),
        prompt: field.optional("prompt", string),
        value: field.optional("value", string),
      }
    )
  );
