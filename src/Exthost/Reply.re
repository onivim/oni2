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
