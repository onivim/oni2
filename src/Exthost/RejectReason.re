open Oni_Core;

type t = string;

let decode =
  Json.Decode.(obj(({field, _}) => field.required("rejectReason", string)));
