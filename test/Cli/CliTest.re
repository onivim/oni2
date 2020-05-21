open TestFramework;

let developmentExtensions =
  Rench.Path.join(Sys.getcwd(), "development_extensions");

describe("CLI", ({describe, _}) => {
  describe("launcher", ({test, _})
    => {
      test("--version should show output", _ => {
        Test.(
          startWithArgs(["--version"])
          |> validateOutputContains("Onivim 2")
          |> validateExitStatus(WEXITED(0))
          |> finish
        )
      });

      test("--list-extensions should show extensions", _ => {
        Test.(
          startWithArgs([
            "--list-extensions",
            "--extensions-dir=" ++ developmentExtensions,
          ])
          |> validateOutputContains("dev-extension")
          |> validateExitStatus(WEXITED(0))
          |> finish
        )
      });
    })
    //	describe("editor", ({test, _}) => {
    //
    //	});
});
