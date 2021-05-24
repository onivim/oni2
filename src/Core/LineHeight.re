[@deriving show]
type t =
  | Proportional(float)
  | Absolute(float)
  | Padding(int);

let absolute = size => Absolute(size);

let proportional = size => Proportional(size);

let padding = size => Padding(size);

let default = proportional(1.2);

let calculate = (~measuredFontHeight: float) =>
  fun
  | Proportional(size) => measuredFontHeight *. size
  | Absolute(size) => size
  | Padding(size) => float(size) +. measuredFontHeight;

module Decode = {
  open Json.Decode;

  let proportional =
    obj(({field, _}) => {
      Proportional(field.required("proportional", float))
    });

  let absolute =
    obj(({field, _}) => {Absolute(field.required("absolute", float))});

  let padded =
    obj(({field, _}) => {Padding(field.required("padding", int))});

  let justFloat =
    float |> map(size => size >= 5. ? Absolute(size) : Proportional(1.2));

  let decode =
    one_of([
      ("float", justFloat),
      ("proportional", proportional),
      ("absolute", absolute),
      ("padded", padded),
    ]);
};

let decode = Decode.decode;

module Encode = {
  open Json.Encode;

  let encode =
    fun
    | Proportional(size) => obj([("proportional", size |> float)])
    | Absolute(size) => obj([("absolute", size |> float)])
    | Padding(size) => obj([("padding", size |> int)]);
};

let encode = Encode.encode;
