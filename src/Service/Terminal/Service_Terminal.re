open Oni_Core;
open Oni_Core.Utility;

module Time = Revery.Time;

module Internal = {
  let onExtensionMessage: Revery.Event.t(Exthost.Msg.TerminalService.msg) =
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
    arguments: list(string),
    extHostClient: Exthost.Client.t,
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

      let name = "Terminal";

      let id = ({id, _}) => string_of_int(id);

      let init = (~params, ~dispatch) => {
        let launchConfig =
          Exthost.ShellLaunchConfig.{
            name: "Terminal",
            executable: params.cmd,
            arguments: params.arguments,
          };

        let isResizing = ref(false);

        let throttledScreenDispatch =
          FunEx.throttle(~time=Time.zero, dispatch);
        let throttledCursorDispatch =
          FunEx.throttle(~time=Time.zero, dispatch);

        let onEffect = eff =>
          switch (eff) {
          | ReveryTerminal.ScreenResized(_) => ()
          | ReveryTerminal.ScreenUpdated(screen) =>
            if (! isResizing^) {
              throttledScreenDispatch(
                ScreenUpdated({id: params.id, screen}),
              );
            }
          | ReveryTerminal.CursorMoved(cursor) =>
            throttledCursorDispatch(CursorMoved({id: params.id, cursor}))
          | ReveryTerminal.Output(output) =>
            Exthost.Request.TerminalService.acceptProcessInput(
              ~id=params.id,
              ~data=output,
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

        Exthost.Request.TerminalService.spawnExtHostProcess(
          ~id=params.id,
          ~shellLaunchConfig=launchConfig,
          ~activeWorkspaceRoot=params.workspaceUri,
          ~cols=params.columns,
          ~rows=params.rows,
          ~isWorkspaceShellAllowed=true,
          params.extHostClient,
        );

        let dispose =
          Revery.Event.subscribe(
            Internal.onExtensionMessage,
            (msg: Exthost.Msg.TerminalService.msg) => {
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
        let rows = params.rows;
        let columns = params.columns;
        if (rows > 0
            && columns > 0
            && (rows != state.rows || columns != state.columns)) {
          Exthost.Request.TerminalService.acceptProcessResize(
            ~id=params.id,
            ~cols=columns,
            ~rows=rows,
            params.extHostClient,
          );

          state.isResizing := true;
          ReveryTerminal.resize(~rows, ~columns, state.terminal);
          state.isResizing := false;
          {...state, rows, columns};
        } else {
          state;
        };
      };

      let dispose = (~params, ~state) => {
        let () =
          Exthost.Request.TerminalService.acceptProcessShutdown(
            ~immediate=false,
            ~id=params.id,
            params.extHostClient,
          );

        Hashtbl.remove(Internal.idToTerminal, params.id);
        state.dispose();
      };
    });

  let terminal =
      (~id, ~arguments, ~cmd, ~columns, ~rows, ~workspaceUri, ~extHostClient) =>
    TerminalSubscription.create({
      id,
      arguments,
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
      ("<DOWN>", Down),
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

let handleExtensionMessage = (msg: Exthost.Msg.TerminalService.msg) => {
  Revery.Event.dispatch(Internal.onExtensionMessage, msg);
};

let getScreen = terminalId => {
  terminalId
  |> Hashtbl.find_opt(Internal.idToTerminal)
  |> Option.map(ReveryTerminal.screen);
};
