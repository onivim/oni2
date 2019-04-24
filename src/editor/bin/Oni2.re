/*
 * Oni2.re
 *
 * This is the entry point for launching the editor.
 */

open Revery;
open Revery.UI;

open Rench;

open Oni_UI;

module Core = Oni_Core;
module Model = Oni_Model;
module Store = Oni_Store;

/**
   This allows a stack trace to be printed when exceptions occur
 */
switch (Sys.getenv_opt("ONI2_DEBUG")) {
| Some(_) => Printexc.record_backtrace(true) |> ignore
| None => ()
};

/* The 'main' function for our app */
let init = app => {
  let w =
    App.createWindow(
      ~createOptions=
        WindowCreateOptions.create(
          ~maximized=false,
          ~icon=Some("logo.png"),
          (),
        ),
      app,
      "Oni2",
    );

  let setup = Core.Setup.init();
  let cliOptions = Core.Cli.parse(setup);
  Sys.chdir(cliOptions.folder);

  PreflightChecks.run();

  let initialState = Model.State.create();
  let currentState = ref(Model.State.create());

  let update = UI.start(w, <Root state=currentState^ />);

  let onStateChanged = v => {
    currentState := v;
    let state = currentState^;
    GlobalContext.set({...GlobalContext.current(), state});

    update(<Root state />);
  };

  let dispatch =
    Store.StoreThread.start(
      ~cliOptions,
      ~setup,
      ~executingDirectory=Revery.Environment.getExecutingDirectory(),
      ~onStateChanged,
      (),
    );

  GlobalContext.set({
    getState: () => currentState^,
    notifySizeChanged: (~width, ~height, ()) =>
      dispatch(
        Model.Actions.SetEditorSize(
          Core.Types.EditorSize.create(
            ~pixelWidth=width,
            ~pixelHeight=height,
            (),
          ),
        ),
      ),
    openEditorById: id => dispatch(Model.Actions.OpenFileById(id)),
    closeEditorById: id => dispatch(Model.Actions.ViewCloseEditor(id)),
    editorScroll: (~deltaY, ()) =>
      dispatch(Model.Actions.EditorScroll(deltaY)),
    dispatch,
    state: initialState,
  });

  dispatch(Model.Actions.Init);

  let setFont = (fontFamily, fontSize) => {
    let scaleFactor =
      Window.getDevicePixelRatio(w)
      *. float_of_int(Window.getScaleFactor(w));

    let adjSize = int_of_float(float_of_int(fontSize) *. scaleFactor +. 0.5);

    Fontkit.fk_new_face(
      Revery.Environment.getExecutingDirectory() ++ fontFamily,
      adjSize,
      font => {
        open Oni_Model.Actions;
        open Oni_Core.Types;

        /* Measure text */
        let shapedText = Fontkit.fk_shape(font, "H");
        let firstShape = shapedText[0];
        let glyph = Fontkit.renderGlyph(font, firstShape.glyphId);

        let metrics = Fontkit.fk_get_metrics(font);
        let actualHeight =
          float_of_int(fontSize)
          *. float_of_int(metrics.height)
          /. float_of_int(metrics.unitsPerEm);

        /* Set editor text based on measurements */
        dispatch(
          SetEditorFont(
            EditorFont.create(
              ~fontFile=fontFamily,
              ~fontSize,
              ~measuredWidth=
                float_of_int(glyph.advance) /. (64. *. scaleFactor),
              ~measuredHeight=floor(actualHeight +. 0.5),
              (),
            ),
          ),
        );
      },
      _ => prerr_endline("setFont: Failed to load font " ++ fontFamily),
    );
  };

  setFont("FiraCode-Regular.ttf", 14);

  let commands = Core.Keybindings.get();

  /* Add an updater to handle a KeyboardInput action */
  let inputHandler = Input.handle(~commands);

  /**
     The key handlers return (keyPressedString, shouldOniListen)
     i.e. if ctrl or alt or cmd were pressed then Oni2 should listen
     /respond to commands otherwise if input is alphabetical AND
     a revery element is focused oni2 should defer to revery
   */
  let keyEventListener = key =>
    switch (key, Focus.focused) {
    | (None, _) => ()
    | (Some((k, true)), {contents: Some(_)})
    | (Some((k, _)), {contents: None}) =>
      inputHandler(~state=currentState^, k) |> List.iter(dispatch)
    | (Some((_, false)), {contents: Some(_)}) => ()
    };

  Event.subscribe(w.onKeyDown, keyEvent =>
    Input.keyPressToCommand(keyEvent) |> keyEventListener
  )
  |> ignore;

  Reglfw.Glfw.glfwSetCharModsCallback(w.glfwWindow, (_w, codepoint, mods) =>
    Input.charToCommand(codepoint, mods) |> keyEventListener
  );
};

/* Let's get this party started! */
App.start(init);
