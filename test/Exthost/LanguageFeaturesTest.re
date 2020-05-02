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
    test("gets completion items", ({expect, _}) => {
      let addedDelta =
        DocumentsAndEditorsDelta.create(
          ~removedDocuments=[],
          ~addedDocuments=[model(~lines=["Hello", "World"])],
        );
      let suggestHandle = ref(-1);

      let getCompletionItems = client =>
        Request.LanguageFeatures.provideCompletionItems(
          ~handle=suggestHandle^,
          ~resource=testUri,
          ~position=OneBasedPosition.{lineNumber: 1, column: 1},
          ~context=
            CompletionContext.{triggerKind: Invoke, triggerCharacter: None},
          client,
        );

      let waitForRegisterSuggestSupport =
        fun
        | Msg.LanguageFeatures(RegisterSuggestSupport({handle, _})) => {
            suggestHandle := handle;
            true;
          }
        | _ => false;

      Test.startWithExtensions(["oni-language-features"])
      |> Test.waitForReady
      |> Test.waitForExtensionActivation("oni-language-features")
      |> Test.waitForMessage(
           ~name="RegisterSuggestSupport",
           waitForRegisterSuggestSupport,
         )
      |> Test.withClient(
           Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
             ~delta=addedDelta,
           ),
         )
      |> Test.withClientRequest(
           ~name="Get completion items",
           ~validate=
             (suggestResult: Exthost.SuggestResult.t) => {
               let {completions, isIncomplete}: Exthost.SuggestResult.t = suggestResult;
               expect.int(List.length(completions)).toBe(2);
               expect.bool(isIncomplete).toBe(false);

               let firstResult: Exthost.SuggestItem.t =
                 List.nth(completions, 0);
               let secondResult: Exthost.SuggestItem.t =
                 List.nth(completions, 1);

               expect.string(firstResult.label).toEqual("item1");
               expect.string(secondResult.label).toEqual("item2");
               true;
             },
           getCompletionItems,
         )
      |> Test.validateNoPendingRequests
      |> Test.terminate
      |> Test.waitForProcessClosed;
    })
  })
});
