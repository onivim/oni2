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
  describe("deltaExtensions", ({test, _}) => {
    test("wildcard", _
      => {
        let extensionToAdd = Test.getExtensionManifest("oni-always-activate");
        let waitForActivation =
          fun
          | Msg.ExtensionService(DidActivateExtension({extensionId, _})) =>
            extensionId == "oni-always-activate"
          | _ => false;

        Test.startWithExtensions([])
        |> Test.waitForReady
        |> Test.waitForInitialized
        |> Test.withClientRequest(
             ~name="$deltaExtensions: add oni-always-activate",
             ~validate=
               () => {
                 prerr_endline("GOT MESSAGE");
                 true;
               },
             client => {
               prerr_endline("SENDING MESSAGE!");
               Exthost.Request.ExtensionService.deltaExtensions(
                 ~toAdd=[extensionToAdd],
                 ~toRemove=[],
                 client,
               );
             },
           )
        |> Test.activate(
             ~extensionId="oni-always-activate",
             ~reason=
               Exthost.ExtensionActivationReason.create(
                 ~extensionId="oni-always-activate",
                 ~startup=true,
                 ~activationEvent="*",
               ),
           )
        |> Test.waitForMessage(~name="Activation", waitForActivation)
        |> Test.terminate
        |> Test.waitForProcessClosed;
      })
      //    test("onLanguage:testlang", _ => {
      //      let extensionToAdd = Test.getExtensionManifest("oni-activation-events");
      //      let waitForActivation =
      //        fun
      //        | Msg.ExtensionService(DidActivateExtension({extensionId, _})) =>
      //          extensionId == "oni-activation-events"
      //        | _ => false;
      //
      //      Test.startWithExtensions([])
      //      |> Test.waitForReady
      //      |> Test.waitForInitialized
      //      |> Test.withClientRequest(
      //        ~name="$deltaExtensions: add oni-activation-events",
      //        ~validate=() => {
      //          prerr_endline ("GOT MESSAGE");
      //          true;
      //        },
      //        (client) => {
      //          prerr_endline ("SENDING MESSAGE!");
      //          Exthost.Request.ExtensionService.deltaExtensions(
      //            ~toAdd=[extensionToAdd],
      //            ~toRemove=[],
      //            client
      //          );
      //        }
      //      )
      //      |> Test.activateByEvent(~event="onLanguage:testlang")
      //      |> Test.waitForMessage(~name="Activation", waitForActivation)
      //      |> Test.terminate
      //      |> Test.waitForProcessClosed;
      //    })
  });
});
