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
          arguments: field.withDefault("arguments", [`Null], list(value)),
        }
      )
    );
  };
};

module Encode = {
  open Json.Encode;
  let encode = cmd =>
    obj([
      ("id", cmd.id |> nullable(string)),
      ("title", cmd.label |> nullable(Label.encode)),
      ("arguments", cmd.arguments |> list(value)),
    ]);
};

let decode = Decode.decode;
let encode = Encode.encode;
