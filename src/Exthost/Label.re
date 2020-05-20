open Oni_Core;

type segment =
| Text(string)
| Icon(string);

type t = list(segment);
  
module Parse = {
    let parse = str => [Text(str)];
}

module Decode = {
   open Json.Decode;
   let decode: Json.decoder(t) = string |> map(Parse.parse);
};

let decode = Decode.decode;
let of_string = Parse.parse;
