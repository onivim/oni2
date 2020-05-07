open Oni_Core;

type t = {id: string};

let decode =
  Json.Decode.(obj(({field, _}) => {id: field.required("id", string)}));
