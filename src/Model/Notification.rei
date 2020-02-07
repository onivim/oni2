[@deriving show]
type t =
  pri {
    id: int,
    kind,
    message: string,
  }

and kind =
  | Success
  | Info
  | Warning
  | Error;

let create: (~kind: kind=?, string) => t;
