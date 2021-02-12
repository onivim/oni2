open Oni_Core;

[@deriving show]
type t = {
  id: option(string),
  label: option(Label.t),
};

module Decode = {
  let decode = {
    Json.Decode.(
      obj(({field, _}) =>
        {
          id: field.optional("id", string),
          label: field.optional("title", Label.decode),
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
    ]);
};

let decode = Decode.decode;
let encode = Encode.encode;
