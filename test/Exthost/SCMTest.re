open TestFramework;

open Exthost;

describe("SCM", ({test, _}) => {
  test("oni-scm extension activates", _ => {
    Test.startWithExtensions(["oni-scm"])
    |> Test.waitForExtensionActivation("oni-scm")
    |> Test.terminate
    |> Test.waitForProcessClosed
  });

  test("gets RegisterSourceControl message", _ => {
    Test.startWithExtensions(["oni-scm"])
    |> Test.waitForExtensionActivation("oni-scm")
    |> Test.waitForMessage(
         ~name="RegisterSourceControl",
         fun
         | Msg.SCM(RegisterSourceControl({id, label, _})) =>
           id == "\"test\"" && label == "\"Test\""
         | _ => false,
       )
    |> Test.terminate
    |> Test.waitForProcessClosed
  });

  test("gets RegisterSCMResourceGroup message", _ => {
    Test.startWithExtensions(["oni-scm"])
    |> Test.waitForExtensionActivation("oni-scm")
    |> Test.waitForMessage(
         ~name="RegisterSCMResourceGroup",
         fun
         | Msg.SCM(RegisterSCMResourceGroup({id, label, _})) =>
           id == "resourceGroup" && label == "resourceGroup"
         | _ => false,
       )
    |> Test.terminate
    |> Test.waitForProcessClosed
  });

  test("gets UnregisterSourceControl message after disposing", _ => {
    Test.startWithExtensions(["oni-scm"])
    |> Test.waitForExtensionActivation("oni-scm")
    |> Test.withClient(
         Exthost.Request.Commands.executeContributedCommand(
           ~arguments=[],
           ~command="testSCM.dispose",
         ),
       )
    |> Test.waitForMessage(
         ~name="UnregisterSourceControl",
         fun
         | Msg.SCM(UnregisterSourceControl(_)) => true
         | _ => false,
       )
    |> Test.terminate
    |> Test.waitForProcessClosed
  });
  test("gets UnregisterSCMResourceGroup message after disposing", _ => {
    Test.startWithExtensions(["oni-scm"])
    |> Test.waitForExtensionActivation("oni-scm")
    |> Test.withClient(
         Exthost.Request.Commands.executeContributedCommand(
           ~arguments=[],
           ~command="testSCM.dispose",
         ),
       )
    |> Test.waitForMessage(
         ~name="UnregisterSCMResourceGroup",
         fun
         | Msg.SCM(UnregisterSCMResourceGroup(_)) => true
         | _ => false,
       )
    |> Test.terminate
    |> Test.waitForProcessClosed
  });
});
