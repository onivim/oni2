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
    "Verify node executable can execute simple script",
    (setup: Setup.t) => {
      let ret =
        Rench.ChildProcess.spawnSync(
          setup.nodePath,
          [|"-e", "console.log(\"test\")"|],
        );
      ret.stdout |> String.trim |> String.equal("test");
    },
  ),
  (
    "Verify node dependencies",
    (setup: Setup.t) => {
      let ret =
        Rench.ChildProcess.spawnSync(
          setup.nodePath,
          [|Setup.getNodeHealthCheckPath(setup)|],
        );
      ret.stdout |> String.trim |> String.equal("Success!");
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
      let ret =
        Rench.ChildProcess.spawnSync(
          setup.rlsPath,
          [|"--help"|],
        );
      
        ret.stdout 
        |> String.trim 
        |> Utility.StringUtil.contains("Reason Language Server");
  }),
];

let run = () => {
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
  exit(result ? 0 : 1);
};
