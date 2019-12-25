[@deriving show({with_path: false})]
type t = {
  kind,
  message: string,
}

and kind =
  | Success
  | Info
  | Warning
  | Error;

let create = (~kind=Info, message) => {kind, message};
