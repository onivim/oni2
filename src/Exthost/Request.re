open Oni_Core;
module Commands = {
  let executeContributedCommand = (~arguments, ~command, client) => {
    Client.notify(
      ~rpcName="ExtHostCommands",
      ~method="$executeContributedCommand",
      ~args=`List([`String(command), ...arguments]),
      client,
    );
  };
};

module ExtensionService = {
  let activateByEvent = (~event, client) => {
    Client.notify(
      ~rpcName="ExtHostExtensionService",
      ~method="$activateByEvent",
      ~args=`List([`String(event)]),
      client,
    );
  };
};

module TerminalService = {
  let spawnExtHostProcess =
      (
        ~id,
        ~shellLaunchConfig,
        ~activeWorkspaceRoot,
        ~cols,
        ~rows,
        ~isWorkspaceShellAllowed,
        client,
      ) => {
    Client.notify(
      ~rpcName="ExtHostTerminalService",
      ~method="$spawnExtHostProcess",
      ~args=
        `List([
          `Int(id),
          ShellLaunchConfig.to_yojson(shellLaunchConfig),
          Uri.to_yojson(activeWorkspaceRoot),
          `Int(cols),
          `Int(rows),
          `Bool(isWorkspaceShellAllowed),
        ]),
      client,
    );
  };

  let acceptProcessInput = (~id, ~data, client) => {
    Client.notify(
      ~rpcName="ExtHostTerminalService",
      ~method="$acceptProcessInput",
      ~args=`List([`Int(id), `String(data)]),
      client,
    );
  };

  let acceptProcessResize = (~id, ~cols, ~rows, client) => {
    Client.notify(
      ~rpcName="ExtHostTerminalService",
      ~method="$acceptProcessResize",
      ~args=`List([`Int(id), `Int(cols), `Int(rows)]),
      client,
    );
  };

  let acceptProcessShutdown = (~id, ~immediate, client) => {
    Client.notify(
      ~rpcName="ExtHostTerminalService",
      ~method="$acceptProcessShutdown",
      ~args=`List([`Int(id), `Bool(immediate)]),
      client,
    );
  };
};
