open Oni_Core;
open Oni_Extensions;

module Internal = {
  let onExtensionMessage: Revery.Event.t(ExtHostClient.Terminal.msg) =
    Revery.Event.create();

  let idToTerminal: Hashtbl.t(int, ReveryTerminal.t) = Hashtbl.create(8);
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
  | ScreenUpdated({
      id: int,
      screen: [@opaque] ReveryTerminal.Screen.t,
    })
  | CursorMoved({
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
        isResizing: ref(bool),
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

        let isResizing = ref(false);

        let onEffect = eff =>
          switch (eff) {
          | ReveryTerminal.ScreenResized(_) => ()
          | ReveryTerminal.ScreenUpdated(screen) =>
            if (! isResizing^) {
              dispatch(ScreenUpdated({id: params.id, screen}));
            }
          | ReveryTerminal.CursorMoved(cursor) =>
            dispatch(CursorMoved({id: params.id, cursor}))
          | ReveryTerminal.Output(output) =>
            ExtHostClient.Terminal.Requests.acceptProcessInput(
              params.id,
              output,
              params.extHostClient,
            )
          // TODO: Handle term prop changes
          | ReveryTerminal.TermPropChanged(_) => ()
          };

        let terminal =
          ReveryTerminal.make(
            ~scrollBackSize=512,
            ~rows=params.rows,
            ~columns=params.columns,
            ~onEffect,
            (),
          );

        Hashtbl.replace(Internal.idToTerminal, params.id, terminal);

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
                ReveryTerminal.write(~input=data, terminal);
              }
            | _ => ()
            }
          });

        {
          dispose,
          isResizing,
          rows: params.rows,
          columns: params.columns,
          terminal,
        };
      };

      let update = (~params: params, ~state: state, ~dispatch as _) => {
        if (params.rows != state.rows || params.columns != state.columns) {
          ExtHostClient.Terminal.Requests.acceptProcessResize(
            params.id,
            params.columns,
            params.rows,
            params.extHostClient,
          );

          state.isResizing := true;
          ReveryTerminal.resize(
            ~rows=params.rows,
            ~columns=params.columns,
            state.terminal,
          );
          state.isResizing := false;
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

        Hashtbl.remove(Internal.idToTerminal, params.id);
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
  open Vterm;

  let keyToVtermKey =
    [
      ("<CR>", Enter),
      ("<BS>", Backspace),
      ("<TAB>", Tab),
      ("<UP>", Up),
      ("<LEFT>", Left),
      ("<RIGHT>", Right),
      ("<DOWN>", Right),
      ("<ESC>", Escape),
    ]
    |> List.to_seq
    |> Hashtbl.of_seq;

  let controlKeyRegex = Oniguruma.OnigRegExp.create("<C-([a-z])>");

  let getControlKey = keyCandidate => {
    Oniguruma.(
      controlKeyRegex
      |> Stdlib.Result.map(OnigRegExp.search(keyCandidate, 0))
      |> Stdlib.Result.to_option
      |> Utility.OptionEx.flatMap(matches =>
           if (Array.length(matches) < 2) {
             None;
           } else {
             let key = OnigRegExp.Match.getText(matches[1]);
             if (String.length(key) == 1) {
               Some(key.[0]);
             } else {
               None;
             };
           }
         )
    );
  };

  let input = (~id, input) => {
    Isolinear.Effect.create(~name="terminal.input", () => {
      switch (Hashtbl.find_opt(Internal.idToTerminal, id)) {
      | Some(terminal) =>
        switch (getControlKey(input)) {
        | Some(key) =>
          let keyChar = key |> Char.code |> Uchar.of_int;
          ReveryTerminal.input(
            ~key=Unicode(keyChar),
            ~modifier=Control,
            terminal,
          );
        | None =>
          if (String.length(input) == 1) {
            let key = input.[0] |> Char.code |> Uchar.of_int;
            ReveryTerminal.input(~key=Unicode(key), terminal);
          } else {
            switch (Hashtbl.find_opt(keyToVtermKey, input)) {
            | Some(key) => ReveryTerminal.input(~key, terminal)
            | None => ()
            };
          }
        }
      | None => ()
      }
    });
  };
};

let handleExtensionMessage = (msg: ExtHostClient.Terminal.msg) => {
  Revery.Event.dispatch(Internal.onExtensionMessage, msg);
};
