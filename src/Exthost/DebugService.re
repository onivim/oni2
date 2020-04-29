[@deriving show]
type msg =
  | RegisterDebugTypes(list(string));

let handle = (method, args: Yojson.Safe.t) => {
  switch (method, args) {
  | ("$registerDebugTypes", `List([`List(items)])) =>
    let types =
      items
      |> List.filter_map(
           fun
           | `String(str) => Some(str)
           | _ => None,
         );
    Ok(RegisterDebugTypes(types));
  | _ => Error("Unhandled method: " ++ method)
  };
};
