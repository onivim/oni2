open Oni_Core;

[@deriving show]
type t = {
  id: option(string),
  label: option(Label.t),
  arguments: list(Yojson.Safe.t),
};

module Decode = {
  let decode = {
    Json.Decode.(
      obj(({field, _}) =>
        {
          id: field.optional("id", string),
          label: field.optional("title", Label.decode),
          arguments: field.withDefault("arguments", [], list(value)),
        }
      )
    );
  };
};

module Encode = {
  open Json.Encode;
  let encode = lens =>
    obj([
      ("id", lens.id |> nullable(string)),
      ("title", lens.label |> nullable(Label.encode)),
      ("arguments", lens.arguments |> list(value)),
    ]);
};

let decode = Decode.decode;
let encode = Encode.encode;
