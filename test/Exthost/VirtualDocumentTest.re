open TestFramework;

open Oni_Core;
open Exthost;

describe("VirtualDocumentTest", ({test, _}) => {
  test("close - extensions", _ => {
    let waitForActivation =
      fun
      | Msg.ExtensionService(DidActivateExtension({extensionId, _})) =>
        extensionId == "oni-virtual-document"
      | _ => false;

    let handleToUse = ref(-1);
    let schemeToUse = ref("");
    let waitForRegistration =
      fun
      | Msg.DocumentContentProvider(
          RegisterTextContentProvider({handle, scheme}),
        ) => {
          handleToUse := handle;
          schemeToUse := scheme;
          true;
        }
      | _ => false;

    let validateResponse = str => {
      str |> Option.iter(prerr_endline);
      str == Some("virtual document content");
    };
    Test.startWithExtensions(["oni-virtual-document"])
    |> Test.waitForReady
    |> Test.waitForMessage(~name="Activation", waitForActivation)
    |> Test.waitForMessage(~name="Registration", waitForRegistration)
    |> Test.withClientRequest(
         ~name="Get Context",
         ~validate=validateResponse,
         Request.DocumentContentProvider.provideTextDocumentContent(
           ~handle=handleToUse^,
           ~uri=Uri.fromScheme(~scheme=Custom(schemeToUse^), "/some/path"),
         ),
       )
    |> Test.terminate
    |> Test.waitForProcessClosed;
  })
});
