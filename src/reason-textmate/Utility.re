module JsonEx = {
  let from_file = file =>
    try(Ok(Yojson.Safe.from_file(file))) {
    | e => Error(Printexc.to_string(e))
    };
};
