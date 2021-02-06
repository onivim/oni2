[@deriving show({with_path: false})]
type t =
  | Default
  | None
  | Antialiased
  | SubpixelAntialiased;

module Decode = {
  open Json.Decode;
  let decode =
    string
    |> map(
         fun
         | "none" => None
         | "antialiased" => Antialiased
         | "subpixel-antialiased" => SubpixelAntialiased
         | _ => Default,
       );
};

module Encode = {
  open Json.Encode;

  let encode: encoder(t) =
    fun
    | None => string("none")
    | Antialiased => string("antialiased")
    | SubpixelAntialiased => string("subpixel-antialiased")
    | Default => string("default");
};

let decode = Decode.decode;

let encode = Encode.encode;
