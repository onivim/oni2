/*
 * Oni2.re
 *
 * This is the entry point for launching the editor.
 */

open Revery;

open Oni_CLI;
open Oni_UI;

module Core = Oni_Core;
module Input = Oni_Input;
module Model = Oni_Model;
module Store = Oni_Store;
module ExtM = Service_Extensions.Management;
module Log = (val Core.Log.withNamespace("Oni2_editor"));
module ReveryLog = (val Core.Log.withNamespace("Revery"));

module LwtEx = Core.Utility.LwtEx;
module OptionEx = Core.Utility.OptionEx;

Log.debug("Startup: Parsing CLI options");
let (cliOptions, eff) = Oni_CLI.parse(~getenv=Sys.getenv_opt, Sys.argv);

if (cliOptions.needsConsole) {
  Revery.App.initConsole();
};

switch (eff) {
| PrintVersion => Cli.printVersion() |> exit
| InstallExtension(name) => Cli.installExtension(name, cliOptions) |> exit
| QueryExtension(name) => Cli.queryExtension(name, cliOptions) |> exit
| UninstallExtension(name) =>
  Cli.uninstallExtension(name, cliOptions) |> exit
| CheckHealth => HealthCheck.run(~checks=All, cliOptions) |> exit
| ListExtensions => Cli.listExtensions(cliOptions) |> exit
| StartSyntaxServer({parentPid, namedPipe}) =>
  Oni_Syntax_Server.start(~parentPid, ~namedPipe, ~healthCheck=() =>
    HealthCheck.run(~checks=Common, cliOptions)
  )
| Run =>
  // Turn on logging, if necessary
  let loggingToConsole =
    cliOptions.attachToForeground && Option.is_some(cliOptions.logLevel);
  let loggingToFile = Option.is_some(cliOptions.logFile);

  cliOptions.logLevel |> Option.iter(Timber.App.setLevel);

  cliOptions.logFilter |> Option.iter(Timber.App.setNamespaceFilter);

  if (loggingToConsole && loggingToFile) {
    let consoleReporter =
      Timber.Reporter.console(~enableColors=?cliOptions.logColorsEnabled, ());
    let fileReporter = Option.get(cliOptions.logFile) |> Timber.Reporter.file;

    let reporter = Timber.Reporter.combine(consoleReporter, fileReporter);
    Timber.App.enable(reporter);
  } else if (loggingToConsole) {
    let consoleReporter =
      Timber.Reporter.console(~enableColors=?cliOptions.logColorsEnabled, ());
    Timber.App.enable(consoleReporter);
  } else if (loggingToFile) {
    let fileReporter = Option.get(cliOptions.logFile) |> Timber.Reporter.file;
    Timber.App.enable(fileReporter);
  };
  Oni_Core.Log.init();

  // #1161 - OSX - Make sure we're using the terminal / shell PATH.
  Core.ShellUtility.fixOSXPath();

  let initWorkingDirectory = () => {
    let path =
      switch (Oni_CLI.(cliOptions.folder)) {
      | Some(folder) => folder
      | None =>
        switch (Store.Persistence.Global.workspace()) {
        | Some(path) => path
        | None =>
          Dir.User.document()
          |> Option.value(~default=Dir.home())
          |> Fp.toString
        }
      };

    Log.info("Startup: Changing folder to: " ++ path);
    try(Sys.chdir(path)) {
    | Sys_error(msg) => Log.error("Folder does not exist: " ++ msg)
    };

    path;
  };

  // Fix for https://github.com/onivim/oni2/issues/2229
  // Can we take the displays into account, to see if the negative position is actually valid?
  // It's normal for positions to be negative, depending on display configuration - but this filters
  // out the extreme case where we are persisting incorrect values (like -32000 in the case of #2229).
  let isValidPosition = position => position > (-2000);

  let positionToString = (
    fun
    | `Absolute(v) => Printf.sprintf("Absolute(%d)", v)
    | `Centered => "Centered"
  );

  let createWindow = (~forceScaleFactor, ~workingDirectory, app) => {
    let (x, y, width, height, maximized) = {
      open Store.Persistence.Workspace;
      let store = storeFor(workingDirectory);

      (
        windowX(store)
        |> OptionEx.tap(x => Log.infof(m => m("Unsanitized x value: %d", x)))
        |> OptionEx.filter(isValidPosition)
        |> Option.fold(~some=x => `Absolute(x), ~none=`Centered),
        windowY(store)
        |> OptionEx.tap(y => Log.infof(m => m("Unsanitized x value: %d", y)))
        |> OptionEx.filter(isValidPosition)
        |> Option.fold(~some=y => `Absolute(y), ~none=`Centered),
        windowWidth(store),
        windowHeight(store),
        windowMaximized(store),
      );
    };

    Log.infof(m =>
      m(
        "Sanitized values from persistence - x: %s y: %s width: %d height: %d",
        x |> positionToString,
        y |> positionToString,
        width,
        height,
      )
    );

    let decorated =
      switch (Revery.Environment.os) {
      | Windows => false
      | _ => true
      };

    let icon =
      switch (Revery.Environment.os) {
      | Mac =>
        switch (Sys.getenv_opt("ONI2_BUNDLED")) {
        | Some(_) => None
        | None => Some("logo.png")
        }
      | _ => Some("logo.png")
      };

    let window =
      App.createWindow(
        ~createOptions=
          WindowCreateOptions.create(
            ~forceScaleFactor,
            ~acceleration=cliOptions.gpuAcceleration,
            ~maximized,
            ~vsync=Vsync.Immediate,
            ~icon,
            ~titlebarStyle=WindowStyles.Transparent,
            ~x,
            ~y,
            ~width,
            ~height,
            ~decorated,
            (),
          ),
        app,
        "Oni2",
      );

    Window.setBackgroundColor(window, Colors.black);

    window;
  };
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

    Vim.init();

    let initialWorkingDirectory = initWorkingDirectory();
    let window =
      createWindow(
        ~forceScaleFactor=cliOptions.forceScaleFactor,
        ~workingDirectory=initialWorkingDirectory,
        app,
      );

    Log.debug("Initializing setup.");
    let setup = Core.Setup.init();

    let getUserSettings = Feature_Configuration.UserSettingsProvider.getSettings;

    let initialBuffer = {
      let Vim.BufferMetadata.{id, version, filePath, modified, _} =
        Vim.Buffer.openFile(Core.BufferPath.welcome)
        |> Vim.BufferMetadata.ofBuffer;
      Core.Buffer.ofMetadata(
        ~font=Service_Font.default(),
        ~id,
        ~version,
        ~filePath,
        ~modified,
      );
    };

    let initialBufferRenderers =
      Model.BufferRenderers.(
        initial
        |> setById(
             Core.Buffer.getId(initialBuffer),
             Model.BufferRenderer.Welcome,
           )
      );

    let extensionGlobalPersistence =
      Store.Persistence.Global.extensionValues();

    let initialWorkspaceStore =
      Store.Persistence.Workspace.storeFor(initialWorkingDirectory);
    let extensionWorkspacePersistence =
      Store.Persistence.Workspace.extensionValues(initialWorkspaceStore);

    let currentState =
      ref(
        Model.State.initial(
          ~cli=cliOptions,
          ~initialBuffer,
          ~initialBufferRenderers,
          ~getUserSettings,
          ~extensionGlobalPersistence,
          ~extensionWorkspacePersistence,
          ~contributedCommands=[], // TODO
          ~workingDirectory=initialWorkingDirectory,
          // TODO: Use `Fp.t` all the way down
          ~extensionsFolder=cliOptions.overriddenExtensionsDir,
        ),
      );

    let persistGlobal = () => Store.Persistence.Global.persist(currentState^);
    let persistWorkspace = () =>
      Store.Persistence.Workspace.(
        persist(
          (currentState^, window),
          storeFor(currentState^.workspace.workingDirectory),
        )
      );

    let uiDispatch = ref(_ => ());

    let update =
      UI.start(window, <Root state=currentState^ dispatch=uiDispatch^ />);

    let setTitle = title => {
      Window.setTitle(window, title);
    };

    let lastTitle = ref("");
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

    let title = (state: Model.State.t) => {
      let activeBuffer = Model.Selectors.getActiveBuffer(state);
      let config = Model.Selectors.configResolver(state);

      Feature_TitleBar.title(
        ~activeBuffer,
        ~config,
        ~workspaceRoot=state.workspace.rootName,
        ~workspaceDirectory=state.workspace.workingDirectory,
      );
    };

    let tick = _dt => {
      runEventLoop();

      if (isDirty^) {
        update(<Root state=currentState^ dispatch=uiDispatch^ />);
        isDirty := false;
        persistGlobal();
      };

      let currentTitle = title(currentState^);
      if (lastTitle^ != currentTitle) {
        Log.infof(m => m("Setting title: %s", currentTitle));
        lastTitle := currentTitle;
        setTitle(currentTitle);
      };
    };
    let _: unit => unit = Tick.interval(tick, Time.zero);

    let getZoom = () => {
      Window.getZoom(window);
    };

    let setZoom = zoomFactor => Window.setZoom(window, zoomFactor);

    let maximize = () => {
      Window.maximize(window);
    };

    let minimize = () => {
      Window.minimize(window);
    };

    let close = () => {
      App.quit(~askNicely=true, app);
    };

    let restore = () => {
      Window.restore(window);
    };

    // This is called raiseWIndow because if it were simply raise, it would shadow the exception raising function
    let raiseWindow = () => {
      Window.raise(window);
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
        ~setVsync,
        ~maximize,
        ~minimize,
        ~restore,
        ~raiseWindow,
        ~close,
        ~window=Some(window),
        ~filesToOpen=cliOptions.filesToOpen,
        ~shouldLoadExtensions=cliOptions.shouldLoadConfiguration,
        ~shouldSyntaxHighlight=cliOptions.shouldSyntaxHighlight,
        ~shouldLoadConfiguration=cliOptions.shouldLoadConfiguration,
        ~overriddenExtensionsDir=cliOptions.overriddenExtensionsDir,
        ~quit,
        (),
      );

    uiDispatch := dispatch;
    Log.debug("Startup: StoreThread started!");

    let _: App.unsubscribe =
      App.onFileOpen(app, path => {
        dispatch(Model.Actions.OpenFileByPath(path, None, None))
      });
    let _: Window.unsubscribe =
      Window.onMaximized(window, () =>
        dispatch(Model.Actions.WindowMaximized)
      );
    let _: Window.unsubscribe =
      Window.onFullscreen(window, () =>
        dispatch(Model.Actions.WindowFullscreen)
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

    GlobalContext.set({dispatch: dispatch});

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
  let () = App.start(init);
  ();
};
