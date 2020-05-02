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
module LwtEx = Core.Utility.LwtEx;

let installExtension = (path, Cli.{overriddenExtensionsDir, _}) => {
  switch (Store.Utility.getUserExtensionsDirectory(~overriddenExtensionsDir)) {
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

let listExtensions = (Cli.{overriddenExtensionsDir, _}) => {
  let extensions = Store.Utility.getUserExtensions(~overriddenExtensionsDir);
  let printExtension = (ext: Exthost.Extension.Scanner.ScanResult.t) => {
    print_endline(ext.manifest.name);
  };
  List.iter(printExtension, extensions);
  0;
};

let printVersion = _cli => {
  print_endline("Onivim 2 (" ++ Core.BuildInfo.version ++ ")");
  0;
};

Log.debug("Startup: Parsing CLI options");
let cliOptions =
  Cli.parse(
    ~installExtension,
    ~uninstallExtension,
    ~checkHealth=HealthCheck.run(~checks=All),
    ~listExtensions=
      ({overriddenExtensionsDir, _}) => {
        let extensions =
          Store.Utility.getUserExtensions(~overriddenExtensionsDir);
        let printExtension = (ext: Exthost.Extension.Scanner.ScanResult.t) => {
          print_endline(ext.manifest.name);
        };
        List.iter(printExtension, extensions);
        1;
      },
    ~printVersion,
  );

let initWorkingDirectory = () => {
  let path =
    switch (cliOptions.folder) {
    | Some(folder) => folder
    | None =>
      switch (Store.Persistence.Global.(get(workspace))) {
      | Some(path) => path
      | None =>
        Dir.User.document |> Option.value(~default=Dir.home) |> Fp.toString
      }
    };

  Log.info("Startup: Changing folder to: " ++ path);
  try(Sys.chdir(path)) {
  | Sys_error(msg) => Log.error("Folder does not exist: " ++ msg)
  };

  path;
};

let createWindow = (~forceScaleFactor, ~workingDirectory, app) => {
  let (x, y, width, height, maximized) = {
    open Store.Persistence.Workspace;
    let store = storeFor(workingDirectory);

    (
      get(windowX, store)
      |> Option.fold(~some=x => `Absolute(x), ~none=`Centered),
      get(windowY, store)
      |> Option.fold(~some=y => `Absolute(y), ~none=`Centered),
      get(windowWidth, store),
      get(windowHeight, store),
      get(windowMaximized, store),
    );
  };

  let window =
    App.createWindow(
      ~createOptions=
        WindowCreateOptions.create(
          ~forceScaleFactor,
          ~maximized,
          ~vsync=Vsync.Immediate,
          ~icon=Some("logo.png"),
          ~titlebarStyle=WindowStyles.Transparent,
          ~x,
          ~y,
          ~width,
          ~height,
          (),
        ),
      app,
      "Oni2",
    );

  Window.setBackgroundColor(window, Colors.black);

  window;
};

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

    let initialWorkingDirectory = initWorkingDirectory();
    let window =
      createWindow(
        ~forceScaleFactor=cliOptions.forceScaleFactor,
        ~workingDirectory=initialWorkingDirectory,
        app,
      );

    Log.debug("Initializing setup.");
    let setup = Core.Setup.init();

    PreflightChecks.run();

    let getUserSettings = Feature_Configuration.UserSettingsProvider.getSettings;

    let currentState =
      ref(
        Model.State.initial(
          ~getUserSettings,
          ~contributedCommands=[], // TODO
          ~workingDirectory=initialWorkingDirectory,
        ),
      );

    let persistGlobal = () =>
      Store.Persistence.Global.persistIfDirty(currentState^);
    let persistWorkspace = () =>
      Store.Persistence.Workspace.(
        persistIfDirty(
          storeFor(currentState^.workspace.workingDirectory),
          (currentState^, window),
        )
      );

    let update = UI.start(window, <Root state=currentState^ />);

    let isDirty = ref(false);
    let onStateChanged = state => {
      currentState := state;
      isDirty := true;
    };

    let runEventLoop = () => {
      // TODO: How many times should we run it?
      // The ideal amount would be just enough to do pending work,
      // but not too much to just spin. Unfortunately, it seems
      // Luv.Loop.run always returns [true] for us, so we don't
      // have a reliable way to know we're done (at the moment).
      for (_ in 1 to 100) {
        ignore(Luv.Loop.run(~mode=`NOWAIT, ()): bool);
      };
    };

    let tick = _dt => {
      runEventLoop();

      if (isDirty^) {
        update(<Root state=currentState^ />);
        isDirty := false;
        persistGlobal();
      };
    };
    let _: unit => unit = Tick.interval(tick, Time.zero);

    let getZoom = () => {
      Window.getZoom(window);
    };

    let setZoom = zoomFactor => Window.setZoom(window, zoomFactor);

    let setTitle = title => {
      Window.setTitle(window, title);
    };

    let setVsync = vsync => Window.setVsync(window, vsync);

    let quit = code => {
      App.quit(~askNicely=false, ~code, app);
    };

    Log.debug("Startup: Starting StoreThread");
    let (dispatch, runEffects) =
      Store.StoreThread.start(
        ~getUserSettings,
        ~setup,
        ~getClipboardText=() => Sdl2.Clipboard.getText(),
        ~setClipboardText=text => Sdl2.Clipboard.setText(text),
        ~executingDirectory=Revery.Environment.executingDirectory,
        ~getState=() => currentState^,
        ~onStateChanged,
        ~getZoom,
        ~setZoom,
        ~setTitle,
        ~setVsync,
        ~window=Some(window),
        ~filesToOpen=cliOptions.filesToOpen,
        ~shouldLoadExtensions=cliOptions.shouldLoadConfiguration,
        ~shouldSyntaxHighlight=cliOptions.shouldSyntaxHighlight,
        ~shouldLoadConfiguration=cliOptions.shouldLoadConfiguration,
        ~quit,
        (),
      );
    Log.debug("Startup: StoreThread started!");

    let _: Window.unsubscribe =
      Window.onMaximized(window, () =>
        dispatch(Model.Actions.WindowMaximized)
      );
    let _: Window.unsubscribe =
      Window.onMinimized(window, () =>
        dispatch(Model.Actions.WindowMinimized)
      );
    let _: Window.unsubscribe =
      Window.onRestored(window, () => dispatch(Model.Actions.WindowRestored));
    let _: Window.unsubscribe =
      Window.onFocusGained(window, () =>
        dispatch(Model.Actions.WindowFocusGained)
      );
    let _: Window.unsubscribe =
      Window.onFocusLost(window, () =>
        dispatch(Model.Actions.WindowFocusLost)
      );
    let _: Window.unsubscribe =
      Window.onSizeChanged(window, _ => persistWorkspace());
    let _: Window.unsubscribe =
      Window.onMoved(window, _ => persistWorkspace());

    GlobalContext.set({
      openEditorById: id => {
        dispatch(Model.Actions.ViewSetActiveEditor(id));
      },
      closeEditorById: id => dispatch(Model.Actions.ViewCloseEditor(id)),
      editorScrollDelta: (~editorId, ~deltaY, ()) =>
        dispatch(Model.Actions.EditorScroll(editorId, deltaY)),
      editorSetScroll: (~editorId, ~scrollY, ()) =>
        dispatch(Model.Actions.EditorSetScroll(editorId, scrollY)),
      dispatch,
    });

    dispatch(Model.Actions.Init);
    runEffects();

    // Add a quit handler, so that regardless of how we quit -
    // we have the opportunity to clean up
    Revery.App.onBeforeQuit(app, () =>
      if (!currentState^.isQuitting) {
        dispatch(Model.Actions.Quit(true));
      }
    )
    |> (ignore: Revery.App.unsubscribe => unit);

    List.iter(
      v => dispatch(Model.Actions.OpenFileByPath(v, None, None)),
      cliOptions.filesToOpen,
    );
  };

  /* Let's get this party started! */
  Log.debug("Calling App.start");
  App.start(init);
};
