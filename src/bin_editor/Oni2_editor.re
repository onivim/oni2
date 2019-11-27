/*
 * Oni2.re
 *
 * This is the entry point for launching the editor.
 */

open Revery;

open Oni_UI;

module Core = Oni_Core;
module Input = Oni_Input;
module Model = Oni_Model;
module Store = Oni_Store;
module Log = Core.Log;

let cliOptions = Core.Cli.parse();
Log.info("Startup: Parsing CLI options complete");

Log.info("Starting Onivim 2.");

/* The 'main' function for our app */
let init = app => {
  Log.info("Init");

  let _ = Revery.Log.listen((_, msg) => Log.info("[Revery]: " ++ msg));

  let w =
    App.createWindow(
      ~createOptions=
        WindowCreateOptions.create(
          ~forceScaleFactor=cliOptions.forceScaleFactor,
          ~maximized=false,
          ~vsync=Vsync.Immediate,
          ~icon=Some("logo.png"),
          (),
        ),
      app,
      "Oni2",
    );

  Log.info("Initializing setup.");
  let setup = Core.Setup.init();
  Log.info("Startup: Parsing CLI options");

  Log.info("Startup: Changing folder to: " ++ cliOptions.folder);
  switch (Sys.chdir(cliOptions.folder)) {
  | exception (Sys_error(msg)) => Log.error("Folder does not exist: " ++ msg)
  | v => v
  };

  PreflightChecks.run();

  let initialState = Model.State.create();
  let currentState = ref(initialState);

  let update = UI.start(w, <Root state=currentState^ />);

  let isDirty = ref(false);
  let onStateChanged = state => {
    currentState := state;
    isDirty := true;
  };

  let _ =
    Tick.interval(
      _dt =>
        if (isDirty^) {
          let state = currentState^;
          GlobalContext.set({...GlobalContext.current(), state});
          update(<Root state />);
          isDirty := false;
        },
      Time.seconds(0),
    );

  let getScaleFactor = () => {
    Window.getDevicePixelRatio(w) *. Window.getScaleAndZoom(w);
  };

  let getTime = () => Time.now() |> Time.toFloatSeconds;

  let getZoom = () => {
    Window.getZoom(w);
  };

  let setZoom = zoomFactor => Window.setZoom(w, zoomFactor);

  let setTitle = title => {
    Window.setTitle(w, title);
  };

  let setVsync = vsync => Window.setVsync(w, vsync);

  let quit = code => {
    App.quit(~code, app);
  };

  Log.info("Startup: Starting StoreThread");
  let (dispatch, runEffects) =
    Store.StoreThread.start(
      ~setup,
      ~getClipboardText=() => Sdl2.Clipboard.getText(),
      ~setClipboardText=text => Sdl2.Clipboard.setText(text),
      ~getTime,
      ~executingDirectory=Core.Utility.executingDirectory,
      ~onStateChanged,
      ~getScaleFactor,
      ~getZoom,
      ~setZoom,
      ~setTitle,
      ~setVsync,
      ~window=Some(w),
      ~cliOptions=Some(cliOptions),
      ~quit,
      (),
    );
  Log.info("Startup: StoreThread started!");

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
    editorScrollDelta: (~editorId, ~deltaY, ()) =>
      dispatch(Model.Actions.EditorScroll(editorId, deltaY)),
    editorSetScroll: (~editorId, ~scrollY, ()) =>
      dispatch(Model.Actions.EditorSetScroll(editorId, scrollY)),
    setActiveWindow: (splitId, editorGroupId) =>
      dispatch(Model.Actions.WindowSetActive(splitId, editorGroupId)),
    hideNotification: id => dispatch(Model.Actions.HideNotification(id)),
    dispatch,
    state: initialState,
  });

  dispatch(Model.Actions.Init);
  runEffects();

  List.iter(
    v => dispatch(Model.Actions.OpenFileByPath(v, None, None)),
    cliOptions.filesToOpen,
  );
};

/* Let's get this party started! */
Log.info("Calling App.start");
App.start(init);
