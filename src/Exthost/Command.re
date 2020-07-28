open Oni_Core;

[@deriving show]
type t = {
  id: string,
  label: option(Label.t),
};

module Decode = {
  let decode = {
    Json.Decode.(
      obj(({field, _}) =>
        {
          id: field.required("id", string),
          label: field.optional("title", Label.decode),
        }
      )
    );
  };
};

let decode = Decode.decode;
