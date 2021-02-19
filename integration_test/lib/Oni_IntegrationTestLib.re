module Core = Oni_Core;
module FpExp = Core.FpExp;
module Utility = Core.Utility;

module Model = Oni_Model;
module Store = Oni_Store;
module Log = (val Core.Log.withNamespace("IntegrationTest"));
module InitLog = (val Core.Log.withNamespace("IntegrationTest.Init"));
module TextSynchronization = TextSynchronization;
module ExtensionHelpers = ExtensionHelpers;
module SyntaxServerTest = SyntaxServerTest;

open Types;

let _currentClipboard: ref(option(string)) = ref(None);
let _currentTime: ref(float) = ref(0.0);
let _currentZoom: ref(float) = ref(1.0);
let _currentTitle: ref(string) = ref("");
let _currentVsync: ref(Revery.Vsync.t) = ref(Revery.Vsync.Immediate);
let _currentMaximized: ref(bool) = ref(false);
let _currentMinimized: ref(bool) = ref(false);
let _currentRaised: ref(bool) = ref(false);

let setClipboard = v => _currentClipboard := v;
let getClipboard = () => _currentClipboard^;

Service_Clipboard.Testing.setClipboardProvider(~get=() => _currentClipboard^);

let setTime = v => _currentTime := v;

let setZoom = v => _currentZoom := v;
let getZoom = () => _currentZoom^;

let setVsync = vsync => _currentVsync := vsync;

let maximize = () => _currentMaximized := true;
let minimize = () => _currentMinimized := true;
let restore = () => {
  _currentMaximized := false;
  _currentMinimized := false;
};
let raiseWindow = () => _currentRaised := true;

let quit = code => exit(code);

let close = () => quit(0) |> ignore;

exception TestAssetNotFound(string);

let getAssetPath = path =>
  // Does it exist
  if (Sys.file_exists(path)) {
    Log.info("Found file at: " ++ path);
    path;
  } else if (Sys.file_exists("integration_test/" ++ path)) {
    let result = "integration_test/" ++ path;
    Log.info("Found file at: " ++ result);
    result;
  } else {
    raise(TestAssetNotFound(path));
  };

let currentUserSettings = ref(Core.Config.Settings.empty);
let setUserSettings = settings => currentUserSettings := settings;

module Internal = {
  let prepareEnvironment = () => {
    // On Windows, all the paths for build dependencies brought in by esy can easily cause us to
    // exceed the environment limit (the limit of _all_ environment variables).
    // When this occurs, we hit an assertion in nodejs:
    // https://github.com/libuv/libuv/issues/2587

    // To work around this, for the purpose of integration tests (which are run in the esy environment),
    // we'll unset some environment variables that can take up a lot of space, but aren't needed.
    ["CAML_LD_LIBRARY_PATH", "MAN_PATH", "PKG_CONFIG_PATH", "OCAMLPATH'"]
    |> List.iter(p =>
         ignore(Luv.Env.unsetenv(p): result(unit, Luv.Error.t))
       );

    Log.info("== Checking environment === ");
    Unix.environment() |> Array.to_list |> List.iter(Log.info);
    Log.info("== Environment check complete ===");
  };
};

let runTest =
    (
      ~configuration=None,
      ~keybindings=None,
      ~filesToOpen=[],
      ~name="AnonymousTest",
      ~onAfterDispatch=_ => (),
      test: testCallback,
    ) => {
  // Disable colors on windows to prevent hanging on CI

  Revery.App.initConsole();

  Core.Log.enableDebug();
  Timber.App.setLevel(Timber.Level.trace);
  Oni_Core.Log.init();

  Internal.prepareEnvironment();

  switch (Sys.getenv_opt("ONI2_LOG_FILE")) {
  | None => Timber.App.enable(Timber.Reporter.console())
  | Some(logFile) =>
    let fileReporter = Timber.Reporter.file(logFile);
    let reporters = Timber.Reporter.(combine(console(), fileReporter));
    Timber.App.enable(reporters);
  };

  Log.info("Starting test... Working directory: " ++ Sys.getcwd());

  let setup = Core.Setup.init() /* let cliOptions = Core.Cli.parse(setup); */;

  currentUserSettings :=
    (
      switch (configuration) {
      | Some(json) =>
        json |> Yojson.Safe.from_string |> Core.Config.Settings.fromJson
      | None => Core.Config.Settings.empty
      }
    );

  let getUserSettings = () => Ok(currentUserSettings^);

  Vim.init();
  Oni2_KeyboardLayout.init();
  Log.infof(m =>
    m(
      "Keyboard Language: %s Layout: %s",
      Oni2_KeyboardLayout.getCurrentLanguage(),
      Oni2_KeyboardLayout.getCurrentLayout(),
    )
  );

  let initialBuffer = {
    let Vim.BufferMetadata.{id, version, filePath, modified, _} =
      Vim.Buffer.openFile("untitled") |> Vim.BufferMetadata.ofBuffer;
    Core.Buffer.ofMetadata(
      ~font=Oni_Core.Font.default(),
      ~id,
      ~version,
      ~filePath,
      ~modified,
    );
  };
  let writeConfigurationFile = (name, jsonStringOpt) => {
    let tempFilePath = Filename.temp_file(name, ".json");
    let oc = open_out(tempFilePath);

    InitLog.info("Writing configuration file: " ++ tempFilePath);

    let () =
      jsonStringOpt
      |> Option.value(~default="{}")
      |> Printf.fprintf(oc, "%s\n");

    close_out(oc);
    tempFilePath |> FpExp.absoluteCurrentPlatform |> Option.get;
  };

  let keybindingsFilePath =
    writeConfigurationFile("keybindings", keybindings);

  let keybindingsLoader =
    Feature_Input.KeybindingsLoader.file(keybindingsFilePath);

  let currentState =
    ref(
      Model.State.initial(
        ~cli=Oni_CLI.default,
        ~initialBuffer,
        ~initialBufferRenderers=Model.BufferRenderers.initial,
        ~getUserSettings,
        ~keybindingsLoader,
        ~contributedCommands=[],
        ~maybeWorkspace=None,
        ~workingDirectory=Sys.getcwd(),
        ~extensionsFolder=None,
        ~extensionGlobalPersistence=Feature_Extensions.Persistence.initial,
        ~extensionWorkspacePersistence=Feature_Extensions.Persistence.initial,
        ~licenseKeyPersistence=None,
        ~titlebarHeight=0.,
        ~setZoom,
        ~getZoom,
      ),
    );

  let headlessWindow =
    Revery.Utility.HeadlessWindow.create(
      Revery.WindowCreateOptions.create(~width=3440, ~height=1440, ()),
    );

  let onStateChanged = state => {
    currentState := state;
  };

  let uiDispatch = ref(_ => ());

  let _: unit => unit =
    Revery.Tick.interval(
      ~name="Integration Test Ticker",
      _ => {
        let state = currentState^;
        Revery.Utility.HeadlessWindow.render(
          headlessWindow,
          <Oni_UI.Root state dispatch=uiDispatch^ />,
        );
      },
      // Revery.Utility.HeadlessWindow.takeScreenshot(
      //   headlessWindow,
      //   "screenshot.png",
      // );
      Revery.Time.zero,
    );

  InitLog.info("Starting store...");

  let configurationFilePath =
    writeConfigurationFile("configuration", configuration);

  let (dispatch, runEffects) =
    Store.StoreThread.start(
      ~showUpdateChangelog=false,
      ~getUserSettings,
      ~setup,
      ~onAfterDispatch,
      ~getClipboardText=() => _currentClipboard^,
      ~setClipboardText=text => setClipboard(Some(text)),
      ~setVsync,
      ~maximize,
      ~minimize,
      ~restore,
      ~raiseWindow,
      ~close,
      ~executingDirectory=Revery.Environment.getExecutingDirectory(),
      ~getState=() => currentState^,
      ~onStateChanged,
      ~configurationFilePath=Some(configurationFilePath),
      ~quit,
      ~window=None,
      ~filesToOpen,
      (),
    );

  uiDispatch := dispatch;

  InitLog.info("Store started!");

  InitLog.info("Sending init event");

  Oni_UI.GlobalContext.set({dispatch: dispatch});

  dispatch(Model.Actions.Init);

  runEffects();

  let waitForState = (~name, ~timeout=0.5, waiter) => {
    let logWaiter = msg => Log.info(" WAITER (" ++ name ++ "): " ++ msg);
    let startTime = Unix.gettimeofday();
    let maxWaitTime = timeout;
    let iteration = ref(0);

    logWaiter("Starting");
    while (!waiter(currentState^)
           && Unix.gettimeofday()
           -. maxWaitTime < startTime) {
      logWaiter("Iteration: " ++ string_of_int(iteration^));
      incr(iteration);

      // Flush any queued calls from `Revery.App.runOnMainThread`
      Revery.App.flushPendingCallbacks();
      Revery.Tick.pump();

      for (_ in 1 to 100) {
        ignore(Luv.Loop.run(~mode=`NOWAIT, ()): bool);
      };

      // Flush any pending effects
      runEffects();

      Unix.sleepf(0.1);
      Thread.yield();
    };

    let result = waiter(currentState^);

    logWaiter("Finished - result: " ++ string_of_bool(result));

    if (!result) {
      logWaiter("FAILED: " ++ Sys.executable_name);
      assert(false == true);
    };
  };

  let staysTrue = (~name, ~timeout, waiter) => {
    let logWaiter = msg => Log.info(" STAYS TRUE (" ++ name ++ "): " ++ msg);
    let startTime = Unix.gettimeofday();
    let maxWaitTime = timeout;
    let iteration = ref(0);

    logWaiter("Starting");
    while (waiter(currentState^)
           && Unix.gettimeofday()
           -. maxWaitTime < startTime) {
      logWaiter("Iteration: " ++ string_of_int(iteration^));
      incr(iteration);

      // Flush any queued calls from `Revery.App.runOnMainThread`
      Revery.App.flushPendingCallbacks();
      Revery.Tick.pump();

      for (_ in 1 to 100) {
        ignore(Luv.Loop.run(~mode=`NOWAIT, ()): bool);
      };

      // Flush any pending effects
      runEffects();

      Unix.sleepf(0.1);
      Thread.yield();
    };

    let result = waiter(currentState^);

    logWaiter("Finished - result: " ++ string_of_bool(result));

    if (!result) {
      logWaiter("FAILED: " ++ Sys.executable_name);
      assert(false == true);
    };
  };

  let key = (~modifiers=EditorInput.Modifiers.none, key) => {
    let keyPress =
      EditorInput.KeyPress.physicalKey(~key, ~modifiers)
      |> EditorInput.KeyCandidate.ofKeyPress;
    let time = Revery.Time.now();
    dispatch(Model.Actions.KeyDown({key: keyPress, scancode: 1, time}));
    dispatch(Model.Actions.KeyUp({scancode: 1, time}));
    runEffects();
  };

  let input = (~modifiers=EditorInput.Modifiers.none, str) => {
    str
    |> Zed_utf8.explode
    |> List.iter(uchar => {
         key(~modifiers, EditorInput.Key.Character(uchar))
       });
  };

  let ctx = {dispatch, wait: waitForState, runEffects, input, key, staysTrue};

  Log.info("--- Starting test: " ++ name);
  test(ctx);
  Log.info("--- TEST COMPLETE: " ++ name);

  dispatch(Model.Actions.Quit(true));
};

let runCommand = (~dispatch, command: Core.Command.t(_)) =>
  switch (command.msg) {
  | `Arg0(msg) => dispatch(msg)
  | `Arg1(_) => ()
  };
