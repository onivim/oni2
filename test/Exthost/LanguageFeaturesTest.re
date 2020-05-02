open TestFramework;

open Oni_Core;
open Exthost;

let testUri = Uri.fromPath("/test/path");

let model = (~lines) =>
  Exthost.ModelAddedDelta.create(
    ~versionId=0,
    ~lines,
    ~eol=Eol.LF,
    ~modeId="plaintext",
    ~isDirty=false,
    testUri,
  );

describe("LanguageFeaturesTest", ({describe, _}) => {
  describe("completion", ({test, _}) => {
    test("gets completion items", _ => {
      let addedDelta =
        DocumentsAndEditorsDelta.create(
          ~removedDocuments=[],
          ~addedDocuments=[model(~lines=["Hello", "World"])],
        );

      let getCompletionItems =
        Request.LanguageFeatures.provideCompletionItems(
          ~handle=0,
          ~resource=testUri,
          ~position=OneBasedPosition.{lineNumber: 1, column: 1},
          ~context=
            CompletionContext.{triggerKind: Invoke, triggerCharacter: None},
        );

      Test.startWithExtensions(["oni-language-features"])
      |> Test.waitForReady
      |> Test.waitForExtensionActivation("oni-language-features")
      |> Test.withClient(
           Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
             ~delta=addedDelta,
           ),
         )
      |> Test.withClientRequest(
           ~name="Get completion items",
           ~validate=_ => true,
           getCompletionItems,
         )
      |> Test.validateNoPendingRequests
      |> Test.terminate
      |> Test.waitForProcessClosed;
    })
  })
});
