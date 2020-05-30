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
module Completions = Feature_LanguageSupport.Completions;

module Log = (val Log.withNamespace("Oni2.Store.Input"));

let start = (window: option(Revery.Window.t), runEffects) => {
  let (stream, dispatch) = Isolinear.Stream.create();

  let immediateDispatchEffect = actions =>
    Isolinear.Effect.createWithDispatch(
      ~name="input.immediateDispatch", dispatch => {
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

  let updateFromInput = (state: State.t, actions) => {
    (state, immediateDispatchEffect(actions));
  };

  let handleTextEffect = (state: State.t, k: string) => {
    switch (Model.FocusManager.current(state)) {
    | Editor
    | Wildmenu => [Actions.KeyboardInput(k)]

    | Quickmenu => [Actions.QuickmenuInput(k)]

    | Sneak => [Actions.Sneak(Feature_Sneak.KeyboardInput(k))]

    | FileExplorer => [
        Actions.FileExplorer(Model.FileExplorer.KeyboardInput(k)),
      ]

    | SCM => [Actions.SCM(Feature_SCM.Msg.keyPressed(k))]

    | Terminal(id) => [
        Actions.Terminal(Feature_Terminal.KeyPressed({id, key: k})),
      ]

    | Search => [Actions.Search(Feature_Search.Input(k))]

    | Modal => [Actions.Modals(Feature_Modals.KeyPressed(k))]
    };
  };

  let effectToActions = (state, effect) =>
    switch (effect) {
    | Keybindings.Execute(command) => [
        Actions.KeybindingInvoked({command: command}),
      ]
    | Keybindings.Text(text) => handleTextEffect(state, text)
    | Keybindings.Unhandled(key) =>
      let isTextInputActive = isTextInputActive();
      let maybeKeyString = Handler.keyPressToCommand(~isTextInputActive, key);
      switch (maybeKeyString) {
      | None => []
      | Some(k) => handleTextEffect(state, k)
      };
    };

  let reveryKeyToEditorKey =
      ({keycode, scancode, keymod, repeat}: Revery.Key.KeyEvent.t) => {
    // TODO: Should we filter out repeat keys from key binding processing?
    ignore(repeat);

    let shift = Revery.Key.Keymod.isShiftDown(keymod);
    let control = Revery.Key.Keymod.isControlDown(keymod);
    let alt = Revery.Key.Keymod.isAltDown(keymod);
    let meta = Revery.Key.Keymod.isGuiDown(keymod);
    let altGr = Revery.Key.Keymod.isAltGrKeyDown(keymod);

    let (altGr, control, alt) =
      switch (Revery.Environment.os) {
      // On Windows, we need to do some special handling here
      // Windows has this funky behavior where pressing AltGr registers as RAlt+LControl down - more info here:
      // https://devblogs.microsoft.com/oldnewthing/?p=40003
      | Revery.Environment.Windows =>
        let altGr =
          altGr
          || Revery.Key.Keymod.isRightAltDown(keymod)
          && Revery.Key.Keymod.isControlDown(keymod);
        // If altGr is active, disregard control / alt key
        let ctrlKey = altGr ? false : control;
        let altKey = altGr ? false : alt;
        (altGr, ctrlKey, altKey);
      | _ => (altGr, control, alt)
      };

    EditorInput.KeyPress.{
      scancode,
      keycode,
      modifiers: {
        shift,
        control,
        alt,
        meta,
        altGr,
      },
    };
  };

  let keyCodeToString = Sdl2.Keycode.getName;

  let keyPressToString = EditorInput.KeyPress.toString(~keyCodeToString);

  /**
     The key handlers return (keyPressedString, shouldOniListen)
     i.e. if ctrl or alt or cmd were pressed then Oni2 should listen
     /respond to commands otherwise if input is alphabetical AND
     a revery element is focused oni2 should defer to revery
   */
  let handleKeyPress = (state: State.t, key) => {
    let context =
      WhenExpr.ContextKeys.fromSchema(Model.ContextKeys.all, state);

    let (keyBindings, effects) =
      Keybindings.keyDown(~context, ~key, state.keyBindings);

    let newState = {...state, keyBindings};

    let actions =
      effects |> List.map(effectToActions(state)) |> List.flatten;

    updateFromInput(newState, /*Some(keyPressToString(key)),*/ actions);
  };

  let handleTextInput = (state: State.t, text) => {
    let (keyBindings, effects) = Keybindings.text(~text, state.keyBindings);

    let actions =
      effects |> List.map(effectToActions(state)) |> List.flatten;

    let newState = {...state, keyBindings};

    updateFromInput(newState, /*Some("Text: " ++ text),*/ actions);
  };

  let handleKeyUp = (state: State.t, key) => {
    let context =
      WhenExpr.ContextKeys.fromSchema(Model.ContextKeys.all, state);

    //let inputKey = reveryKeyToEditorKey(key);
    let (keyBindings, effects) =
      Keybindings.keyUp(~context, ~key, state.keyBindings);

    let newState = {...state, keyBindings};

    let actions =
      effects |> List.map(effectToActions(state)) |> List.flatten;

    updateFromInput(newState, /*None, */ actions);
  };

  // TODO: This should be moved to a Feature_Keybindings project
  // (Including the KeyDisplayer too!)
  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | KeyDown(event, time) =>
      let keyDisplayer =
        state.keyDisplayer
        |> Option.map(keyDisplayer =>
             Oni_Components.KeyDisplayer.keyPress(
               ~time=Revery.Time.toFloatSeconds(time),
               keyPressToString(event),
               keyDisplayer,
             )
           );

      handleKeyPress({...state, keyDisplayer}, event);
    | KeyUp(event, _time) => handleKeyUp(state, event)
    | TextInput(text, time) =>
      let keyDisplayer =
        state.keyDisplayer
        |> Option.map(keyDisplayer =>
             Oni_Components.KeyDisplayer.textInput(
               ~time=Revery.Time.toFloatSeconds(time),
               text,
               keyDisplayer,
             )
           );

      handleTextInput({...state, keyDisplayer}, text);

    | _ => (state, Isolinear.Effect.none)
    };
  };

  //  SUBSCRIPTIONS

  switch (window) {
  | None => Log.error("No window to subscribe to events")
  | Some(window) =>
    let _: unit => unit =
      Revery.Window.onKeyDown(
        window,
        event => {
          let time = Revery.Time.now();
          dispatch(Actions.KeyDown(event |> reveryKeyToEditorKey, time));
        },
      );

    let _: unit => unit =
      Revery.Window.onKeyUp(
        window,
        event => {
          let time = Revery.Time.now();
          dispatch(Actions.KeyUp(event |> reveryKeyToEditorKey, time));
        },
      );

    let _: unit => unit =
      Revery.Window.onTextInputCommit(
        window,
        event => {
          let time = Revery.Time.now();
          dispatch(Actions.TextInput(event.text, time));
        },
      );
    ();
  };

  (updater, stream);
};
