[@deriving show]
type t = {
  cacheId: option(list(int)),
  range: OneBasedRange.t,
  command: option(Command.t),
};

module Decode = {
  open Oni_Core.Json.Decode;

  let decode =
    obj(({field, _}) =>
      {
        cacheId: field.optional("cacheId", list(int)),
        range: field.required("range", OneBasedRange.decode),
        command: field.optional("command", Command.decode),
      }
    );
};

module Encode = {
  open Oni_Core.Json.Encode;

  let encode = lens =>
    obj([
      ("cacheId", lens.cacheId |> nullable(list(int))),
      ("range", lens.range |> OneBasedRange.encode),
      ("command", lens.command |> nullable(Command.encode)),
    ]);
};

let decode = Decode.decode;
let encode = Encode.encode;

module List = {
  module Decode = {
    open Oni_Core.Json.Decode;

    let simple = list(decode);

    let nested = field("lenses", simple);

    let decode = one_of([("nested", nested), ("simple", simple)]);
  };

  let decode = Decode.decode;
};
