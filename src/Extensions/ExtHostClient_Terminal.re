open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Extensions.Terminal"));

// MODEL
module ShellLaunchConfig  = {
  [@deriving show({with_path: false})]
  type t = {
    name: string,
    executable: string,
    arguments: list(string),
  };
}

// UPDATE

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

let handleMessage = (~dispatch, method, args) =>
  switch (method) {
  | "$sendProcessTitle" =>
    switch (args) {
    | [`Int(terminalId), `String(title)] =>
      dispatch(SendProcessTitle({terminalId, title}));
    | _ => Log.error("Unexpected arguments for $sendProcessTitle")
    }

  | "$sendProcessData" =>
    switch (args) {
  | [`Int(terminalId), `String(data)] =>
      dispatch(SendProcessData({terminalId, data}));
    | _ => Log.error("Unexpected arguments for $sendProcessData")
    }

  | "$sendProcessPid" =>
    switch (args) {
    | [`Int(terminalId), `Int(pid)] => 
      dispatch(SendProcessPid({terminalId, pid}));

    | _ => Log.error("Unexpected arguments for $sendProcessPid")
    }
  
  | "$sendProcessExit" =>
    switch (args) {
    | [`Int(terminalId), `Int(exitCode)] => 
      dispatch(SendProcessExit({terminalId, exitCode}));

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

// REQUESTS

module Requests = {
  let createProcess = (id, launchConfig, workspaceUri, columns, rows, client) => 
    ExtHostTransport.send(
      client,
      ExtHostProtocol.OutgoingNotifications._buildNotification(
        "ExtHostTerminalService",
        "$createProcess",
        `List([
          `Int(id),
          // TODO: launchConfig
          `Assoc([]),
          Uri.to_yojson(workspaceUri),
          columns,
          rows,
          ]),
      )
    );
};
