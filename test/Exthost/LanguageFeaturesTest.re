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
  });
  describe("definition", ({test, _}) => {
    test("gets definition", ({expect, _}) => {
      let addedDelta =
        DocumentsAndEditorsDelta.create(
          ~removedDocuments=[],
          ~addedDocuments=[model(~lines=["Hello", "World"])],
        );
      let definitionHandle = ref(-1);

      let getDefinition = client =>
        Request.LanguageFeatures.provideDefinition(
          ~handle=definitionHandle^,
          ~resource=testUri,
          ~position=OneBasedPosition.{lineNumber: 1, column: 1},
          client,
        );

      let waitForRegisterDefinitionSupport =
        fun
        | Msg.LanguageFeatures(RegisterDefinitionSupport({handle, _})) => {
            definitionHandle := handle;
            true;
          }
        | _ => false;

      Test.startWithExtensions(["oni-language-features"])
      |> Test.waitForReady
      |> Test.waitForExtensionActivation("oni-language-features")
      |> Test.waitForMessage(
           ~name="RegisterDefinitionSupport",
           waitForRegisterDefinitionSupport,
         )
      |> Test.withClient(
           Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
             ~delta=addedDelta,
           ),
         )
      |> Test.withClientRequest(
           ~name="Get definition",
           ~validate=
             (definitions: list(Exthost.DefinitionLink.t)) => {
               expect.int(List.length(definitions)).toBe(1);
               let definition = List.hd(definitions);
               let {range, _}: Exthost.DefinitionLink.t = definition;

               expect.int(range.startLineNumber).toBe(1);
               expect.int(range.endLineNumber).toBe(1);
               expect.int(range.startColumn).toBe(1);
               expect.int(range.endColumn).toBe(1);
               true;
             },
           getDefinition,
         )
      |> Test.validateNoPendingRequests
      |> Test.terminate
      |> Test.waitForProcessClosed;
    })
  });
  describe("declaration", ({test, _}) => {
    test("gets declaration", ({expect, _}) => {
      let addedDelta =
        DocumentsAndEditorsDelta.create(
          ~removedDocuments=[],
          ~addedDocuments=[model(~lines=["Hello", "World"])],
        );
      let declarationHandle = ref(-1);

      let getDeclaration = client =>
        Request.LanguageFeatures.provideDeclaration(
          ~handle=declarationHandle^,
          ~resource=testUri,
          ~position=OneBasedPosition.{lineNumber: 2, column: 2},
          client,
        );

      let waitForRegisterDeclarationSupport =
        fun
        | Msg.LanguageFeatures(RegisterDeclarationSupport({handle, _})) => {
            declarationHandle := handle;
            true;
          }
        | _ => false;

      Test.startWithExtensions(["oni-language-features"])
      |> Test.waitForReady
      |> Test.waitForExtensionActivation("oni-language-features")
      |> Test.waitForMessage(
           ~name="RegisterDeclarationSupport",
           waitForRegisterDeclarationSupport,
         )
      |> Test.withClient(
           Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
             ~delta=addedDelta,
           ),
         )
      |> Test.withClientRequest(
           ~name="Get declaration",
           ~validate=
             (declarations: list(Exthost.DefinitionLink.t)) => {
               expect.int(List.length(declarations)).toBe(1);
               let definition = List.hd(declarations);
               let {range, _}: Exthost.DefinitionLink.t = definition;

               expect.int(range.startLineNumber).toBe(2);
               expect.int(range.endLineNumber).toBe(2);
               expect.int(range.startColumn).toBe(2);
               expect.int(range.endColumn).toBe(2);
               true;
             },
           getDeclaration,
         )
      |> Test.validateNoPendingRequests
      |> Test.terminate
      |> Test.waitForProcessClosed;
    })
  });
});
