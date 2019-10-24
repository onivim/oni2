module Core = Oni_Core;
module Model = Oni_Model;
module Store = Oni_Store;
module Log = Core.Log;

type dispatchFunction = Model.Actions.t => unit;
type runEffectsFunction = unit => unit;
type waiter = Model.State.t => bool;
type waitForState = (~name: string, ~timeout: float=?, waiter) => unit;

type testCallback =
  (dispatchFunction, waitForState, runEffectsFunction) => unit;

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

let setVsync = (vsync) => _currentVsync := vsync;

let quit = code => exit(code);

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

  let setup = Core.Setup.init() /* let cliOptions = Core.Cli.parse(setup); */;

  let initialState = Model.State.create();
  let currentState = ref(initialState);

  let onStateChanged = v => {
    currentState := v;
  };

  let logInit = s => Log.debug(() => "[INITIALIZATION] " ++ s);

  logInit("Starting store...");

  let configPath =
    switch (configuration) {
    | None => None
    | Some(v) =>
      let tempFile = Filename.temp_file("configuration", ".json");
      let oc = open_out(tempFile);
      Printf.fprintf(oc, "%s\n", v);
      close_out(oc);
      Some(tempFile);
    };

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
      ~configurationFilePath=configPath,
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
      logWaiter("FAILED");
      assert(false == true);
    };
  };

  Log.info("--- Starting test: " ++ name);
  test(wrappedDispatch, waitForState, wrappedRunEffects);
  Log.info("--- TEST COMPLETE: " ++ name);

  dispatch(Model.Actions.Quit(true));
};
