open Oni_Core;
open Oni_Core.Utility;
open Oni_Extensions;
open Timber;

type testContext = {
  syntaxClient: Oni_Syntax_Client.t,
  isConnected: unit => bool,
  hasExited: unit => option(int),
  wait: (~name: string=?, unit => bool) => unit,
};

let run = (~parentPid=?, ~name, f) => {
  Timber.App.enable();
  Timber.App.setLevel(Timber.Level.debug);
  module Log = (val Log.withNamespace(name));
  Log.info("Starting test: " ++ name);
  let connected = ref(false);
  let onConnected = () => {
    Log.info("Connected to syntax server");
    connected := true;
  };

  let wait = (~name="TODO", f) => {
    ThreadEx.waitForCondition(f);
    if (!f()) {
      let msg = "Failed: " ++ name;
      Log.error(msg);
      failwith(msg);
    };
  };

  let exitCode = ref(None);
  let onClose = code => {
    Log.infof(m => m("Syntax server closed with code: %d", code));
    exitCode := Some(code);
  };

  let syntaxClient =
    Oni_Syntax_Client.start(
      ~parentPid?,
      ~executablePath=
        Revery.Environment.executingDirectory ++ "SyntaxServer.exe",
      ~onConnected,
      ~onClose,
      ~onHighlights=_ => (),
      ~onHealthCheckResult=_ => (),
      ~scheduler=Scheduler.immediate,
      LanguageInfo.initial,
      Setup.default(),
    );

  let isConnected = () => connected^;
  let hasExited = () => exitCode^;
  wait(~name="Connected", () => connected^);

  f({syntaxClient, isConnected, hasExited, wait});
  Log.info("== PASSED ==");
};
