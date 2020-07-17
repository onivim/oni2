open Oni_Core;

module Location = {
    type t = 
    | Explorer
    | SCM
    | Extensions
    | Window
    | Notification
    | Dialog
    | Other(string);

    let ofInt = fun
    | 1 => Some(Explorer)
    | 3 => Some(SCM)
    | 5 => Some(Extensions)
    | 10 => Some(Window)
    | 15 => Some(Notification)
    | 20 => Some(Dialog)
    | _ => None;
    

    module Decode = {
        open Json.Decode;
        let int = int |> map(ofInt) |> and_then(fun
        | None => fail("Unrecognized location")
        | Some(location) => succeed(location));

        let string = string |> map(str => Other(str));
    };

    let decode = Json.Decode.one_of([
        ("int", Decode.int),
        ("string", Decode.string)
    ]);
};

module Options = {
    type t = {
        location: Location.t,
        title: option(string),
        source: option(string),
        total: option(int),
        cancellable: bool,
        buttons: list(string),
    };

    let decode = Json.Decode.(obj(({field, _}) => {
       location: field.required("location", Location.decode), 
       title: field.optional("title", string),
       source: field.optional("source", string),
       total: field.optional("total", int),
       cancellable: field.withDefault("cancellable", false, bool),
       buttons: field.withDefault("buttons", [], list(string)),
    }));
    
};

module Step = {
    type t = {
        message: option(string),
        increment: option(int),
        total: option(int),
    };

    let decode = Json.Decode.(obj(({field, _}) => {
        message: field.optional("title", string),
        increment: field.optional("increment", int),
        total: field.optional("total", int),
    }));
};
