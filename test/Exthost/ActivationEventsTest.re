open TestFramework;

open Exthost;

describe("ActivationEventsTest", ({describe, _}) => {
  describe("* (wildcard activation)", ({test, _}) => {
    test("close - extensions", _ => {
      let waitForActivation =
        fun
        | Msg.ExtensionService(DidActivateExtension({extensionId, _})) =>
          extensionId == "oni-always-activate"
        | _ => false;

      Test.startWithExtensions(["oni-always-activate"])
      |> Test.waitForReady
      |> Test.waitForMessage(~name="Activation", waitForActivation)
      |> Test.terminate
      |> Test.waitForProcessClosed;
    })
  });
  describe("onCommand", ({test, _}) => {
    test("onCommand:extension.helloWorld", _ => {
      let waitForActivation =
        fun
        | Msg.ExtensionService(DidActivateExtension({extensionId, _})) =>
          extensionId == "oni-activation-events"
        | _ => false;

      Test.startWithExtensions(["oni-activation-events"])
      |> Test.waitForReady
      |> Test.activateByEvent(~event="onCommand:extension.helloWorld")
      |> Test.waitForMessage(~name="Activation", waitForActivation)
      |> Test.terminate
      |> Test.waitForProcessClosed;
    })
  });
  describe("onLanguage", ({test, _}) => {
    test("onLanguage:testlang", _ => {
      let waitForActivation =
        fun
        | Msg.ExtensionService(DidActivateExtension({extensionId, _})) =>
          extensionId == "oni-activation-events"
        | _ => false;

      Test.startWithExtensions(["oni-activation-events"])
      |> Test.waitForReady
      |> Test.activateByEvent(~event="onLanguage:testlang")
      |> Test.waitForMessage(~name="Activation", waitForActivation)
      |> Test.terminate
      |> Test.waitForProcessClosed;
    })
  });
});
