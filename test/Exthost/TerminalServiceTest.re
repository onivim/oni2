open Oni_Core;
open TestFramework;
open Exthost;

let waitForProcessExit =
  fun
  | Msg.TerminalService(Msg.TerminalService.SendProcessExit(_)) => true
  | _ => false;

describe("TerminalServiceTest", ({test, _}) => {
  // With 1.47.1, our no-op process isn't even started, so there is no exit for it.
  //  test("noop process should give process exited", _ => {
  //    Test.startWithExtensions([])
  //    |> Test.waitForReady
  //    |> Test.withClient(
  //         Request.TerminalService.spawnExtHostProcess(
  //           ~id=0,
  //           ~shellLaunchConfig=
  //             ShellLaunchConfig.{
  //               executable: "noop",
  //               arguments: [],
  //               name: "noop",
  //             },
  //           ~activeWorkspaceRoot=Uri.fromPath(Sys.getcwd()),
  //           ~cols=10,
  //           ~rows=10,
  //           ~isWorkspaceShellAllowed=true,
  //         ),
  //       )
  //    |> Test.waitForMessage(
  //         ~name="TerminalService.SendProcessExit",
  //         waitForProcessExit,
  //       )
  //    |> Test.terminate
  //  });
  test("valid process should send data and title", _ => {
    //    |> Test.waitForProcessClosed

    let shellLaunchConfig =
      Sys.win32
        ? ShellLaunchConfig.{
            executable: "cmd.exe",
            name: "Windows Terminal",
            arguments: ["/c", "echo", "hello"],
            env: Inherit,
          }
        : ShellLaunchConfig.{
            executable: "bash",
            name: "Bash Terminal",
            arguments: ["-c", "echo hello"],
            env: Inherit,
          };
    let waitForProcessData =
      fun
      | Msg.TerminalService(Msg.TerminalService.SendProcessData(_)) => true
      | _ => false;

    let waitForProcessTitle =
      fun
      | Msg.TerminalService(Msg.TerminalService.SendProcessTitle(_)) => true
      | _ => false;

    Test.startWithExtensions([])
    |> Test.waitForReady
    |> Test.withClient(
         Request.TerminalService.spawnExtHostProcess(
           ~id=0,
           ~shellLaunchConfig,
           ~activeWorkspaceRoot=Uri.fromPath(Sys.getcwd()),
           ~cols=10,
           ~rows=10,
           ~isWorkspaceShellAllowed=true,
         ),
       )
    |> Test.waitForMessage(
         ~name="TerminalService.SendProcessTitle",
         waitForProcessTitle,
       )
    |> Test.waitForMessage(
         ~name="TerminalService.SendProcessData",
         waitForProcessData,
       )
    |> Test.waitForMessage(
         ~name="TerminalService.SendProcessExit",
         waitForProcessExit,
       )
    |> Test.terminate
    |> Test.waitForProcessClosed;
  })
});
