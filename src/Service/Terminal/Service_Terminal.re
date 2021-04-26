open Oni_Core;

module Time = Revery.Time;

module Log = (val Log.withNamespace("Service_Terminal"));

module Internal = {
  let onExtensionMessage: Revery.Event.t(Exthost.Msg.TerminalService.msg) =
    Revery.Event.create();

  let idToTerminal: Hashtbl.t(int, ReveryTerminal.t) = Hashtbl.create(8);

  let getEnvironmentFromConfiguration =
      (env: Exthost.ShellLaunchConfig.environment) => {
    let augmentExistingEnvironment = (variablesToAdd: StringMap.t(string)) => {
      let existingEnv =
        switch (Luv.Env.environ()) {
        | Ok(env) =>
          env
          |> List.fold_left(
               (stringMap, cur) => {
                 let (key, v) = cur;
                 StringMap.add(key, v, stringMap);
               },
               StringMap.empty,
             )
        | Error(msg) =>
          Log.errorf(m =>
            m("Error getting environment: %s", Luv.Error.strerror(msg))
          );
          StringMap.empty;
        };

      StringMap.merge(
        (_key, original, augmented) => {
          switch (original, augmented) {
          | (None, None) => None
          | (Some(v), None) => Some(v)
          | (None, Some(v)) => Some(v)
          | (Some(_), Some(v)) => Some(v)
          }
        },
        existingEnv,
        variablesToAdd,
      );
    };

    Exthost.ShellLaunchConfig.(
      {
        switch (env) {
        | Inherit => augmentExistingEnvironment(StringMap.empty)
        | Additive(augmentedEnv) => augmentExistingEnvironment(augmentedEnv)
        | Strict(env) => env
        };
      }
    )
    |> StringMap.bindings;
  };
};

[@deriving show({with_path: false})]
type msg =
  | ProcessExit({
      id: int,
      exitCode: int,
    })
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
      cursor: [@opaque] ReveryTerminal.Cursor.t,
    });

module Sub = {
  type params = {
    setup: Setup.t,
    id: int,
    launchConfig: Exthost.ShellLaunchConfig.t,
    //    cmd: string,
    //    arguments: list(string),
    extHostClient: Exthost.Client.t,
    workspaceUri: Uri.t,
    rows: int,
    columns: int,
  };

  module TerminalSubscription =
    Isolinear.Sub.Make({
      type state = {
        maybePty: ref(option(Pty.t)),
        rows: int,
        columns: int,
        terminal: ReveryTerminal.t,
        isResizing: ref(bool),
      };

      type nonrec msg = msg;
      type nonrec params = params;

      let name = "Terminal";

      let id = ({id, _}) => string_of_int(id);

      let init = (~params, ~dispatch) => {
        let launchConfig = params.launchConfig;
        let maybePty = ref(None);

        let onEffect = eff =>
          switch (eff) {
          | ReveryTerminal.ScreenResized(_) => ()
          | ReveryTerminal.ScreenUpdated(_) => ()
          | ReveryTerminal.CursorMoved(_) => ()
          | ReveryTerminal.Output(output) =>
            maybePty^ |> Option.iter(pty => Pty.write(pty, output))
          // TODO: Handle term prop changes
          | ReveryTerminal.TermPropChanged(_) => ()
          };

        let rows = params.rows;
        let columns = params.columns;

        let terminal =
          ReveryTerminal.make(
            ~scrollBackSize=512,
            ~rows,
            ~columns,
            ~onEffect,
            (),
          );
        ReveryTerminal.resize(~rows, ~columns=40, terminal);
        let onData = data => {
          ReveryTerminal.write(~input=data, terminal);
          let cursor = ReveryTerminal.cursor(terminal);
          let screen = ReveryTerminal.screen(terminal);
          dispatch(ScreenUpdated({id: params.id, screen, cursor}));
        };

        let onExit = (~exitCode) => {
          dispatch(ProcessExit({id: params.id, exitCode}));
        };

        let onPidChanged = pid => {
          dispatch(ProcessStarted({id: params.id, pid}));
        };

        let onTitleChanged = title => {
          dispatch(ProcessTitleChanged({id: params.id, title}));
        };

        let env = Internal.getEnvironmentFromConfiguration(launchConfig.env);

        let ptyResult =
          Pty.start(
            ~setup=params.setup,
            ~env,
            ~cwd=Sys.getcwd(),
            ~rows,
            ~cols=columns,
            ~cmd=params.launchConfig.executable,
            ~arguments=params.launchConfig.arguments,
            ~onData,
            ~onExit,
            ~onPidChanged,
            ~onTitleChanged,
          );

        switch (ptyResult) {
        | Error(_) => ()
        | Ok(pty) => maybePty := Some(pty)
        };

        let isResizing = ref(false);

        Hashtbl.replace(Internal.idToTerminal, params.id, terminal);

        {maybePty, isResizing, rows, columns, terminal};
      };

      let update = (~params: params, ~state: state, ~dispatch as _) => {
        let rows = params.rows;
        let columns = params.columns;
        if (rows > 0
            && columns > 0
            && (rows != state.rows || columns != state.columns)) {
          state.isResizing := true;
          state.maybePty^ |> Option.iter(Pty.resize(~rows, ~cols=columns));
          ReveryTerminal.resize(~rows, ~columns, state.terminal);
          state.isResizing := false;
          {...state, rows, columns};
        } else {
          state;
        };
      };

      let dispose = (~params, ~state) => {
        switch (state.maybePty^) {
        | None => ()
        | Some(pty) => Pty.close(pty)
        };

        state.maybePty := None;

        Hashtbl.remove(Internal.idToTerminal, params.id);
      };
    });

  let terminal =
      (
        ~setup,
        ~id,
        ~launchConfig,
        ~columns,
        ~rows,
        ~workspaceUri,
        ~extHostClient,
      ) =>
    TerminalSubscription.create({
      setup,
      id,
      launchConfig,
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

  let paste = (~id, input) => {
    Isolinear.Effect.create(~name="terminal.input", () => {
      switch (Hashtbl.find_opt(Internal.idToTerminal, id)) {
      | Some(terminal) =>
        input
        |> Zed_utf8.iter(uchar => {
             ReveryTerminal.input(~key=Unicode(uchar), terminal)
           })
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
