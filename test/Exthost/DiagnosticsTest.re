open TestFramework;

open Exthost;

describe("DiagnosticsTest", ({test, _}) => {
    test("gets diagnostic message", _ => {
      let waitForActivation =
        fun
        | Msg.ExtensionService(DidActivateExtension({extensionId, _})) =>
          extensionId == "oni-diagnostics"
        | _ => false;

      Test.startWithExtensions(["oni-diagnostics"])
      |> Test.waitForReady
      |> Test.waitForMessage(~name="Activation", waitForActivation)
      |> Test.terminate
      |> Test.waitForProcessClosed;
    })
});
