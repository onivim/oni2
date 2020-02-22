open Oni_Core;
open Oni_Extensions;

module Internal = {
  let onExtensionMessage: Revery.Event.t(ExtHostClient.Terminal.msg) =
    Revery.Event.create();
};

type msg =
  | ProcessStarted({
      id: int,
      pid: int,
    })
  | ProcessTitleChanged({
      id: int,
      title: string,
    });

module Sub = {
  type params = {
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
        rows: int,
        columns: int,
        dispose: unit => unit,
      };

      type nonrec msg = msg;
      type nonrec params = params;

      let subscriptionName = "Terminal";

      let getUniqueId = ({id, _}) => string_of_int(id);

      let init = (~params, ~dispatch) => {
        let launchConfig =
          ExtHostClient.Terminal.ShellLaunchConfig.{
            name: "Terminal",
            executable: params.cmd,
            arguments: [],
          };

        ExtHostClient.Terminal.Requests.createProcess(
          params.id,
          launchConfig,
          params.workspaceUri,
          params.columns,
          params.rows,
          params.extHostClient,
        );

        let dispose =
          Revery.Event.subscribe(
            Internal.onExtensionMessage, (msg: ExtHostClient.Terminal.msg) => {
            switch (msg) {
            | SendProcessTitle({terminalId, title}) =>
              dispatch(ProcessTitleChanged({id: terminalId, title}))
            | SendProcessPid({terminalId, pid}) =>
              dispatch(ProcessStarted({id: terminalId, pid}))
            | _ => ()
            }
          });

        {dispose, rows: params.rows, columns: params.columns};
      };

      let update = (~params: params, ~state: state, ~dispatch as _) => {
        if (params.rows != state.rows || params.columns != state.columns) {
          ExtHostClient.Terminal.Requests.acceptProcessResize(
            params.id,
            params.rows,
            params.columns,
            params.extHostClient,
          );
        };

        {...state, rows: params.rows, columns: params.columns};
      };

      let dispose = (~params, ~state) => {
        let () =
          ExtHostClient.Terminal.Requests.acceptProcessShutdown(
            ~immediate=false,
            params.id,
            params.extHostClient,
          );

        state.dispose();
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
  let input = (~id as _, ~input as _, _extHostClient) => Isolinear.Effect.none;
};

let handleExtensionMessage = (msg: ExtHostClient.Terminal.msg) => {
  Revery.Event.dispatch(Internal.onExtensionMessage, msg);
};
