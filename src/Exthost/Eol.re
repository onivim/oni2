open Oni_Core;

[@deriving (show({with_path: false}), yojson({strict: false}))]
type t =
  | [@name "\n"] LF
  | [@name "\r\n"] CRLF;

let default = Sys.win32 ? CRLF : LF;

let toString = (v: t) =>
  switch (v) {
  | CRLF => "\r\n"
  | LF => "\n"
  };

let sizeInBytes =
  fun
  | LF => 1
  | CRLF => 2;

let encode = eol => eol |> toString |> Json.Encode.string;
