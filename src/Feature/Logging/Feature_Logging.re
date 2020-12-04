open Oni_Core;

module Log = (val Log.withNamespace("Feature.Logging"));

// MODEL

type model = unit;
let initial = ();

[@deriving show]
type command =
  | LogToTraceFile
  | ToggleRenderPerformanceView;

[@deriving show]
type msg =
  | Command(command)
  | LogFileStarted(string);

// EFFECT

module Effects = {
  let traceToFile = (~toMsg) =>
    Isolinear.Effect.createWithDispatch(
      ~name="Feature.Logging.traceToFile", dispatch => {
      Log.warn("Switching to trace file; further logging will be to file.");
      let logFileName = "onivim2-trace.log";
      Timber.App.enable(Timber.Reporter.file(logFileName));
      Timber.App.setLevel(Timber.Level.trace);
      dispatch(toMsg(logFileName));
    });

  let toggleDebugDraw =
    Isolinear.Effect.create(~name="Feature.Logging.toggleDebugDraw", () =>
      if (Revery.Debug.isEnabled()) {
        Revery.Debug.disable();
      } else {
        Revery.Debug.enable();
      }
    );
};

// UPDATE

type outmsg =
  | Effect(Isolinear.Effect.t(msg))
  | ShowInfoNotification(string);

let update = (msg, _model) =>
  switch (msg) {
  | Command(command) =>
    switch (command) {
    | LogToTraceFile => (
        (),
        Effect(Effects.traceToFile(~toMsg=str => LogFileStarted(str))),
      )
    | ToggleRenderPerformanceView => ((), Effect(Effects.toggleDebugDraw))
    }
  | LogFileStarted(filePath) => (
      (),
      ShowInfoNotification("Logging to: " ++ filePath),
    )
  };

module Commands = {
  open Feature_Commands.Schema;

  let logToTraceFile =
    define(
      ~category="Debug",
      ~title="Log to trace file",
      "oni.debug.startTraceLogging",
      Command(LogToTraceFile),
    );

  let showDebugVisualization =
    define(
      ~category="Debug",
      ~title="Render performance",
      "oni.debug.toggleRenderPerformance",
      Command(ToggleRenderPerformanceView),
    );
};

module Contributions = {
  let commands = Commands.[logToTraceFile, showDebugVisualization];
};
