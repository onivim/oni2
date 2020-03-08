open Oni_Core;
open Utility;

module Log = (val Log.withNamespace("Oni2_editor.HealthCheck"));

type checks =
  | Common
  | All;

let commonChecks = [
  (
    "Verify oniguruma dependency",
    _ => {
      Oniguruma.(
        {
          OnigRegExp.create("(@selector\\()(.*?)(\\))")
          |> Stdlib.Result.map(
               OnigRegExp.search("@selector(windowWillClose:)", 0),
             )
          |> Stdlib.Result.map(result => {
               OnigRegExp.(
                 Match.getText(result[1]) == "@selector("
                 && Match.getText(result[3]) == ")"
               )
             })
          |> Stdlib.Result.value(~default=false);
        }
      );
    },
  ),
  (
    "Verify PATH is available",
    _ => {
      let path = ShellUtility.getShellPath();
      Log.info("Got PATH: " ++ path);
      true;
    },
  ),
  (
    "Verify textmate dependency",
    _ => {
      Textmate.(
        {
          let matchRegex = RegExpFactory.create("a|b|c");
          let grammar =
            Grammar.create(
              ~scopeName="source.abc",
              ~patterns=[
                Match({
                  matchRegex,
                  matchName: Some("keyword.letter"),
                  captures: [],
                }),
              ],
              ~repository=[],
              (),
            );

          let grammarRepository = _ => None;
          let (tokens, _) =
            Grammar.tokenize(~grammarRepository, ~grammar, "a");
          List.length(tokens) > 0;
        }
      );
    },
  ),
];

let mainChecks = [
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
      |> LwtEx.sync
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
      Sys.file_exists(
        Revery.Environment.executingDirectory ++ "FiraCode-Regular.ttf",
      ),
  ),
  (
    "Revery: Verify can measure & shape font",
    _ => {
      let fontPath =
        Revery.Environment.executingDirectory ++ "FiraCode-Regular.ttf";
      switch (Revery.Font.load(fontPath)) {
      | Ok(font) =>
        let metrics = Revery.Font.getMetrics(font, 12.0);
        ignore(metrics);

        let {height, width}: Revery.Font.measureResult =
          Revery.Font.measure(
            ~smoothing=Revery.Font.Smoothing.default,
            font,
            12.0,
            "hello",
          );
        Log.infof(m =>
          m("Measurements - width: %f height: %f", width, height)
        );

        let shapeResult = Revery.Font.shape(font, "abc => def");
        let glyphCount = Revery.Font.ShapeResult.size(shapeResult);
        Log.infof(m => m("Shaped glyphs: %d", glyphCount));
        true;
      | Error(msg) =>
        Log.error(msg);
        false;
      };
    },
  ),
  (
    "SDl2: Verify version",
    _ => {
      let compiledVersion = Sdl2.Version.getCompiled();
      let linkedVersion = Sdl2.Version.getLinked();

      Log.info(
        "SDL2 - compiled version: " ++ Sdl2.Version.toString(compiledVersion),
      );
      Log.info(
        "SDL2 - linked version: " ++ Sdl2.Version.toString(linkedVersion),
      );

      (
        compiledVersion.major == 2
        && compiledVersion.minor >= 0
        && compiledVersion.patch >= 10
      )
      && linkedVersion.major == 2
      && linkedVersion.minor >= 0
      && linkedVersion.patch >= 10;
    },
  ),
  (
    "Verify bundled reason-language-server executable",
    (setup: Setup.t) => {
      let ret = Rench.ChildProcess.spawnSync(setup.rlsPath, [|"--help"|]);

      ret.stdout |> String.trim |> StringEx.contains("Reason Language Server");
    },
  ),
  (
    "Verify bundled syntax server",
    (setup: Setup.t) => {
      let connected = ref(false);
      let closed = ref(false);
      let healthCheckResult = ref(false);
      let syntaxClient =
        Oni_Syntax_Client.start(
          ~scheduler=Scheduler.immediate,
          ~onConnected=() => connected := true,
          ~onClose=_ => closed := true,
          ~onHighlights=_ => (),
          ~onHealthCheckResult=res => healthCheckResult := res,
          Oni_Extensions.LanguageInfo.initial,
          setup,
        );

      let waitForRef = (condition: ref(bool)) => {
        let tries = ref(0);
        let waitThread: Thread.t =
          ThreadHelper.create(
            ~name="HealthCheck.waitThread",
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

      // Run health check for syntax server
      Oni_Syntax_Client.healthCheck(syntaxClient);

      waitForRef(healthCheckResult);

      // Verify we are able to close it
      Oni_Syntax_Client.close(syntaxClient);
      waitForRef(closed);

      connected^ && closed^;
    },
  ),
];

let run = (~checks, _cli) => {
  let setup = Setup.init();

  let checks =
    switch (checks) {
    | All => commonChecks @ mainChecks
    | Common => commonChecks
    };

  let passed =
    List.fold_left(
      (acc, (name, f)) => {
        Log.info("RUNNING CHECK: " ++ name);
        let passed = f(setup);
        Log.infof(m => m(" -- RESULT: %s", passed ? "PASS" : "FAIL"));
        acc && passed;
      },
      true,
      checks,
    );

  Log.info("");

  if (passed) {
    Log.info("** PASSED **");
  } else {
    Log.info("** FAILED **");
  };

  Log.info("");

  Log.info("All systems go.");
  Log.info("Checking for remaining threads...");
  ThreadHelper.showRunningThreads() |> Log.info;
  passed ? 0 : 1;
};
