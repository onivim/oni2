open Oni_Core;

let checks = [
  (
    "Verify camomile:datadir",
    _ => Sys.is_directory(CamomileBundled.LocalConfig.datadir),
  ),
  (
    "Verify camomile:localedir",
    _ => Sys.is_directory(CamomileBundled.LocalConfig.localedir),
  ),
  (
    "Verify camomile:charmapdir",
    _ => Sys.is_directory(CamomileBundled.LocalConfig.charmapdir),
  ),
  (
    "Verify camomile:unimapdir",
    _ => Sys.is_directory(CamomileBundled.LocalConfig.unimapdir),
  ),
  (
    "Verify node executable",
    (setup: Setup.t) => Sys.file_exists(setup.nodePath),
  ),
  (
    "Verify node dependencies",
    (setup: Setup.t) => {
      Oni_Extensions.NodeTask.run(
        ~scheduler=Scheduler.immediate,
        ~setup,
        "check-health.js",
      )
      |> Utility.LwtUtil.sync
      |> (
        fun
        | Ok(_) => true
        | Error(_) => false
      );
    },
  ),
  (
    "Verify ripgrep (rg) executable",
    (setup: Setup.t) => Sys.file_exists(setup.rgPath),
  ),
  (
    "Verify bundled extensions path exists",
    (setup: Setup.t) => Sys.is_directory(setup.bundledExtensionsPath),
  ),
  (
    "Verify bundled font exists",
    _ =>
      Sys.file_exists(Utility.executingDirectory ++ "FiraCode-Regular.ttf"),
  ),
  (
    "Verify bundled reason-language-server executable",
    (setup: Setup.t) => {
      let ret = Rench.ChildProcess.spawnSync(setup.rlsPath, [|"--help"|]);

      ret.stdout
      |> String.trim
      |> Utility.StringUtil.contains("Reason Language Server");
    },
  ),
  (
    "Verify bundled syntax server",
    (setup: Setup.t) => {
      let connected = ref(false);
      let closed = ref(false);
      let syntaxClient =
        Oni_Syntax_Client.start(
          ~scheduler=Scheduler.immediate,
          ~onConnected=() => connected := true,
          ~onClose=_ => closed := true,
          ~onHighlights=_ => (),
          Oni_Extensions.LanguageInfo.initial,
          setup,
        );

      let waitForRef = (condition: ref(bool)) => {
        let tries = ref(0);
        let waitThread: Thread.t =
          Thread.create(
            () => {
              while (condition^ == false && tries^ < 10) {
                Unix.sleepf(0.2);
                incr(tries);
              }
            },
            (),
          );

        Thread.join(waitThread);
      };

      // Verify the syntax client spins up and emits a connection message
      waitForRef(connected);

      // Verify we are able to close it
      Oni_Syntax_Client.close(syntaxClient);
      waitForRef(closed);

      connected^ && closed^;
    },
  ),
];

let run = _cli => {
  let setup = Setup.init();

  let result =
    List.fold_left(
      (prev, curr) => {
        let (name, f) = curr;
        Log.info("RUNNING CHECK: " ++ name);
        let result = f(setup);
        let resultString = result ? "PASS" : "FAIL";
        Log.info(" -- RESULT: " ++ resultString);
        prev && result;
      },
      true,
      checks,
    );

  Log.info("");

  result ? Log.info("** PASSED **") : Log.info("** FAILED **");
  Log.info("");

  Log.info("All systems go.");
  result ? 0 : 1;
};
