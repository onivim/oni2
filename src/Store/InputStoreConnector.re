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
    | Feature_Input.(Execute(NamedCommand(command))) => [
        Actions.KeybindingInvoked({command: command}),
      ]
    | Feature_Input.Text(text) => handleTextEffect(~isText=true, state, text)
    | Feature_Input.Unhandled(key) =>
      let isTextInputActive = isTextInputActive();
      let maybeKeyString = Handler.keyPressToCommand(~isTextInputActive, key);
      switch (maybeKeyString) {
      | None => []
      | Some(k) => handleTextEffect(~isText=false, state, k)
      };

    // TODO: Show a notification that recursion limit was hit
    | Feature_Input.RemapRecursionLimitHit => []
    };

  let reveryKeyToEditorKey =
      ({scancode, keymod, repeat, _}: Revery.Key.KeyEvent.t) => {
    // TODO: Should we filter out repeat keys from key binding processing?
    ignore(repeat);

    let name = Sdl2.Scancode.ofInt(scancode) |> Sdl2.Scancode.getName;

    if (name == "Left Shift" || name == "Right Shift") {
      None;
    } else {
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
      let keyboard = Oni2_KeyboardLayout.Keymap.getCurrent();
      let maybeKeymap =
        Oni2_KeyboardLayout.Keymap.entryOfScancode(keyboard, scancode);

      switch (maybeKeymap) {
      | Some(keymap) =>
        Log.infof(m =>
          m(
            "Key info: %s\n",
            Oni2_KeyboardLayout.Keymap.entryToString(keymap),
          )
        )
      | None => Log.info("No keymap for key.")
      };

      let stringToKey = maybeString => {
        maybeString
        |> OptionEx.flatMap(str =>
             if (String.length(str) != 1) {
               None;
             } else {
               Some(str.[0]);
             }
           );
      };

      let maybeKey =
        maybeKeymap
        |> OptionEx.tap(keymap => {
             Log.infof(m =>
               m(
                 "Key info: %s\n",
                 Oni2_KeyboardLayout.Keymap.entryToString(keymap),
               )
             )
           })
        |> OptionEx.tapNone(() => Log.info("No keymap for key"))
        |> OptionEx.flatMap((keymap: Oni2_KeyboardLayout.Keymap.entry) =>
             if (shift && altGr) {
               stringToKey(keymap.withAltGraphShift);
             } else if (shift) {
               stringToKey(keymap.withShift);
             } else if (altGr) {
               stringToKey(keymap.withAltGraph);
             } else {
               stringToKey(keymap.unmodified);
             }
           )
        |> Option.map(keyChar => EditorInput.Key.Character(keyChar));

      maybeKey
      |> Option.map(key => {
           EditorInput.KeyPress.physicalKey(
             ~key,
             ~modifiers={shift, control, alt, meta, altGr},
           )
         });
    };
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

    updateFromInput(newState, /*Some("Text: " ++ text),*/ actions);
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
    // TODO hardcoded scancode:
    | KeyDown(event, time) =>
      handleKeyPress(~scancode=101, state, time, event)

    // TODO - hardcoded scancode:
    | KeyUp(_event, _time) => handleKeyUp(~scancode=101, state)

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
          |> Option.iter(key => {dispatch(Actions.KeyDown(key, time))});
        },
      );

    let _: unit => unit =
      Revery.Window.onKeyUp(
        window,
        event => {
          let time = Revery.Time.now();
          event
          |> reveryKeyToEditorKey
          |> Option.iter(key => {dispatch(Actions.KeyUp(key, time))});
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
