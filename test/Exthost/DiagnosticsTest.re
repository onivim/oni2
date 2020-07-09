open TestFramework;

open Exthost;

describe("DiagnosticsTest", ({test, _}) => {
  test("gets diagnostic message", _ => {
    let waitForActivation =
      fun
      | Msg.ExtensionService(DidActivateExtension({extensionId, _})) =>
        extensionId == "oni-diagnostics"
      | _ => false;

    let waitForChangeMany =
      fun
      | Msg.Diagnostics(ChangeMany({owner, entries})) =>
        owner == "diags" && List.length(entries) == 1
      | _ => false;

    let waitForClear =
      fun
      | Msg.Diagnostics(Clear({owner})) => owner == "diags"
      | _ => false;

    Test.startWithExtensions(["oni-diagnostics"])
    |> Test.waitForReady
    |> Test.waitForMessage(~name="Activation", waitForActivation)
    |> Test.waitForMessage(~name="Diagnostics$changeMany", waitForChangeMany)
    |> Test.waitForMessage(~name="Diagnostics$clear", waitForClear)
    |> Test.terminate
    |> Test.waitForProcessClosed;
  })
});
