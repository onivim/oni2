type t =
  | Nothing
  | OkEmpty
  | OkBuffer({bytes: Bytes.t})
  | OkJson({json: Yojson.Safe.t})
  | ErrorMessage({message: string});

let none = Nothing;

let okEmpty = OkEmpty;

let error = message => ErrorMessage({message: message});

let okBuffer = bytes => OkBuffer({bytes: bytes});
let okJson = json => OkJson({json: json});

let toDebugString =
  fun
  | Nothing => "Nothing"
  | OkEmpty => "OkEmpty"
  | OkBuffer({bytes}) =>
    Printf.sprintf("OkBuffer(%s)", Bytes.to_string(bytes))
  | OkJson({json}) =>
    Printf.sprintf("OkJson(%s)", Yojson.Safe.to_string(json))
  | ErrorMessage({message}) => Printf.sprintf("ErrorMessage(%s)", message);
