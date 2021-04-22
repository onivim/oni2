open Oni_Core;

module Location = {
  [@deriving show]
  type t =
    | Explorer
    | SCM
    | Extensions
    | Window
    | Notification
    | Dialog
    | Other(string);

  let ofInt =
    fun
    | 1 => Some(Explorer)
    | 3 => Some(SCM)
    | 5 => Some(Extensions)
    | 10 => Some(Window)
    | 15 => Some(Notification)
    | 20 => Some(Dialog)
    | _ => None;

  module Decode = {
    open Json.Decode;
    let int =
      int
      |> map(ofInt)
      |> and_then(
           fun
           | None => fail("Unrecognized location")
           | Some(location) => succeed(location),
         );

    let string = string |> map(str => Other(str));
  };

  let decode =
    Json.Decode.one_of([("int", Decode.int), ("string", Decode.string)]);
};

module Options = {
  [@deriving show]
  type t = {
    location: Location.t,
    title: option(string),
    source: option(string),
    total: option(int),
    cancellable: bool,
    buttons: list(string),
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          location: field.required("location", Location.decode),
          title: field.optional("title", string),
          source: field.optional("source", string),
          total: field.optional("total", int),
          cancellable: field.withDefault("cancellable", false, bool),
          buttons: field.withDefault("buttons", [], list(string)),
        }
      )
    );
};

module Step = {
  [@deriving show]
  type t = {
    message: option(string),
    increment: option(float),
    total: option(float),
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          message: field.optional("title", string),
          increment: field.optional("increment", float),
          total: field.optional("total", float),
        }
      )
    );

  let%test_module "Step" =
    (module
     {
       let%test "decode: integer increment / total" = {
         let actual =
           {|
  {"title": "test", "increment": 1, "total": 2}
|}
           |> Yojson.Safe.from_string
           |> Json.Decode.decode_value(decode);
         actual
         == Ok({
              message: Some("test"),
              increment: Some(1.0),
              total: Some(2.0),
            });
       };

       let%test "decode: float increment / total" = {
         let actual =
           {|
  {"title": "test", "increment": 1.0, "total": 2.0}
|}
           |> Yojson.Safe.from_string
           |> Json.Decode.decode_value(decode);
         actual
         == Ok({
              message: Some("test"),
              increment: Some(1.0),
              total: Some(2.0),
            });
       };
     });
};
