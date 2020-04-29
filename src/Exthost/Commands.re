[@deriving show]
type msg =
  | RegisterCommand(string)
  | UnregisterCommand(string)
  | ExecuteCommand({
      command: string,
      args: list(Yojson.Safe.t),
      retry: bool,
    })
  | GetCommands;

let handle = (method, args: Yojson.Safe.t) => {
  switch (method, args) {
  | ("$registerCommand", `List([`String(cmd)])) => Ok(RegisterCommand(cmd))
  | ("$unregisterCommand", `List([`String(cmd)])) =>
    Ok(UnregisterCommand(cmd))
  | ("$getCommands", _) => Ok(GetCommands)
  | (
      "$executeCommand",
      `List([`String(command), `List(args), `Bool(retry)]),
    ) =>
    Ok(ExecuteCommand({command, args, retry}))
  | _ => Error("Unhandled method: " ++ method)
  };
};
