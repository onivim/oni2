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

    test("-v should show output", _ => {
      TestRunner.(
        startWithArgs(["-v"])
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
  describe("--query-extension", ({test, _}) => {
    test("Query for Java extension", _ => {
      TestRunner.(
        startEditorWithArgs(["--query-extension", "redhat.java"])
        |> validateOutputContains("Language Support for Java")
        |> validateExitStatus(WEXITED(0))
        |> finish
      )
    })
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

  describe("install / uninstall extensions", ({test, _}) => {
    test("install / uninstall extension from file system", _ => {
      TestRunner.(
        {
          let extensionPath =
            Rench.Path.join(
              Sys.getcwd(),
              "test/collateral/markdown-1.0.0.vsix",
            );

          let () =
            startEditorWithArgs(["--install-extension", extensionPath])
            |> validateExitStatus(WEXITED(0))
            |> finish;

          let () =
            startEditorWithArgs(["--list-extensions"])
            |> validateOutputContains("vscode.markdown")
            |> validateExitStatus(WEXITED(0))
            |> finish;

          let () =
            startEditorWithArgs(["--uninstall-extension", "vscode.markdown"])
            |> validateOutputContains("vscode.markdown")
            |> validateExitStatus(WEXITED(0))
            |> finish;

          let () =
            startEditorWithArgs(["--list-extensions"])
            |> validateOutputDoesNotContain("vscode.markdown")
            |> validateExitStatus(WEXITED(0))
            |> finish;
          ();
        }
      )
    })
  });

  describe("ex commands", ({test, _})
    // On Linux Azure CI, this test fails when creating a window -
    // need to find a workaround to allow `SDL_CreateWindow` to succeed on CI machines.
    =>
      if (!Revery.Environment.isLinux) {
        test("run ex commands with '+'", ({expect, _}) => {
          TestRunner.(
            {
              let filePath =
                Rench.Path.join(
                  Sys.getcwd(),
                  "test-" ++ string_of_int(Luv.Pid.getpid()),
                );

              let () =
                startEditorWithArgs([
                  "-f",
                  "+new " ++ filePath,
                  "+norm! oabc",
                  "+xa!",
                ])
                |> validateExitStatus(WEXITED(0))
                |> finish;

              let lines = Oni_Core.Utility.File.readAllLines(filePath);

              expect.equal(lines, ["", "abc"]);
            }
          )
        });
      }
    );
});
