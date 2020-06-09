type t =
  | CR
  | LF
  | CRLF;

let toString =
  fun
  | LF => "LF"
  | CR => "CR"
  | CRLF => "CRLF";
