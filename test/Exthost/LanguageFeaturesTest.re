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

let addedDelta =
  DocumentsAndEditorsDelta.create(
    ~removedDocuments=[],
    ~addedDocuments=[model(~lines=["Hello", "World"])],
  );


describe("LanguageFeaturesTest", ({describe, _}) => {

  let startTest = () => {
      Test.startWithExtensions(["oni-language-features"])
      |> Test.waitForReady
      |> Test.waitForExtensionActivation("oni-language-features")
  };

  describe("completion", ({test, _}) => {
    test("gets completion items", ({expect, _}) => {
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

      startTest()
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

      startTest()
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
             (definitions: list(Exthost.Location.t)) => {
               expect.int(List.length(definitions)).toBe(1);
               let definition = List.hd(definitions);
               let {range, _}: Exthost.Location.t = definition;

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

      startTest()
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
             (declarations: list(Exthost.Location.t)) => {
               expect.int(List.length(declarations)).toBe(1);
               let definition = List.hd(declarations);
               let {range, _}: Exthost.Location.t = definition;

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
  describe("highlights", ({test, _}) => {
    test("gets highlights", ({expect, _}) => {
      let highlightsHandle = ref(-1);

      let getHighlights = client =>
        Request.LanguageFeatures.provideDocumentHighlights(
          ~handle=highlightsHandle^,
          ~resource=testUri,
          ~position=OneBasedPosition.{lineNumber: 2, column: 2},
          client,
        );

      let waitForRegisterDocumentHighlightProvider =
        fun
        | Msg.LanguageFeatures(
            RegisterDocumentHighlightProvider({handle, _}),
          ) => {
            highlightsHandle := handle;
            true;
          }
        | _ => false;

      startTest()
      |> Test.waitForMessage(
           ~name="RegisterHighlightProvider",
           waitForRegisterDocumentHighlightProvider,
         )
      |> Test.withClient(
           Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
             ~delta=addedDelta,
           ),
         )
      |> Test.withClientRequest(
           ~name="Get highlights",
           ~validate=
             (highlights: list(Exthost.DocumentHighlight.t)) => {
               expect.int(List.length(highlights)).toBe(2);
               let highlight0: Exthost.DocumentHighlight.t =
                 List.nth(highlights, 0);
               let highlight1: Exthost.DocumentHighlight.t =
                 List.nth(highlights, 1);

               expect.equal(
                 highlight0.kind,
                 Exthost.DocumentHighlight.Kind.Text,
               );
               expect.equal(
                 highlight1.kind,
                 Exthost.DocumentHighlight.Kind.Text,
               );

               expect.int(highlight0.range.startLineNumber).toBe(2);
               expect.int(highlight0.range.endLineNumber).toBe(4);
               expect.int(highlight0.range.startColumn).toBe(3);
               expect.int(highlight0.range.endColumn).toBe(5);

               expect.int(highlight1.range.startLineNumber).toBe(6);
               expect.int(highlight1.range.endLineNumber).toBe(8);
               expect.int(highlight1.range.startColumn).toBe(7);
               expect.int(highlight1.range.endColumn).toBe(9);
               true;
             },
           getHighlights,
         )
      |> Test.validateNoPendingRequests
      |> Test.terminate
      |> Test.waitForProcessClosed;
    })
  });
  describe("references", ({test, _}) => {
    test("gets references", ({expect, _}) => {
      let referencesHandle = ref(-1);

      let getReferences = client =>
        Request.LanguageFeatures.provideReferences(
          ~handle=referencesHandle^,
          ~resource=testUri,
          ~position=OneBasedPosition.{lineNumber: 2, column: 2},
          ~context=Exthost.ReferenceContext.{includeDeclaration: true},
          client,
        );

      let waitForRegisterReferencesProvider =
        fun
        | Msg.LanguageFeatures(RegisterReferenceSupport({handle, _})) => {
            referencesHandle := handle;
            true;
          }
        | _ => false;

      startTest()
      |> Test.waitForMessage(
           ~name="RegisterReferencesProvider",
           waitForRegisterReferencesProvider,
         )
      |> Test.withClient(
           Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
             ~delta=addedDelta,
           ),
         )
      |> Test.withClientRequest(
           ~name="Get highlights",
           ~validate=
             (references: list(Exthost.Location.t)) => {
               expect.int(List.length(references)).toBe(1);
               let location: Exthost.Location.t = List.nth(references, 0);

               expect.int(location.range.startLineNumber).toBe(2);
               expect.int(location.range.endLineNumber).toBe(4);
               expect.int(location.range.startColumn).toBe(3);
               expect.int(location.range.endColumn).toBe(5);
               true;
             },
           getReferences,
         )
      |> Test.validateNoPendingRequests
      |> Test.terminate
      |> Test.waitForProcessClosed;
    })
  });
});
