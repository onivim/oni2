open Oni_Core;
open Oni_Extensions;

module Msg = {
  type t =
    | Resized(unit)
    | Updated(unit);
};

module Sub = {
  type terminalSubParams = {
    id: int,
    cmd: string,
    extHostClient: ExtHostClient.t,
    workspaceUri: Uri.t,
    rows: int,
    columns: int,
  };

  module TerminalSubscription =
    Isolinear.Sub.Make({
      type state = {
        currentRows: int,
        currentColumns: int,
      };

      type msg = Msg.t;
      type params = terminalSubParams;

      let subscriptionName = "Terminal";

      let getUniqueId = ({id, _}) => string_of_int(id);

      let init = (~params, ~dispatch) => {
        let launchConfig =
          ExtHostClient.Terminal.ShellLaunchConfig.{
            name: "Terminal",
            executable: params.cmd,
            arguments: [],
          };

        let () =
          ExtHostClient.Terminal.Requests.createProcess(
            params.id,
            launchConfig,
            params.workspaceUri,
            params.columns,
            params.rows,
            params.extHostClient,
          );

        {currentRows: params.rows, currentColumns: params.columns};
      };

      let update = (~params, ~state, ~dispatch) => {
        if (params.rows != state.currentRows
            || params.columns != state.currentColumns) {
          let () =
            ExtHostClient.Terminal.Requests.acceptProcessResize(
              params.id,
              params.rows,
              params.columns,
              params.extHostClient,
            );
          ();
        };

        {currentRows: params.rows, currentColumns: params.columns};
      };

      let dispose = (~params, ~state) => {
        let () =
          ExtHostClient.Terminal.Requests.acceptProcessShutdown(
            ~immediate=false,
            params.id,
            params.extHostClient,
          );
        ();
      };
    });

  let terminal = (~id, ~cmd, ~columns, ~rows, ~workspaceUri, ~extHostClient) =>
    TerminalSubscription.create({
      id,
      cmd,
      columns,
      rows,
      workspaceUri,
      extHostClient,
    });
};

module Effect = {
  let input = (~id, ~input, extHostClient) => Isolinear.Effect.none;
};
