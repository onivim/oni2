/*
 * InputStoreConnector.re
 *
 * This module connects external user input to the store.
 */

open Oni_Core;
open Oni_Input;

module Model = Oni_Model;
module State = Model.State;
module Actions = Model.Actions;

type captureMode =
  | Normal
  | Wildmenu
  | Quickmenu;

let conditionsOfState = (state: State.t) => {
  // Not functional, but we'll use the hashtable for performance
  let ret: Handler.Conditions.t = Hashtbl.create(16);

  if (state.quickmenu != None) {
    Hashtbl.add(ret, MenuFocus, true);
  }

  // HACK: Because we don't have AND conditions yet for input
  // (the conditions array are OR's), we are making `insertMode`
  // only true when the editor is insert mode AND we are in the
  // editor (editorTextFocus is set)
  switch (state.quickmenu != None, state.mode) {
  | (false, Vim.Types.Insert) =>
    Hashtbl.add(ret, Types.Input.InsertMode, true);
    Hashtbl.add(ret, Types.Input.EditorTextFocus, true);
  | (false, _) => Hashtbl.add(ret, Types.Input.EditorTextFocus, true)
  | _ => ()
  };

  ret;
};

let start =
    (
      getState: unit => Model.State.t,
      window: option(Revery.Window.t),
      runEffects,
    ) => {
  let (stream, dispatch) = Isolinear.Stream.create();

  let getActionsForBinding =
      (inputKey, commands, currentConditions: Handler.Conditions.t) => {
    Keybindings.(
      List.fold_left(
        (defaultAction, {key, command, condition}) =>
          Handler.matchesCondition(
            condition,
            currentConditions,
            inputKey,
            key,
          )
            ? [Actions.Command(command)] : defaultAction,
        [],
        commands,
      )
    );
  };


  /**
     The key handlers return (keyPressedString, shouldOniListen)
     i.e. if ctrl or alt or cmd were pressed then Oni2 should listen
     /respond to commands otherwise if input is alphabetical AND
     a revery element is focused oni2 should defer to revery
   */
  let keyEventListener = key => {
    let state = getState();
    let commands = state.keyBindings;
    let conditions = conditionsOfState(state);
    let time = Revery.Time.getTime() |> Revery.Time.toSeconds;

    let captureMode =
      switch (state.quickmenu) {
      | Some({ variant: Wildmenu(_) }) =>
        Wildmenu

      | Some({ variant: CommandPalette })
      | Some({ variant: Buffers })
      | Some({ variant: WorkspaceFiles }) =>
        Quickmenu

      | None =>
        Normal
      };

    let isUnfocused =
      Revery.UI.Focus.focused^ == None;

    let shouldListen =
      switch key {
      | Some((_, true)) => true
      | Some((_, false)) when isUnfocused => true
      | _ => false
      };

    switch (key) {
    | None =>
      ()

    | Some((k, _)) =>
      let bindingActions =
        getActionsForBinding(k, commands, conditions);

      let actions = 
        switch (captureMode) {
          | Normal
          | Wildmenu when bindingActions == [] && shouldListen =>
            Log.info("Input::handle - sending raw input: " ++ k);
            [Actions.KeyboardInput(k)]

          | _ =>
            Log.info("Input::handle - sending bound actions.");
            bindingActions
        };

      [Actions.NotifyKeyPressed(time, k), ...actions]
        |> List.iter(dispatch);
    };

    // Run input effects _immediately_
    runEffects();
  };

  switch (window) {
  | None =>
    Log.info("Input - no window to subscribe to events")

  | Some(window) =>
    Revery.Event.subscribe(window.onKeyDown, keyEvent =>
      Handler.keyPressToCommand(keyEvent, Revery_Core.Environment.os)
      |> keyEventListener
    )
    |> ignore;

    Reglfw.Glfw.glfwSetCharModsCallback(
      window.glfwWindow, (_w, codepoint, mods) =>
      Handler.charToCommand(codepoint, mods) |> keyEventListener
    );
  };

  stream;
};
