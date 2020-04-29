open TestFramework;

open Exthost;

describe("StatusBar", ({test, _}) => {
  test("receives status bar message on activation", _ => {
    let waitForStatusBar =
      fun
      | Msg.StatusBar(SetEntry({text, _})) => {
          String.equal(text, "\"Hello!\"");
        }
      | _ => false;

    Test.startWithExtensions(["oni-basic-events"])
    |> Test.waitForReady
    |> Test.waitForMessage(~name="StatusBarSetEntry", waitForStatusBar)
    |> Test.terminate
    |> Test.waitForProcessClosed;
  })
});
