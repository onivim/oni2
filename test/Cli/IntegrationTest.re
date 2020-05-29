open TestFramework;

let developmentExtensions =
  Rench.Path.join(Sys.getcwd(), "development_extensions");

describe("CLI Integration Tests", ({describe, _}) => {
  describe("launcher", ({test, _}) => {
    test("--version should show output", _ => {
      TestRunner.(
        startWithArgs(["--version"])
        |> validateOutputContains("Onivim 2")
        |> validateExitStatus(WEXITED(0))
        |> finish
      )
    });

    test("--list-extensions should show extensions", _ => {
      TestRunner.(
        startWithArgs([
          "--list-extensions",
          "--extensions-dir=" ++ developmentExtensions,
        ])
        |> validateOutputContains("dev-extension")
        |> validateExitStatus(WEXITED(0))
        |> finish
      )
    });
  });
  describe("editor", ({test, _}) => {
    test("rogue -psn argument shouldn't cause a failure", _ => {
      TestRunner.(
        startEditorWithArgs(["-psn_0_989382", "--list-extensions"])
        |> validateExitStatus(WEXITED(0))
        |> finish
      )
    })
  });
});
