/*
 * KeyBindingsStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for managing key bindings
 */

open Oni_Core;
open Oni_Model;

module Log = (val Log.withNamespace("Oni2.Store.Keybindings"));

module Commands = GlobalCommands;

let start = () => {
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
    | KeybindingInvoked({command, arguments}) =>
      if (command |> Utility.StringEx.startsWith(~prefix=":")) {
        (
          state,
          executeExCommandEffect(Base.String.drop_prefix(command, 1)),
        );
      } else {
        switch (Command.Lookup.get(command, CommandManager.current(state))) {
        | Some(command: Command.t(_)) => (
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
