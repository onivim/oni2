open Oni_Core;
open Oni_Core.Utility;
open Oni_Extensions;
open Timber;

type testContext = {
  syntaxClient: Oni_Syntax_Client.t,
  isConnected: unit => bool,
  hasExited: unit => option(int),
  wait: (~name: string=?, ~timeout: float=?, unit => bool) => unit,
};

let run = (~parentPid=?, ~name, f) => {
  if (Sys.win32) {
    Timber.App.disableColors();
  };

  Oni_Core.Log.enableDebug();
  Revery.App.initConsole();
  module Log = (val Log.withNamespace(name));

  Timber.App.enable();
  Timber.App.setLevel(Timber.Level.debug);
  Log.info("Starting test: " ++ name);
  let connected = ref(false);
  let onConnected = () => {
    Log.info("Connected to syntax server");
    connected := true;
  };

  let wait = (~name="TODO", ~timeout=1.0, f) => {
    let wrappedF = () => {
      for(_ in 0 to 100) {
        ignore(Luv.Loop.run(~mode=`NOWAIT, ()): bool);
      }

      f();
    };

    ThreadEx.waitForCondition(~timeout, wrappedF);
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
      LanguageInfo.initial,
      Setup.default(),
    )
    |> Result.get_ok;

  let isConnected = () => connected^;
  let hasExited = () => exitCode^;
  f({syntaxClient, isConnected, hasExited, wait});
  Log.info("== PASSED ==");
};
