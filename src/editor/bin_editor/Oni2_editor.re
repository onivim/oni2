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
module Log = Core.Log;

let cliOptions = Core.Cli.parse();
Log.debug("Startup: Parsing CLI options complete");

Log.debug("Starting Onivim 2.");

/* The 'main' function for our app */
let init = app => {
  Log.debug("Init");
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
  switch (Sys.chdir(cliOptions.folder)) {
  | exception (Sys_error(msg)) => Log.debug("Folder does not exist: " ++ msg)
  | v => v
  };

  PreflightChecks.run();

  let initialState = Model.State.create();
  let currentState = ref(initialState);

  let update = UI.start(w, <Root state=currentState^ />);

  let onStateChanged = v => {
    currentState := v;
    let state = currentState^;
    GlobalContext.set({...GlobalContext.current(), state});

    update(<Root state />);
  };

  let getScaleFactor = () => {
    Window.getDevicePixelRatio(w) *. float_of_int(Window.getScaleFactor(w));
  };

  Log.debug("Startup: Starting StoreThread");
  let (dispatch, runEffects) =
    Store.StoreThread.start(
      ~setup,
      ~executingDirectory=Core.Utility.executingDirectory,
      ~onStateChanged,
      ~getScaleFactor,
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

  dispatch(Model.Actions.LoadEditorFont("FiraCode-Regular.ttf", 14));
  runEffects();

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
};

/* Let's get this party started! */
Log.debug("Calling App.start");
App.start(init);
