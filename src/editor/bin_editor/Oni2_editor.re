/*
 * Oni2.re
 *
 * This is the entry point for launching the editor.
 */

Printexc.record_backtrace(true);

print_endline ("Top");

print_endline ("1");

module Core = Oni_Core;
module Model = Oni_Model;
module Store = Oni_Store;
module Log = Core.Log;

print_endline ("2");

let cliOptions = Core.Cli.parse();
Log.debug("Startup: Parsing CLI options complete");

Log.debug("Starting Onivim 2.");

/* The 'main' function for our app */
let init = app => {
  open Oni_UI;

  open Revery;
  open Revery.UI;
  open Rench;

  print_endline ("init");
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

  Log.debug("Initializing setup.");
  let setup = Core.Setup.init();
  Log.debug("Startup: Parsing CLI options");

  Log.debug("Startup: Changing folder to: " ++ cliOptions.folder);
  Sys.chdir(cliOptions.folder);

  PreflightChecks.run();

  print_endline ("init done!");
  
};
/*  Log.debug("Init");

  let initialState = Model.State.create();
  let currentState = ref(initialState);

  let update = UI.start(w, <Root state=currentState^ />);

  let onStateChanged = v => {
    currentState := v;
    let state = currentState^;
    GlobalContext.set({...GlobalContext.current(), state});

    update(<Root state />);
  };

  Log.debug("Startup: Starting StoreThread");
  let (dispatch, runEffects) =
    Store.StoreThread.start(
      ~setup,
      ~executingDirectory=Core.Utility.executingDirectory,
      ~onStateChanged,
      (),
    );
  Log.debug("Startup: StoreThread started!");

  GlobalContext.set({
    getState: () => currentState^,
    notifyWindowTreeSizeChanged: (~width, ~height, ()) =>
      dispatch(Model.Actions.WindowTreeSetSize(width, height)),
    notifyEditorSizeChanged: (~editorGroupId, ~width, ~height, ()) =>
      dispatch(
        Model.Actions.EditorGroupSetSize(
          editorGroupId,
          Core.Types.EditorSize.create(
            ~pixelWidth=width,
            ~pixelHeight=height,
            (),
          ),
        ),
      ),
    openEditorById: id => {
      dispatch(Model.Actions.ViewSetActiveEditor(id));
    },
    closeEditorById: id => dispatch(Model.Actions.ViewCloseEditor(id)),
    editorScroll: (~deltaY, ()) =>
      dispatch(Model.Actions.EditorScroll(deltaY)),
    setActiveWindow: (splitId, editorGroupId) =>
      dispatch(Model.Actions.WindowSetActive(splitId, editorGroupId)),
    dispatch,
    state: initialState,
  });

  dispatch(Model.Actions.Init);
  runEffects();

  List.iter(
    v => dispatch(Model.Actions.OpenFileByPath(v, None)),
    cliOptions.filesToOpen,
  );

  let setFont = (fontFamily, fontSize) => {
    let scaleFactor =
      Window.getDevicePixelRatio(w)
      *. float_of_int(Window.getScaleFactor(w));

    let adjSize = int_of_float(float_of_int(fontSize) *. scaleFactor +. 0.5);

    let fontFile = Core.Utility.executingDirectory ++ fontFamily;

    Log.info("Loading font: " ++ fontFile);

    Fontkit.fk_new_face(
      fontFile,
      adjSize,
      font => {
        Log.info("Font loaded!");
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
      _ => Log.error("setFont: Failed to load font " ++ fontFamily),
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
  let keyEventListener = key => {
    switch (key, Focus.focused) {
    | (None, _) => ()
    | (Some((k, true)), {contents: Some(_)})
    | (Some((k, _)), {contents: None}) =>
      inputHandler(~state=currentState^, k) |> List.iter(dispatch)
    | (Some((_, false)), {contents: Some(_)}) => ()
    };
  };

  Event.subscribe(w.onKeyDown, keyEvent =>
    Input.keyPressToCommand(keyEvent, Revery_Core.Environment.os)
    |> keyEventListener
  )
  |> ignore;

  Reglfw.Glfw.glfwSetCharModsCallback(w.glfwWindow, (_w, codepoint, mods) =>
    Input.charToCommand(codepoint, mods) |> keyEventListener
  );
*/

/* Let's get this party started! */
Log.debug("Calling App.start");
Revery.App.start(init);
