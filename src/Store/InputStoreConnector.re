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

module Log = (val Log.withNamespace("Oni2.InputStoreConnector"));

type captureMode =
  | Normal
  | Wildmenu
  | Quickmenu;

let isQuickmenuOpen = (state: State.t) => state.quickmenu != None;

let conditionsOfState = (state: State.t) => {
  // Not functional, but we'll use the hashtable for performance
  let ret: Hashtbl.t(string, bool) = Hashtbl.create(16);

  switch (state.quickmenu) {
  | Some({variant, query, cursorPosition, _}) =>
    Hashtbl.add(ret, "listFocus", true);
    Hashtbl.add(ret, "inQuickOpen", true);

    if (cursorPosition == String.length(query)) {
      Hashtbl.add(ret, "quickmenuCursorEnd", true);
    };

    if (variant == EditorsPicker) {
      Hashtbl.add(ret, "inEditorsPicker", true);
    };

  | None => ()
  };

  if (Model.Completions.isActive(state.completions)) {
    Hashtbl.add(ret, "suggestWidgetVisible", true);
  };

  // HACK: Because we don't have AND conditions yet for input
  // (the conditions array are OR's), we are making `insertMode`
  // only true when the editor is insert mode AND we are in the
  // editor (editorTextFocus is set)
  switch (isQuickmenuOpen(state), state.mode) {
  | (false, Vim.Types.Insert) =>
    Hashtbl.add(ret, "insertMode", true);
    Hashtbl.add(ret, "editorTextFocus", true);
  | (false, _) => Hashtbl.add(ret, "editorTextFocus", true)
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

  // We also use 'text input' mode for SDL2.
  // This enables us to get resolved keyboard events, and IME.

  // For IME: Is this sufficient? Or will we need a way to turn off / toggle IME when switching modes?
  Sdl2.TextInput.start();

  let getActionsForBinding =
      (inputKey, bindings, currentConditions: Hashtbl.t(string, bool)) => {
    let inputKey = String.uppercase_ascii(inputKey);

    let getValue = v =>
      switch (Hashtbl.find_opt(currentConditions, v)) {
      | Some(variableValue) => variableValue
      | None => false
      };

    Keybindings.Keybinding.(
      List.fold_left(
        (defaultAction, {key, command, condition}) =>
          Handler.matchesCondition(condition, inputKey, key, getValue)
            ? [Actions.Command(command)] : defaultAction,
        [],
        bindings,
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
    let bindings = state.keyBindings;
    let conditions = conditionsOfState(state);
    let time = Revery.Time.now() |> Revery.Time.toFloatSeconds;

    let captureMode =
      switch (state.quickmenu) {
      | Some({variant: Wildmenu(_), _}) => Wildmenu

      | Some({variant: CommandPalette, _})
      | Some({variant: EditorsPicker, _})
      | Some({variant: FilesPicker, _})
      | Some({variant: ThemesPicker, _}) => Quickmenu

      | None => Normal
      };

    switch (key) {
    | None => ()

    | Some(k) =>
      let bindingActions = getActionsForBinding(k, bindings, conditions);

      let actions =
        switch (captureMode) {
        | Normal
        | Wildmenu
            when bindingActions == [] && Revery.UI.Focus.focused^ == None =>
          Log.info("handle - sending raw input: " ++ k);
          [Actions.KeyboardInput(k)];

        | _ =>
          Log.info("handle - sending bound actions.");
          bindingActions;
        };

      [Actions.NotifyKeyPressed(time, k), ...actions] |> List.iter(dispatch);
    };

    // Run input effects _immediately_
    runEffects();
  };

  let isTextInputActive = () => {
    switch (window) {
    | None => false
    | Some(v) => Revery.Window.isTextInputActive(v)
    };
  };

  switch (window) {
  | None => Log.info("no window to subscribe to events")
  | Some(window) =>
    let _ignore =
      Revery.Event.subscribe(
        window.onKeyDown,
        keyEvent => {
          let isTextInputActive = isTextInputActive();
          Log.info(
            "got keydown - text input:" ++ string_of_bool(isTextInputActive),
          );
          Handler.keyPressToCommand(~isTextInputActive, keyEvent)
          |> keyEventListener;
        },
      );

    let _ignore =
      Revery.Event.subscribe(
        window.onTextInputCommit,
        textEvent => {
          Log.info("onTextInputCommit: " ++ textEvent.text);
          keyEventListener(Some(textEvent.text));
        },
      );
    ();
  };

  stream;
};
