open Oni_Core;
open Oni_Extensions;

module Internal = {
  let onExtensionMessage: Revery.Event.t(ExtHostClient.Terminal.msg) =
    Revery.Event.create();
};

[@deriving show({with_path: false})]
type msg =
  | ProcessStarted({
      id: int,
      pid: int,
    })
  | ProcessTitleChanged({
      id: int,
      title: string,
    })
  | TerminalScreenUpdated({
      id: int,
      screen: [@opaque] ReveryTerminal.Screen.t,
    })
  | TerminalCursorMoved({
      id: int,
      cursor: [@opaque] ReveryTerminal.Cursor.t,
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
        terminal: ReveryTerminal.t,
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

        let onEffect = eff =>
          switch (eff) {
          | ReveryTerminal.ScreenResized(screen)
          | ReveryTerminal.ScreenUpdated(screen) =>
            dispatch(TerminalScreenUpdated({id: params.id, screen}))
          | ReveryTerminal.CursorMoved(cursor) =>
            dispatch(TerminalCursorMoved({id: params.id, cursor}))
          // TODO: Handle output
          | _ => ()
          };

        let terminal =
          ReveryTerminal.make(
            ~rows=params.rows,
            ~columns=params.columns,
            ~onEffect,
          );

        let dispatchIfMatches = (id, msg) =>
          if (id == params.id) {
            dispatch(msg);
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
              dispatchIfMatches(
                terminalId,
                ProcessTitleChanged({id: terminalId, title}),
              )
            | SendProcessPid({terminalId, pid}) =>
              dispatchIfMatches(
                terminalId,
                ProcessStarted({id: terminalId, pid}),
              )
            | SendProcessData({terminalId, data}) =>
              if (terminalId == params.id) {
                let _: int = ReveryTerminal.write(~input=data, terminal);
                ();
              }
            | _ => ()
            }
          });

        {dispose, rows: params.rows, columns: params.columns, terminal};
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
