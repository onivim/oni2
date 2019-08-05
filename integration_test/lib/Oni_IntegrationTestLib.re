module Core = Oni_Core;
module Model = Oni_Model;
module Store = Oni_Store;
module Log = Core.Log;

type dispatchFunction = Model.Actions.t => unit;
type runEffectsFunction = unit => unit;
type waiter = Model.State.t => bool;
type waitForState = (~name: string, waiter) => unit;

type testCallback =
  (dispatchFunction, waitForState, runEffectsFunction) => unit;

let _currentClipboard: ref(option(string)) = ref(None);

let setClipboard = v => _currentClipboard := v;

let runTest = (~name="AnonymousTest", test: testCallback) => {
  Printexc.record_backtrace(true);
  Log.enablePrinting();
  Log.enableDebugLogging();

  let setup = Core.Setup.init() /* let cliOptions = Core.Cli.parse(setup); */;

  let initialState = Model.State.create();
  let currentState = ref(initialState);

  let onStateChanged = v => {
    currentState := v;
  };

  let logInit = s => Log.debug("[INITILIAZATION] " ++ s);

  logInit("Starting store...");

  let (dispatch, runEffects) =
    Store.StoreThread.start(
      ~setup,
      ~getClipboardText=() => _currentClipboard^,
      ~executingDirectory=Revery.Environment.getExecutingDirectory(),
      ~onStateChanged,
      (),
    );

  logInit("Store started!");

  logInit("Sending init event");

  dispatch(Model.Actions.Init);

  let wrappedRunEffects = () => {
    runEffects();
  };

  wrappedRunEffects();

  logInit("Setting editor font");
  dispatch(
    Model.Actions.SetEditorFont(
      Core.Types.EditorFont.create(
        ~fontFile="test_font",
        ~fontSize=14,
        ~measuredWidth=7.5,
        ~measuredHeight=10.25,
        (),
      ),
    ),
  );

  let wrappedDispatch = action => {
    dispatch(action);
  };

  let waitForState = (~name, waiter) => {
    let logWaiter = msg => Log.info(" WAITER (" ++ name ++ "): " ++ msg);
    let startTime = Unix.gettimeofday();
    let maxWaitTime = 0.5;
    let iteration = ref(0);

    logWaiter("Starting");
    while (!waiter(currentState^)
           && Unix.gettimeofday()
           -. maxWaitTime < startTime) {
      logWaiter("Iteration: " ++ string_of_int(iteration^));
      incr(iteration);
      wrappedRunEffects();
      Unix.sleepf(0.1);
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
