open Oni_Core;

[@deriving show]
type severity =
  | Ignore
  | Info
  | Warning
  | Error;

let intToSeverity =
  fun
  | 0 => Ignore
  | 1 => Info
  | 2 => Warning
  | 3 => Error
  | _ => Ignore;

[@deriving show]
type handle = int;

module Command = {
  [@deriving show]
  type t = {
    title: string,
    isCloseAffordance: bool,
    handle,
  };

  let decode = {
    Json.Decode.(
      obj(({field, _}) =>
        {
          title: field.required("title", string),
          isCloseAffordance: field.required("isCloseAffordance", bool),
          handle: field.required("handle", int),
        }
      )
    );
  };
};
