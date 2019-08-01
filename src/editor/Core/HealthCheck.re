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
    "Verify camomile:charmpdir",
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
      let ret = Rench.ChildProcess.spawnSync(setup.nodePath, [|"-e", "console.log(\"test\")"|]);
      ret.stdout
      |> String.trim
      |> String.equal("test");
    }
  ),
  (
    "Verify rg executable",
    (setup: Setup.t) => Sys.file_exists(setup.rgPath),
  ),
  (
    "Verify textmate service exists",
    (setup: Setup.t) => Sys.file_exists(setup.textmateServicePath),
  ),
  (
    "Verify bundled extensions exists",
    (setup: Setup.t) => Sys.is_directory(setup.bundledExtensionsPath),
  ),
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
