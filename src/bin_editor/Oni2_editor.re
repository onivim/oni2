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

let cliOptions =
  Core.Cli.parse(
    ~installExtension,
    ~uninstallExtension,
    ~checkHealth=HealthCheck.run(~checks=All),
    ~listExtensions=
      cli => {
        let extensions = Store.Utility.getUserExtensions(cli);
        let printExtension = (ext: Exthost.Extension.Scanner.ScanResult.t) => {
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

    let window =
      App.createWindow(
        ~createOptions=
          WindowCreateOptions.create(
            ~forceScaleFactor=cliOptions.forceScaleFactor,
            ~maximized=false,
            ~vsync=Vsync.Immediate,
            ~icon=Some("logo.png"),
            ~titlebarStyle=WindowStyles.Transparent,
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

    Revery.Window.setBackgroundColor(window, Colors.black);

    PreflightChecks.run();

    let getUserSettings = Feature_Configuration.UserSettingsProvider.getSettings;

    let currentState =
      ref(
        Model.State.initial(
          ~getUserSettings,
          ~contributedCommands=[] // TODO
        ),
      );

    let persistGlobal = () =>
      Store.Persistence.(persistIfDirty(Global.store, currentState^));
    let persistWorkspace = () =>
      switch (currentState^) {
      | {workspace: Some({workingDirectory, _}), _} as state =>
        Store.Persistence.(
          persistIfDirty(
            Workspace.storeFor(workingDirectory),
            (state, window),
          )
        )
      | _ => ()
      };

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
        ~cliOptions=Some(cliOptions),
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
