open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Extensions.Terminal"));

module ShellLaunchConfig = {
  [@deriving show({with_path: false})]
  type t = {
    name: string,
    executable: string,
    arguments: list(string),
  };

  let to_yojson = ({name, executable, arguments}) => {
    let args = arguments |> List.map(s => `String(s));
    `Assoc([
      ("name", `String("Hello")),
      ("executable", `String("bash")),
      ("args", `List([])),
    ]);
  };
};

// MSG
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

let handleMessage = (~dispatch, method, args) => {
  switch (method) {
  | "$sendProcessTitle" =>
    switch (args) {
    | [`Int(terminalId), `String(title)] =>
      dispatch(SendProcessTitle({terminalId, title}))
    | _ => Log.error("Unexpected arguments for $sendProcessTitle")
    }

  | "$sendProcessData" =>
    switch (args) {
    | [`Int(terminalId), `String(data)] =>
      dispatch(SendProcessData({terminalId, data}))
    | _ => Log.error("Unexpected arguments for $sendProcessData")
    }

  | "$sendProcessPid" =>
    switch (args) {
    | [`Int(terminalId), `Int(pid)] =>
      dispatch(SendProcessPid({terminalId, pid}))

    | _ => Log.error("Unexpected arguments for $sendProcessPid")
    }

  | "$sendProcessExit" =>
    switch (args) {
    | [`Int(terminalId), `Int(exitCode)] =>
      dispatch(SendProcessExit({terminalId, exitCode}))

    | _ => Log.error("Unexpected arguments for $sendProcessExit")
    }

  | _ =>
    Log.warnf(m =>
      m(
        "Unhandled Terminal message - %s: %s",
        method,
        Yojson.Safe.to_string(`List(args)),
      )
    )
  };
};

// REQUESTS
module Requests = {
  let createProcess = (id, launchConfig, workspaceUri, columns, rows, client) => {
    let () =
      ExtHostTransport.send(
        client,
        ExtHostProtocol.OutgoingNotifications._buildNotification(
          "ExtHostTerminalService",
          "$createProcess",
          `List([
            `Int(id),
            ShellLaunchConfig.to_yojson(launchConfig),
            //Uri.to_yojson(workspaceUri),
            `Assoc([]),
            `Int(columns),
            `Int(rows),
          ]),
        ),
      );
    ();
  };

  let acceptProcessResize = (id, columns, rows, client) => {
    ExtHostTransport.send(
      client,
      ExtHostProtocol.OutgoingNotifications._buildNotification(
        "ExtHostTerminalService",
        "$acceptProcessResize",
        `List([`Int(id), `Int(columns), `Int(rows)]),
      ),
    );
  };

  let acceptProcessInput = (id, data, client) => {
    ExtHostTransport.send(
      client,
      ExtHostProtocol.OutgoingNotifications._buildNotification(
        "ExtHostTerminalService",
        "$acceptProcessInput",
        `List([`Int(id), `String(data)]),
      ),
    );
  };

  let acceptProcessShutdown = (~immediate=false, id, client) => {
    ExtHostTransport.send(
      client,
      ExtHostProtocol.OutgoingNotifications._buildNotification(
        "ExtHostTerminalService",
        "$acceptProcessShutdown",
        `List([`Int(id), `Bool(immediate)]),
      ),
    );
  };
};
