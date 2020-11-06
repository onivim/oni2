open Oni_Core;
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
           id == "test" && label == "Test"
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
         | Msg.SCM(RegisterSCMResourceGroups({groups, _})) =>
           groups
           |> List.exists(({id, label, _}: Exthost.SCM.Group.t) => {
                id == "resourceGroup" && label == "resourceGroup"
              })
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
  test("provideOriginalResource returns URI", _ => {
    let testUri = Uri.fromPath("a-test-file.txt");
    Test.startWithExtensions(["oni-scm"])
    |> Test.waitForExtensionActivation("oni-scm")
    |> Test.withClientRequest(
         ~name="provideOriginalResource",
         ~validate=maybeUri => maybeUri == Some(testUri),
         Exthost.Request.SCM.provideOriginalResource(~handle=0, ~uri=testUri),
       )
    |> Test.terminate
    |> Test.waitForProcessClosed;
  });
});
