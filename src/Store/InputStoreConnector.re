/*
 * InputStoreConnector.re
 *
 * This module connects external user input to the store.
 */

open Oni_Core;
open Utility;

open Oni_Input;

module Model = Oni_Model;
module State = Model.State;
module Actions = Model.Actions;

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

  let handleTextEffect = (~isText, state: State.t, k: string) => {
    switch (Model.FocusManager.current(state)) {
    | Editor
    | Wildmenu => [
        Actions.KeyboardInput({isText, input: k}),
        Actions.LanguageSupport(
          Feature_LanguageSupport.Msg.Hover.keyPressed(k),
        ),
      ]

    | Quickmenu => [Actions.QuickmenuInput(k)]

    | NewQuickmenu => [
        Actions.Quickmenu(Feature_Quickmenu.Msg.keyPressed(k)),
      ]

    | Sneak => [Actions.Sneak(Feature_Sneak.KeyboardInput(k))]

    | FileExplorer => [
        Actions.FileExplorer(Feature_Explorer.Msg.keyPressed(k)),
      ]

    | Pane => [Actions.Pane(Feature_Pane.Msg.keyPressed(k))]

    | SCM => [Actions.SCM(Feature_SCM.Msg.keyPressed(k))]

    | Terminal(id) => [
        Actions.Terminal(Feature_Terminal.KeyPressed({id, key: k})),
      ]

    | Search => [Actions.Search(Feature_Search.Msg.input(k))]
    | Extensions => [
        Actions.Extensions(Feature_Extensions.Msg.keyPressed(k)),
      ]

    | Modal => [Actions.Modals(Feature_Modals.KeyPressed(k))]
    | InsertRegister => [
        Actions.Registers(Feature_Registers.Msg.keyPressed(k)),
      ]
    | LicenseKey => [
        Actions.Registration(Feature_Registration.KeyPressed(k)),
      ]
    | LanguageSupport => [
        Actions.LanguageSupport(Feature_LanguageSupport.Msg.keyPressed(k)),
      ]
    };
  };

  let pasteEffect = (~rawText, ~isMultiLine as _, ~lines, state) =>
    if (Array.length(lines) >= 1) {
      let firstLine = lines[0];
      let action =
        switch (Model.FocusManager.current(state)) {
        | Editor => Actions.Vim(Feature_Vim.Pasted(rawText))
        | Wildmenu => Actions.Vim(Feature_Vim.Pasted(firstLine))
        | Quickmenu => Actions.QuickmenuPaste(firstLine)
        | NewQuickmenu =>
          Actions.Quickmenu(Feature_Quickmenu.Msg.pasted(firstLine))
        | Extensions =>
          Actions.Extensions(Feature_Extensions.Msg.pasted(firstLine))
        | SCM => Actions.SCM(Feature_SCM.Msg.paste(firstLine))
        | Search => Actions.Search(Feature_Search.Msg.pasted(firstLine))
        | LanguageSupport =>
          Actions.LanguageSupport(
            Feature_LanguageSupport.Msg.pasted(firstLine),
          )
        | LicenseKey =>
          Actions.Registration(Feature_Registration.Pasted(firstLine))
        | Terminal(id) =>
          Actions.Terminal(Feature_Terminal.Pasted({id, text: rawText}))

        // No paste handling in these UIs, currently...
        | Pane => Actions.Noop
        | InsertRegister
        | Sneak
        | FileExplorer
        | Modal => Actions.Noop
        };

      Isolinear.Effect.createWithDispatch(~name="input.pasteEffect", dispatch => {
        dispatch(action)
      });
    } else {
      Isolinear.Effect.none;
    };

  let effectToActions = (state, effect) =>
    switch (effect) {
    | Feature_Input.(Execute(VimExCommand(command))) => [
        Actions.VimExecuteCommand({allowAnimation: true, command}),
      ]
    | Feature_Input.(Execute(NamedCommand({command, arguments}))) => [
        Actions.KeybindingInvoked({command, arguments}),
      ]
    | Feature_Input.Text(text) => handleTextEffect(~isText=true, state, text)
    | Feature_Input.Unhandled({key, isProducedByRemap}) =>
      let isTextInputActive = isTextInputActive();

      let maybeKeyString =
        key
        |> EditorInput.KeyCandidate.toList
        |> (
          list =>
            List.nth_opt(list, 0)
            |> OptionEx.flatMap(
                 Handler.keyPressToCommand(
                   ~force=isProducedByRemap,
                   ~isTextInputActive,
                 ),
               )
        );
      switch (maybeKeyString) {
      | None => []
      | Some(k) => handleTextEffect(~isText=false, state, k)
      };

    // TODO: Show a notification that recursion limit was hit
    | Feature_Input.RemapRecursionLimitHit => []
    };

  let reveryKeyToEditorKey = keyEvent => {
    keyEvent |> Feature_Input.ReveryKeyConverter.reveryKeyToKeyPress;
  };

  /**
     The key handlers return (keyPressedString, shouldOniListen)
     i.e. if ctrl or alt or cmd were pressed then Oni2 should listen
     /respond to commands otherwise if input is alphabetical AND
     a revery element is focused oni2 should defer to revery
   */
  let handleKeyPress = (~scancode, state: State.t, time, key) => {
    let context = Model.ContextKeys.all(state);

    let config = Model.Selectors.configResolver(state);
    let (input, effects) =
      Feature_Input.keyDown(
        ~config,
        ~context,
        ~scancode,
        ~key,
        ~time,
        state.input,
      );

    let newState = {...state, input};

    let actions =
      effects |> List.map(effectToActions(state)) |> List.flatten;

    updateFromInput(newState, /*Some(keyPressToString(key)),*/ actions);
  };

  let handleTextInput = (state: State.t, time, text) => {
    let (input, effects) = Feature_Input.text(~text, ~time, state.input);

    let actions =
      effects |> List.map(effectToActions(state)) |> List.flatten;

    let newState = {...state, input};

    updateFromInput(newState, actions);
  };

  let handleTimeout = (state: State.t) => {
    let context = Model.ContextKeys.all(state);
    let (input, effects) = Feature_Input.timeout(~context, state.input);

    let actions =
      effects |> List.map(effectToActions(state)) |> List.flatten;

    let newState = {...state, input};

    updateFromInput(newState, actions);
  };

  let handleKeyUp = (~scancode, state: State.t) => {
    let context = Model.ContextKeys.all(state);

    let config = Model.Selectors.configResolver(state);
    let (input, effects) =
      Feature_Input.keyUp(~config, ~scancode, ~context, state.input);

    let newState = {...state, input};

    let actions =
      effects |> List.map(effectToActions(state)) |> List.flatten;

    updateFromInput(newState, /*None, */ actions);
  };

  // TODO: This should be moved to a Feature_Keybindings project
  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | KeyDown({key, scancode, time}) =>
      handleKeyPress(~scancode, state, time, key)

    | KeyUp({scancode, _}) => handleKeyUp(~scancode, state)

    | KeyTimeout => handleTimeout(state)

    | TextInput(text, time) => handleTextInput(state, time, text)

    | Pasted({rawText, isMultiLine, lines}) => (
        state,
        pasteEffect(~rawText, ~isMultiLine, ~lines, state),
      )

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
          event
          |> reveryKeyToEditorKey
          |> Option.iter(key => {
               dispatch(
                 Actions.KeyDown({key, time, scancode: event.scancode}),
               )
             });
        },
      );

    let _: unit => unit =
      Revery.Window.onKeyUp(
        window,
        event => {
          let time = Revery.Time.now();
          dispatch(Actions.KeyUp({time, scancode: event.scancode}));
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
