open Oni_Core;

[@deriving show]
type t = {
  range: OneBasedRange.t,
  text: string,
};

let decode =
  Json.Decode.(
    obj(({field, _}) =>
      {
        range: field.required("range", OneBasedRange.decode),
        text: field.required("text", string),
      }
    )
  );
