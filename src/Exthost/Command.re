open Oni_Core;

[@deriving show]
type t = {
  id: string,
  title: option(string),
};

module Decode = {
  let decode = {
    Json.Decode.(
      obj(({field, _}) =>
        {
          id: field.required("id", string),
          title: field.optional("title", string),
        }
      )
    );
  };
};

let decode = Decode.decode;
