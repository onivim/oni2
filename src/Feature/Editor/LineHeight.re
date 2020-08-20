open Oni_Core;
[@deriving show]
type t =
  | Proportional(float)
  | Absolute(float);

let absolute = size => Absolute(size);

let proportional = size => Proportional(size);

let default = proportional(1.2);

let calculate = (~measuredFontHeight: float) =>
  fun
  | Proportional(size) => measuredFontHeight *. size
  | Absolute(size) => size;

module Decode = {
  open Json.Decode;

  let proportional =
    obj(({field, _}) => {
      Proportional(field.required("proportional", float))
    });

  let absolute =
    obj(({field, _}) => {Absolute(field.required("absolute", float))});

  let justFloat =
    float |> map(size => size >= 5. ? Absolute(size) : Proportional(1.2));

  let decode =
    one_of([
      ("float", justFloat),
      ("proportional", proportional),
      ("absolute", absolute),
    ]);
};

let decode = Decode.decode;

module Encode = {
  open Json.Encode;

  let encode =
    fun
    | Proportional(size) => obj([("proportional", size |> float)])
    | Absolute(size) => obj([("absolute", size |> float)]);
};

let encode = Encode.encode;
