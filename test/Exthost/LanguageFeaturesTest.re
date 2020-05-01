open TestFramework;

open Exthost;

describe("LanguageFeaturesTest", ({describe, _}) => {
  describe("completion", ({test, _}) => {
    test("gets completion items", _ => {
      let waitForActivation =
        fun
        | Msg.ExtensionService(DidActivateExtension({extensionId, _})) =>
          extensionId == "oni-language-features"
        | _ => false;

      Test.startWithExtensions(["oni-language-features"])
      |> Test.waitForReady
      |> Test.waitForMessage(~name="Activation",waitForActivation)
      |> Test.terminate
      |> Test.waitForProcessClosed;
    })
  })
});
