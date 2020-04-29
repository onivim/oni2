[@deriving show]
type msg =
  | SendProcessTitle({
      terminalId: int,
      title: string,
    })
  | SendProcessData({
      terminalId: int,
      data: string,
    })
  | SendProcessPid({
      terminalId: int,
      pid: int,
    })
  | SendProcessExit({
      terminalId: int,
      exitCode: int,
    });

let handle = (method, args: Yojson.Safe.t) => {
  switch (method) {
  | "$sendProcessTitle" =>
    switch (args) {
    | `List([`Int(terminalId), `String(title)]) =>
      Ok(SendProcessTitle({terminalId, title}))
    | _ => Error("Unexpected arguments for $sendProcessTitle")
    }

  | "$sendProcessData" =>
    switch (args) {
    | `List([`Int(terminalId), `String(data)]) =>
      Ok(SendProcessData({terminalId, data}))
    | _ => Error("Unexpected arguments for $sendProcessData")
    }

  | "$sendProcessPid" =>
    switch (args) {
    | `List([`Int(terminalId), `Int(pid)]) =>
      Ok(SendProcessPid({terminalId, pid}))

    | _ => Error("Unexpected arguments for $sendProcessPid")
    }

  | "$sendProcessExit" =>
    switch (args) {
    | `List([`Int(terminalId), `Int(exitCode)]) =>
      Ok(SendProcessExit({terminalId, exitCode}))

    | _ => Error("Unexpected arguments for $sendProcessExit")
    }

  | _ =>
    Error(
      Printf.sprintf(
        "Unhandled Terminal message - %s: %s",
        method,
        Yojson.Safe.to_string(args),
      ),
    )
  };
};
