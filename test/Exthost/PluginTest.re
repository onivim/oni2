open TestFramework;

open Exthost;

describe("PluginTest", ({test, _}) => {
    test("oni-dev-extension", _ => {
      let waitForActivation =
        fun
        | Msg.ExtensionService(DidActivateExtension({extensionId, _})) =>
          extensionId == "oni-dev-extension"
        | _ => false;

      Test.startWithExtensions(
      ~rootPath=Rench.Path.join(Sys.getcwd(), "development_extensions"),
      ["oni-dev-extension"])
      |> Test.waitForReady
      |> Test.waitForMessage(~name="Activation", waitForActivation)
      |> Test.terminate
      |> Test.waitForProcessClosed;
    })
});
