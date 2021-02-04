/*
 * KeyBindingsStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for managing key bindings
 */

open Oni_Core;
open Oni_Input;
open Oni_Model;

module Log = (val Log.withNamespace("Oni2.Store.Keybindings"));

module Commands = GlobalCommands;

let start = maybeKeyBindingsFilePath => {
  let getKeybindingsFile = () => {
    Filesystem.getOrCreateConfigFile(
      ~overridePath=?maybeKeyBindingsFilePath,
      "keybindings.json",
    );
  };

  let reloadConfigOnWritePost = (~configPath, dispatch) => {
    let _: unit => unit =
      Vim.AutoCommands.onDispatch((cmd, buffer) => {
        let bufferFileName =
          switch (Vim.Buffer.getFilename(buffer)) {
          | None => ""
          | Some(fileName) => fileName
          };
        if (bufferFileName == configPath && cmd == Vim.Types.BufWritePost) {
          Log.info("Reloading key bindings from: " ++ configPath);
          dispatch(Actions.KeyBindingsReload);
        };
      });
    ();
  };

  let loadKeyBindingsEffect = isFirstLoad =>
    Isolinear.Effect.createWithDispatch(~name="keyBindings.load", dispatch => {
      getKeybindingsFile()
      |> Utility.ResultEx.tapError(err => {
           Log.warnf(m => m("Error getting keybindings file: %s", err))
         })
      |> Result.iter(keyBindingsFile => {
           let checkFirstLoad = keyBindingPath =>
             if (isFirstLoad) {
               reloadConfigOnWritePost(~configPath=keyBindingPath, dispatch);
             };

           let onError = msg => {
             let errorMsg = "Error parsing keybindings: " ++ msg;
             Log.error(errorMsg);
             dispatch(Actions.KeyBindingsParseError(errorMsg));
           };

           let (keyBindings, individualErrors) =
             keyBindingsFile
             |> Fp.toString
             |> Utility.FunEx.tap(checkFirstLoad)
             |> Utility.JsonEx.from_file
             |> Utility.ResultEx.flatMap(Keybindings.of_yojson_with_errors)
             // Handle error case when parsing entire JSON file
             |> Utility.ResultEx.tapError(onError)
             |> Stdlib.Result.value(~default=([], []));

           // Handle individual binding errors
           individualErrors |> List.iter(onError);

           Log.infof(m =>
             m("Loading %i keybindings", List.length(keyBindings))
           );

           keyBindings
           |> List.iteri((idx, binding) =>
                Log.tracef(m =>
                  m(
                    "Binding %d: %s",
                    idx,
                    Feature_Input.Schema.resolvedToString(binding),
                  )
                )
              );

           dispatch(
             Actions.Input(
               Feature_Input.Msg.keybindingsUpdated(keyBindings),
             ),
           );
         })
    });

  let executeCommandEffect = (msg, arguments) =>
    Isolinear.Effect.createWithDispatch(
      ~name="keybindings.executeCommand", dispatch =>
      switch (msg) {
      | `Arg0(msg) => dispatch(msg)
      | `Arg1(msgf) => dispatch(msgf(arguments))
      }
    );

  let executeExCommandEffect = command =>
    Isolinear.Effect.createWithDispatch(
      ~name="keybindings.executeExCommand", dispatch =>
      dispatch(Actions.VimExecuteCommand({allowAnimation: true, command}))
    );

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | Actions.Init => (state, loadKeyBindingsEffect(true))

    | Actions.KeyBindingsReload => (state, loadKeyBindingsEffect(false))

    | Actions.KeyBindingsParseError(msg) => (
        state,
        Feature_Notification.Effects.create(~kind=Error, msg)
        |> Isolinear.Effect.map(msg => Actions.Notification(msg)),
      )

    | CommandInvoked({command, arguments}) =>
      if (command |> Utility.StringEx.startsWith(~prefix=":")) {
        (
          state,
          executeExCommandEffect(Base.String.drop_prefix(command, 1)),
        );
      } else {
        switch (Command.Lookup.get(command, CommandManager.current(state))) {
        | Some((command: Command.t(_))) => (
            state,
            executeCommandEffect(command.msg, arguments),
          )
        | None =>
          Log.errorf(m => m("Unknown command: %s", command));
          (state, Isolinear.Effect.none);
        };
      }

    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
