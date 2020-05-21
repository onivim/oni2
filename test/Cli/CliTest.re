open TestFramework;

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
      })
    })
    //	describe("editor", ({test, _}) => {
    //
    //	});
});
