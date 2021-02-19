open Oni_Core;
open Utility;

module Log = (val Log.withNamespace("Feature.Logging"));

// MODEL

type model = unit;
let initial = ();

[@deriving show]
type command =
  | LogToTraceFile;

[@deriving show]
type msg =
  | Command(command)
  | LogFileStarted([@opaque] FpExp.t(FpExp.absolute));

// EFFECT

module Effects = {
  let traceToFile = (~toMsg) =>
    Isolinear.Effect.createWithDispatch(
      ~name="Feature.Logging.traceToFile", dispatch => {
      Log.warn("Switching to trace file; further logging will be to file.");
      let logFileName = "onivim2-trace.log";
      let maybeWorkingDirectory = Luv.Path.cwd();
      let maybeLogPath =
        maybeWorkingDirectory
        |> Result.to_option
        |> OptionEx.flatMap(FpExp.absoluteCurrentPlatform)
        |> Option.map(path => FpExp.At.(path / logFileName));

      maybeLogPath
      |> Option.iter(logPath => {
           Timber.App.enable(Timber.Reporter.file(FpExp.toString(logPath)));
           Timber.App.setLevel(Timber.Level.trace);
           dispatch(toMsg(logPath));
         });
    });
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
    }
  | LogFileStarted(filePath) => (
      (),
      ShowInfoNotification(
        Printf.sprintf("Logging to: %s", FpExp.toString(filePath)),
      ),
    )
  };

module Commands = {
  open Feature_Commands.Schema;

  let nextEditor =
    define(
      ~category="Debug",
      ~title="Log to trace file",
      "oni.debug.startTraceLogging",
      Command(LogToTraceFile),
    );
};

module Contributions = {
  let commands = Commands.[nextEditor];
};
