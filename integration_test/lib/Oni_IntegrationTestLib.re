module Core = Oni_Core;
module Option = Core.Utility.Option;

module Model = Oni_Model;
module Store = Oni_Store;
module Log = Core.Log;
module TextSynchronization = TextSynchronization;

open Types;

let _currentClipboard: ref(option(string)) = ref(None);
let _currentTime: ref(float) = ref(0.0);
let _currentZoom: ref(float) = ref(1.0);
let _currentTitle: ref(string) = ref("");
let _currentVsync: ref(Revery.Vsync.t) = ref(Revery.Vsync.Immediate);

let setClipboard = v => _currentClipboard := v;
let getClipboard = () => _currentClipboard^;

let setTime = v => _currentTime := v;
let getTime = () => _currentTime^;

let setTitle = title => _currentTitle := title;
let getTitle = () => _currentTitle^;

let setZoom = v => _currentZoom := v;
let getZoom = () => _currentZoom^;

let getScaleFactor = () => 1.0;

let setVsync = vsync => _currentVsync := vsync;

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

let runTest =
    (
      ~configuration=None,
      ~cliOptions=None,
      ~name="AnonymousTest",
      test: testCallback,
    ) => {
  Printexc.record_backtrace(true);
  Log.enablePrinting();
  Log.enableDebugLogging();
  Log.info("Starting test... Working directory: " ++ Sys.getcwd());

  let setup = Core.Setup.init() /* let cliOptions = Core.Cli.parse(setup); */;

  let initialState = Model.State.create();
  let currentState = ref(initialState);

  let onStateChanged = v => {
    currentState := v;
  };

  let logInit = s => Log.debug(() => "[INITIALIZATION] " ++ s);

  logInit("Starting store...");

  let configurationFilePath = Filename.temp_file("configuration", ".json");
  let oc = open_out(configurationFilePath);

  logInit("Writing configuration file: " ++ configurationFilePath);

  let () =
    configuration
    |> Option.value(~default="{}")
    |> Printf.fprintf(oc, "%s\n");

  close_out(oc);

  let (dispatch, runEffects) =
    Store.StoreThread.start(
      ~setup,
      ~getClipboardText=() => _currentClipboard^,
      ~setClipboardText=text => setClipboard(Some(text)),
      ~getScaleFactor,
      ~getTime,
      ~setTitle,
      ~getZoom,
      ~setZoom,
      ~setVsync,
      ~executingDirectory=Revery.Environment.getExecutingDirectory(),
      ~onStateChanged,
      ~cliOptions,
      ~configurationFilePath=Some(configurationFilePath),
      ~quit,
      ~window=None,
      (),
    );

  logInit("Store started!");

  logInit("Sending init event");

  dispatch(Model.Actions.Init);

  let wrappedRunEffects = () => {
    runEffects();
  };

  wrappedRunEffects();

  let wrappedDispatch = action => {
    dispatch(action);
  };

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

      // Flush any pending effects
      wrappedRunEffects();

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
  test(wrappedDispatch, waitForState, wrappedRunEffects);
  Log.info("--- TEST COMPLETE: " ++ name);

  dispatch(Model.Actions.Quit(true));
};

let runTestWithInput = (~name, f: testCallbackWithInput) => {
  runTest(
    ~name,
    (dispatch, wait, runEffects) => {
      let input = key => {
        dispatch(Model.Actions.KeyboardInput(key));
        runEffects();
      };

      f(input, dispatch, wait, runEffects);
    },
  );
};
