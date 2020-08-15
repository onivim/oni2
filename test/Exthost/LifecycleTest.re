open TestFramework;

describe("LifecycleTest", ({test, _}) => {
  test("close - no extensions", _ => {
    Test.startWithExtensions([])
    |> Test.waitForReady
    |> Test.terminate
    |> Test.waitForProcessClosed
  });
  test("close - extensions", _ => {
    Test.startWithExtensions(["oni-always-activate"])
    |> Test.waitForReady
    |> Test.terminate
    |> Test.waitForProcessClosed
  });
  test("close - no parent process", _ => {
    // Extension host process close automatically,
    // if parent process is gone.
    let dummyProcess = Sys.win32 ? "cmd.exe" : "sh";

    let dummyPid =
      Luv.Process.spawn(dummyProcess, ["echo '1'"])
      |> Result.map(Luv.Process.pid)
      |> Result.get_ok;

    Test.startWithExtensions(~pid=dummyPid, ["oni-always-activate"])
    |> Test.waitForReady
    |> Test.waitForProcessClosed;
  });
});
