module Core = Oni_Core;
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

let setClipboard = v => _currentClipboard := v;
let getClipboard = () => _currentClipboard^;

let setTime = v => _currentTime := v;

let setTitle = title => _currentTitle := title;
let getTitle = () => _currentTitle^;

let setZoom = v => _currentZoom := v;
let getZoom = () => _currentZoom^;

let setVsync = vsync => _currentVsync := vsync;

let maximize = () => _currentMaximized := true;
let minimize = () => _currentMinimized := true;

let quit = code => exit(code);

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
  if (Sys.win32) {
    Timber.App.disableColors();
  };

  Revery.App.initConsole();

  Core.Log.enableDebug();
  Timber.App.enable();
  Timber.App.setLevel(Timber.Level.trace);

  switch (Sys.getenv_opt("ONI2_LOG_FILE")) {
  | None => ()
  | Some(logFile) => Timber.App.setLogFile(logFile)
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

  let currentState =
    ref(
      Model.State.initial(
        ~getUserSettings,
        ~contributedCommands=[],
        ~workingDirectory=Sys.getcwd(),
      ),
    );

  let headlessWindow =
    Revery.Utility.HeadlessWindow.create(
      Revery.WindowCreateOptions.create(~width=3440, ~height=1440, ()),
    );

  let onStateChanged = state => {
    currentState := state;
  };

  let _: unit => unit =
    Revery.Tick.interval(
      _ => {
        let state = currentState^;
        Revery.Utility.HeadlessWindow.render(
          headlessWindow,
          <Oni_UI.Root state />,
        );
      },
      //        Revery.Utility.HeadlessWindow.takeScreenshot(
      //          headlessWindow,
      //          "screenshot.png",
      //        );
      Revery.Time.zero,
    );

  InitLog.info("Starting store...");

  let writeConfigurationFile = (name, jsonStringOpt) => {
    let tempFilePath = Filename.temp_file(name, ".json");
    let oc = open_out(tempFilePath);

    InitLog.info("Writing configuration file: " ++ tempFilePath);

    let () =
      jsonStringOpt
      |> Option.value(~default="{}")
      |> Printf.fprintf(oc, "%s\n");

    close_out(oc);
    tempFilePath;
  };

  let configurationFilePath =
    writeConfigurationFile("configuration", configuration);
  let keybindingsFilePath =
    writeConfigurationFile("keybindings", keybindings);

  let (dispatch, runEffects) =
    Store.StoreThread.start(
      ~showUpdateChangelog=false,
      ~getUserSettings,
      ~setup,
      ~onAfterDispatch,
      ~getClipboardText=() => _currentClipboard^,
      ~setClipboardText=text => setClipboard(Some(text)),
      ~setTitle,
      ~getZoom,
      ~setZoom,
      ~setVsync,
      ~maximize,
      ~minimize,
      ~executingDirectory=Revery.Environment.getExecutingDirectory(),
      ~getState=() => currentState^,
      ~onStateChanged,
      ~configurationFilePath=Some(configurationFilePath),
      ~keybindingsFilePath=Some(keybindingsFilePath),
      ~quit,
      ~window=None,
      ~filesToOpen,
      (),
    );

  InitLog.info("Store started!");

  InitLog.info("Sending init event");

  Oni_UI.GlobalContext.set({
    closeEditorById: id => dispatch(Model.Actions.ViewCloseEditor(id)),
    editorScrollDelta: (~editorId, ~deltaY, ()) =>
      dispatch(Model.Actions.EditorScroll(editorId, deltaY)),
    editorSetScroll: (~editorId, ~scrollY, ()) =>
      dispatch(Model.Actions.EditorSetScroll(editorId, scrollY)),
    dispatch,
  });

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

  Log.info("--- Starting test: " ++ name);
  test(dispatch, waitForState, runEffects);
  Log.info("--- TEST COMPLETE: " ++ name);

  dispatch(Model.Actions.Quit(true));
};

let runTestWithInput =
    (
      ~configuration=?,
      ~keybindings=?,
      ~name,
      ~onAfterDispatch=?,
      f: testCallbackWithInput,
    ) => {
  runTest(
    ~name,
    ~configuration?,
    ~keybindings?,
    ~onAfterDispatch?,
    (dispatch, wait, runEffects) => {
      let input = key => {
        dispatch(Model.Actions.KeyboardInput(key));
        runEffects();
      };

      f(input, dispatch, wait, runEffects);
    },
  );
};
