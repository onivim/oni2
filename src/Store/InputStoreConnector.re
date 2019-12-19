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

module Log = (val Log.withNamespace("Oni2.InputStore"));

module Option = Utility.Option;
module List = Utility.List;

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

  let immediateDispatchEffect = actions =>
    Isolinear.Effect.createWithDispatch(~name="input.immediateDispatch", dispatch => {
      actions |> List.iter(dispatch);

      // Run input effects _immediately_
      runEffects();
    });


  // We also use 'text input' mode for SDL2.
  // This enables us to get resolved keyboard events, and IME.

  // For IME: Is this sufficient? Or will we need a way to turn off / toggle IME when switching modes?
  Sdl2.TextInput.start();

  let isTextInputActive = () => {
    switch (window) {
    | None => false
    | Some(v) => Revery.Window.isTextInputActive(v)
    };
  };

  let getKeyUpBindings = {
    let containsCtrl = Re.execp(Re.compile(Re.str("C-")));
    let containsShift = Re.execp(Re.compile(Re.str("S-")));
    let containsAlt = Re.execp(Re.compile(Re.str("A-")));

    // NOTE: THis currently only generates a single command based on an ordinary
    // keybinding in order to emulate vscode's behaviour in the editors picker
    List.filter_map((binding: Keybindings.Keybinding.t) =>
      switch (binding.command) {
      | "workbench.action.openNextRecentlyUsedEditorInGroup" =>
        let createBinding = key =>
          Keybindings.Keybinding.{
            key,
            command: "list.select",
            condition: Variable("inEditorsPicker"),
          };

        if (containsCtrl(binding.key)) {
          Some(createBinding("<C>"));
        } else if (containsShift(binding.key)) {
          Some(createBinding("<S>"));
        } else if (containsAlt(binding.key)) {
          Some(createBinding("<A>"));
        } else {
          None;
        };

      | _ => None
      }
    );
  };

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
  let handleKeyPress = (state, key) => {
    let state = getState();
    let bindings = state.keyBindings;
    let conditions = conditionsOfState(state);
    let time = Revery.Time.now() |> Revery.Time.toFloatSeconds;

    let captureMode =
      switch (state.quickmenu) {
      | Some({variant: Wildmenu(_), _}) => Wildmenu

      | Some({variant: DocumentSymbols, _})
      | Some({variant: CommandPalette, _})
      | Some({variant: EditorsPicker, _})
      | Some({variant: FilesPicker, _})
      | Some({variant: ThemesPicker, _}) => Quickmenu

      | None => Normal
      };

    switch (key) {
    | Some(k) =>
      let bindingActions = getActionsForBinding(k, bindings, conditions);

      let actions =
        switch (captureMode) {
        | Normal
        | Wildmenu
            when bindingActions == [] && Revery.UI.Focus.focused^ == None =>
          [Actions.KeyboardInput(k)];

        | _ =>
          bindingActions;
        };

      (state, immediateDispatchEffect([Actions.NotifyKeyPressed(time, k), ...actions]));

    | None =>
      (state, Isolinear.Effect.none)
    };
  };

  let handleKeyUp = (state, event: Revery.Key.KeyEvent.t) => {
    let state = getState();
    let bindings = state.keyBindings;
    let conditions = conditionsOfState(state);

    // NOTE: This curretly only handles Ctrl, Shift and Alt. Everything else is ignored
    let maybeKeyString =
      switch (event.keycode) {
      | 1073742048 => Some("<C>")
      | 1073742049 => Some("<S>")
      | 1073742050 => Some("<A>")
      | _ => None
      };

    switch (maybeKeyString) {
    | Some(keyString) =>
      let bindings = getKeyUpBindings(bindings);
      let actions = getActionsForBinding(keyString, bindings, conditions);

      (state, immediateDispatchEffect(actions));

    | None =>
      (state, Isolinear.Effect.none)
    };
  };

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | KeyDown(event) => 
      let isTextInputActive = isTextInputActive();
      event
      |> Handler.keyPressToCommand(~isTextInputActive)
      |> handleKeyPress(state);

    | KeyUp(event) => 
      handleKeyUp(state, event);

    | TextInput(event) => 
      handleKeyPress(state, Some(event.text));

    | _ => (state, Isolinear.Effect.none)
    };
  };

  //  SUBSCRIPTIONS

  switch (window) {
  | None => Log.error("no window to subscribe to events")
  | Some(window) =>
    let _: unit => unit =
      Revery.Event.subscribe(
        window.onKeyDown,
        event => dispatch(Actions.KeyDown(event)),
      );

    let _: unit => unit =
      Revery.Event.subscribe(
        window.onKeyUp,
        event => dispatch(Actions.KeyUp(event)),
      );

    let _: unit => unit =
      Revery.Event.subscribe(
        window.onTextInputCommit,
        event => dispatch(Actions.TextInput(event)),
      );
    ();
  };

  (updater, stream);
};
