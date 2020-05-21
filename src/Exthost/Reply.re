type t =
  | Nothing
  | OkEmpty
  | OkJson({json: Yojson.Safe.t})
  | ErrorMessage({message: string});

let none = Nothing;

let okEmpty = OkEmpty;

let error = message => ErrorMessage({message: message});

let okJson = json => OkJson({json: json});
