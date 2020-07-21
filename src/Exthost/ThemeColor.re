open Oni_Core;

[@deriving show]
type t = {id: string};

let decode =
  Json.Decode.(obj(({field, _}) => {id: field.required("id", string)}));
