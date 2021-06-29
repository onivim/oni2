type t =
  | Nothing
  | OkEmpty
  | OkBuffer({bytes: Bytes.t})
  | OkJson({json: Yojson.Safe.t})
  | ErrorJson({error: Yojson.Safe.t})
  | ErrorMessage({message: string});

let none = Nothing;

let okEmpty = OkEmpty;

let error = message => ErrorMessage({message: message});

let errorJson = json => ErrorJson({error: json});

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
  | ErrorJson({error}) =>
    Printf.sprintf("ErrorMessage(%s)", Yojson.Safe.to_string(error))
  | ErrorMessage({message}) => Printf.sprintf("ErrorMessage(%s)", message);
