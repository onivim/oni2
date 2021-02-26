/*
 * Oni2.re
 *
 * This is the entry point for launching the editor.
 */

open Revery;

open Oni_CLI;
open Oni_UI;

module Core = Oni_Core;
module FpExp = Oni_Core.FpExp;
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

let initializeLogging = () => {
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
};

switch (eff) {
| PrintVersion => Cli.printVersion() |> exit
| InstallExtension(name) => Cli.installExtension(name, cliOptions) |> exit
| QueryExtension(name) => Cli.queryExtension(name, cliOptions) |> exit
| UninstallExtension(name) =>
  Cli.uninstallExtension(name, cliOptions) |> exit
| CheckHealth =>
  initializeLogging();
  HealthCheck.run(~checks=All, cliOptions) |> exit;
| ListExtensions => Cli.listExtensions(cliOptions) |> exit
| StartSyntaxServer({parentPid, namedPipe}) =>
  Oni_Syntax_Server.start(~parentPid, ~namedPipe, ~healthCheck=() =>
    HealthCheck.run(~checks=Common, cliOptions)
  )
| Run =>
  initializeLogging();

  // #1161 - OSX - Make sure we're using the terminal / shell PATH.
  // Only fix path when launched from finder -
  // it seems running `zsh -ilc` hangs when running from terminal.
  if (Sys.getenv_opt("ONI2_LAUNCHED_FROM_FINDER") |> Option.is_some) {
    Core.ShellUtility.fixOSXPath();
  };

  let initWorkspace = () => {
    let maybePath =
      switch (Oni_CLI.(cliOptions.folder)) {
      // If a folder was specified, we should for sure use that
      | Some(folder) => Some(folder)

      // If no files were specified, we can pull from persistence
      | None when cliOptions.filesToOpen == [] =>
        Store.Persistence.Global.workspace()

      // If files were specified (#1983), don't open workspace from persistence
      | None => None
      };

    let couldChangeDirectory = ref(false);
    maybePath
    |> Option.iter(path => {
         Log.infof(m => m("Startup: Trying to change folder to: %s", path));

         let chdirResult = {
           open Base.Result.Let_syntax;
           // First, check if we have permission to read the directory.
           // In some cases - like #2742 - the directory might not be valid.
           let%bind _: Luv.File.Dir.t = Luv.File.Sync.opendir(path);
           Log.info(" - Have read permission");
           let%bind () = Luv.Path.chdir(path);
           Log.info("- Ran chdir");
           Ok();
         };

         chdirResult
         |> Result.iter(() => {
              couldChangeDirectory := true;
              Log.infof(m =>
                m("Successfully changed working directory to: %s", path)
              );
            });
       });

    // The directory that was persisted is a valid workspace, so we can use it
    if (couldChangeDirectory^) {
      maybePath |> OptionEx.flatMap(FpExp.absoluteCurrentPlatform);
    } else {
      None;
    };
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

  let createWindow = (~forceScaleFactor, ~maybeWorkspace, app) => {
    let (x, y, width, height, maximized) = {
      Store.Persistence.Workspace.(
        maybeWorkspace
        |> Option.map(workspace => {
             let store = storeFor(FpExp.toString(workspace));
             (
               windowX(store)
               |> OptionEx.tap(x =>
                    Log.infof(m => m("Unsanitized x value: %d", x))
                  )
               |> OptionEx.filter(isValidPosition)
               |> Option.fold(~some=x => `Absolute(x), ~none=`Centered),
               windowY(store)
               |> OptionEx.tap(y =>
                    Log.infof(m => m("Unsanitized x value: %d", y))
                  )
               |> OptionEx.filter(isValidPosition)
               |> Option.fold(~some=y => `Absolute(y), ~none=`Centered),
               windowWidth(store),
               windowHeight(store),
               windowMaximized(store),
             );
           })
        |> Option.value(~default=(`Centered, `Centered, 800, 600, false))
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
      | Windows(_) => false
      | _ => true
      };

    let icon =
      switch (Revery.Environment.os) {
      | Mac(_) =>
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
      "Starting Onivim 2 (%s / %s / %s / %s)",
      Core.BuildInfo.version,
      Core.BuildInfo.commitId,
      Feature_AutoUpdate.defaultUpdateChannel,
      Core.BuildInfo.extensionHostVersion,
    )
  );

  /* The 'main' function for our app */
  let init = app => {
    Log.debug("Init");

    Vim.init();
    Oni2_KeyboardLayout.init();
    Oni2_Sparkle.init();

    Log.infof(m =>
      m(
        "Keyboard Language: %s Layout: %s",
        Oni2_KeyboardLayout.getCurrentLanguage(),
        Oni2_KeyboardLayout.getCurrentLayout(),
      )
    );

    // Grab initial working directory prior to trying to set it -
    // in some cases, a directory that does not have permissions may be persisted (ie #2742)
    let initialWorkingDirectory = Sys.getcwd();
    let maybeWorkspace = initWorkspace();
    let workingDirectory =
      maybeWorkspace
      |> Option.map(FpExp.toString)
      |> Option.value(~default=initialWorkingDirectory);

    let window =
      createWindow(
        ~forceScaleFactor=cliOptions.forceScaleFactor,
        ~maybeWorkspace,
        app,
      );

    Log.debug("Initializing setup.");
    let setup = Core.Setup.init();

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

    let licenseKeyPersistence = Store.Persistence.Global.licenseKey();

    let initialWorkspaceStore =
      Store.Persistence.Workspace.storeFor(workingDirectory);
    let extensionWorkspacePersistence =
      Store.Persistence.Workspace.extensionValues(initialWorkspaceStore);

    let getZoom = () => {
      Window.getZoom(window);
    };

    let setZoom = zoomFactor => Window.setZoom(window, zoomFactor);

    let keybindingsLoader =
      Oni_Core.Filesystem.getOrCreateConfigFile("keybindings.json")
      |> Result.map(Feature_Input.KeybindingsLoader.file)
      |> Oni_Core.Utility.ResultEx.tapError(msg =>
           Log.errorf(m => m("Error initializing keybindings file: %s", msg))
         )
      |> Result.value(~default=Feature_Input.KeybindingsLoader.none);

    let configurationLoader =
      Feature_Configuration.(
        if (!cliOptions.shouldLoadConfiguration) {
          ConfigurationLoader.none;
        } else {
          Oni_Core.Filesystem.getOrCreateConfigFile("configuration.json")
          |> Result.map(ConfigurationLoader.file)
          |> Oni_Core.Utility.ResultEx.tapError(msg =>
               Log.errorf(m =>
                 m("Error initializing configurationj file: %s", msg)
               )
             )
          |> Result.value(~default=ConfigurationLoader.none);
        }
      );

    let currentState =
      ref(
        Model.State.initial(
          ~cli=cliOptions,
          ~initialBuffer,
          ~initialBufferRenderers,
          ~configurationLoader,
          ~keybindingsLoader,
          ~extensionGlobalPersistence,
          ~extensionWorkspacePersistence,
          ~workingDirectory,
          ~maybeWorkspace,
          // TODO: Use `FpExp.t` all the way down
          ~extensionsFolder=cliOptions.overriddenExtensionsDir,
          ~licenseKeyPersistence,
          ~titlebarHeight=Revery.Window.getTitlebarHeight(window),
          ~setZoom,
          ~getZoom,
        ),
      );

    let persistGlobal = () => Store.Persistence.Global.persist(currentState^);
    let persistWorkspace = () => {
      Feature_Workspace.openedFolder(currentState^.workspace)
      |> Option.iter(workspace => {
           Store.Persistence.Workspace.(
             persist((currentState^, window), storeFor(workspace))
           )
         });
    };

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
        ~workspaceRoot=Feature_Workspace.rootName(state.workspace),
        ~workspaceDirectory=
          Feature_Workspace.workingDirectory(state.workspace),
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
    let _: unit => unit =
      Tick.interval(~name="Oni2_Editor Apploop", tick, Time.zero);

    let maximize = () => {
      Window.maximize(window);
    };

    let minimize = () => {
      Window.minimize(window);
    };

    let close = () => {
      App.quit(~askNicely=true, app);
    };

    Callback.register("oni2_close", close);

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
        ~setup,
        ~getClipboardText=() => Sdl2.Clipboard.getText(),
        ~setClipboardText=text => Sdl2.Clipboard.setText(text),
        ~executingDirectory=Revery.Environment.executingDirectory,
        ~getState=() => currentState^,
        ~onStateChanged,
        ~setVsync,
        ~maximize,
        ~minimize,
        ~restore,
        ~raiseWindow,
        ~close,
        ~window=Some(window),
        ~shouldLoadExtensions=cliOptions.shouldLoadConfiguration,
        ~shouldSyntaxHighlight=cliOptions.shouldSyntaxHighlight,
        ~overriddenExtensionsDir=cliOptions.overriddenExtensionsDir,
        ~quit,
        (),
      );

    uiDispatch := dispatch;
    Log.debug("Startup: StoreThread started!");

    let _: App.unsubscribe =
      App.onFileOpen(app, path => {
        dispatch(Model.Actions.FilesDropped({paths: [path]}))
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

    List.iter(
      command => {
        dispatch(
          Model.Actions.VimExecuteCommand({allowAnimation: false, command}),
        );
        runEffects();
      },
      cliOptions.vimExCommands,
    );
  };

  /* Let's get this party started! */
  Log.debug("Calling App.start");
  let () = App.start(init);
  ();
};
