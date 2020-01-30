/*
 * Oni2.re
 *
 * This is the entry point for launching the editor.
 */

open Revery;

open Oni_UI;

module Core = Oni_Core;
module Ext = Oni_Extensions;
module Input = Oni_Input;
module Model = Oni_Model;
module Store = Oni_Store;
module ExtM = Oni_ExtensionManagement;
module Log = (val Core.Log.withNamespace("Oni2_editor"));
module ReveryLog = (val Core.Log.withNamespace("Revery"));
module Option = Core.Utility.Option;
module LwtEx = Core.Utility.LwtEx;

let installExtension = (path, cli) => {
  switch (Store.Utility.getUserExtensionsDirectory(cli)) {
  | Some(extensionsFolder) =>
    let result = ExtM.install(~extensionsFolder, ~path) |> LwtEx.sync;

    switch (result) {
    | Ok(_) =>
      Printf.printf("Successfully installed extension: %s\n", path);
      0;

    | Error(_) =>
      Printf.printf("Failed to install extension: %s\n", path);
      1;
    };

  | None =>
    prerr_endline("Error locating user extension folder.");
    1;
  };
};

let uninstallExtension = (_extensionId, _cli) => {
  prerr_endline("Not implemented yet.");
  1;
};

let listExtensions = cli => {
  let extensions = Store.Utility.getUserExtensions(cli);
  let printExtension = (ext: Ext.ExtensionScanner.t) => {
    print_endline(ext.manifest.name);
  };
  List.iter(printExtension, extensions);
  0;
};

let printVersion = _cli => {
  print_endline("Onivim 2 (" ++ Core.BuildInfo.version ++ ")");
  0;
};

let cliOptions =
  Core.Cli.parse(
    ~installExtension,
    ~uninstallExtension,
    ~checkHealth=HealthCheck.run(~checks=All),
    ~listExtensions=
      cli => {
        let extensions = Store.Utility.getUserExtensions(cli);
        let printExtension = (ext: Ext.ExtensionScanner.t) => {
          print_endline(ext.manifest.name);
        };
        List.iter(printExtension, extensions);
        1;
      },
    ~printVersion,
  );
if (cliOptions.syntaxHighlightService) {
  Oni_Syntax_Server.start(~healthCheck=() =>
    HealthCheck.run(~checks=Common, cliOptions)
  );
} else {
  Log.infof(m =>
    m(
      "Starting Onivim 2.%s (%s)",
      Core.BuildInfo.version,
      Core.BuildInfo.commitId,
    )
  );

  /* The 'main' function for our app */
  let init = app => {
    Log.debug("Init");

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

    Log.debug("Initializing setup.");
    let setup = Core.Setup.init();
    Log.debug("Startup: Parsing CLI options");

    Log.info("Startup: Changing folder to: " ++ cliOptions.folder);
    switch (Sys.chdir(cliOptions.folder)) {
    | exception (Sys_error(msg)) =>
      Log.error("Folder does not exist: " ++ msg)
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
      App.quit(~askNicely=false, ~code, app);
    };

    Log.debug("Startup: Starting StoreThread");
    let (dispatch, runEffects) =
      Store.StoreThread.start(
        ~setup,
        ~getClipboardText=() => Sdl2.Clipboard.getText(),
        ~setClipboardText=text => Sdl2.Clipboard.setText(text),
        ~getTime,
        ~executingDirectory=Revery.Environment.executingDirectory,
        ~onStateChanged,
        ~getZoom,
        ~setZoom,
        ~setTitle,
        ~setVsync,
        ~window=Some(w),
        ~cliOptions=Some(cliOptions),
        ~quit,
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
            Core.EditorSize.create(
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
  Log.debug("Calling App.start");
  App.start(init);
};
