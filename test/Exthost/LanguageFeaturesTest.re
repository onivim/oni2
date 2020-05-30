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
    (),
  );

describe("LanguageFeaturesTest", ({describe, _}) => {
  let startTest = () => {
    Test.startWithExtensions(["oni-language-features"])
    |> Test.waitForReady
    |> Test.waitForExtensionActivation("oni-language-features");
  };

  let finishTest = context => {
    context
    |> Test.validateNoPendingRequests
    |> Test.terminate
    |> Test.waitForProcessClosed;
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
      |> finishTest;
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
      |> finishTest;
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
      |> finishTest;
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
      |> finishTest;
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
      |> finishTest;
    })
  });
  describe("symbols", ({test, _}) => {
    test("get symbols", ({expect, _}) => {
      let symbolsHandle = ref(-1);

      let getSymbols = client =>
        Request.LanguageFeatures.provideDocumentSymbols(
          ~handle=symbolsHandle^,
          ~resource=testUri,
          client,
        );

      let waitForRegisterDocumentSymbolProvider =
        fun
        | Msg.LanguageFeatures(RegisterDocumentSymbolProvider({handle, _})) => {
            symbolsHandle := handle;
            true;
          }
        | _ => false;

      startTest()
      |> Test.waitForMessage(
           ~name="RegisterSymbolProvider",
           waitForRegisterDocumentSymbolProvider,
         )
      |> Test.withClient(
           Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
             ~delta=addedDelta,
           ),
         )
      |> Test.withClientRequest(
           ~name="Get symbols",
           ~validate=
             (symbols: list(Exthost.DocumentSymbol.t)) => {
               expect.int(List.length(symbols)).toBe(2);
               let symbol0: Exthost.DocumentSymbol.t = List.nth(symbols, 0);
               let symbol1: Exthost.DocumentSymbol.t = List.nth(symbols, 1);

               expect.equal(symbol0.name, "symbol1");
               expect.equal(symbol0.kind, Exthost.SymbolKind.File);
               expect.equal(symbol1.name, "symbol2");
               expect.equal(symbol1.kind, Exthost.SymbolKind.TypeParameter);

               true;
             },
           getSymbols,
         )
      |> finishTest;
    })
  });
  describe("hover", ({test, _}) => {
    test("gets hover", ({expect, _}) => {
      let hoverHandle = ref(-1);

      let getHover = client =>
        Request.LanguageFeatures.provideHover(
          ~handle=hoverHandle^,
          ~resource=testUri,
          ~position=OneBasedPosition.{lineNumber: 2, column: 2},
          client,
        );

      let waitForRegisterHoverProvider =
        fun
        | Msg.LanguageFeatures(RegisterHoverProvider({handle, _})) => {
            hoverHandle := handle;
            true;
          }
        | _ => false;

      startTest()
      |> Test.waitForMessage(
           ~name="RegisterHoverProvider",
           waitForRegisterHoverProvider,
         )
      |> Test.withClient(
           Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
             ~delta=addedDelta,
           ),
         )
      |> Test.withClientRequest(
           ~name="Get symbols",
           ~validate=
             (maybeHover: option(Exthost.Hover.t)) => {
               expect.equal(
                 maybeHover,
                 Some({
                   contents: ["Hover Content"],
                   range:
                     Some(
                       OneBasedRange.{
                         startLineNumber: 2,
                         endLineNumber: 2,
                         startColumn: 1,
                         endColumn: 6,
                       },
                     ),
                 }),
               );

               true;
             },
           getHover,
         )
      |> finishTest;
    })
  });
  describe("signature help", ({test, _}) => {
    test("get signature help", ({expect, _}) => {
      let signatureHelpHandle = ref(-1);

      let getSignatureHelp = client =>
        Request.LanguageFeatures.provideSignatureHelp(
          ~handle=signatureHelpHandle^,
          ~position=OneBasedPosition.{lineNumber: 2, column: 2},
          ~resource=testUri,
          ~context=
            SignatureHelp.RequestContext.{
              triggerKind: SignatureHelp.TriggerKind.Invoke,
              triggerCharacter: None,
              isRetrigger: false,
            },
          client,
        );

      let waitForRegisterSignatureHelpProvider =
        fun
        | Msg.LanguageFeatures(
            RegisterSignatureHelpProvider({handle, metadata, _}),
          ) => {
            signatureHelpHandle := handle;
            expect.equal(metadata.triggerCharacters, ["("]);
            expect.equal(metadata.retriggerCharacters, [","]);
            true;
          }
        | _ => false;

      startTest()
      |> Test.waitForMessage(
           ~name="RegisterSignatureHelpProvider",
           waitForRegisterSignatureHelpProvider,
         )
      |> Test.withClient(
           Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
             ~delta=addedDelta,
           ),
         )
      |> Test.withClientRequest(
           ~name="Get signature help",
           ~validate=
             (signatureHelp: option(Exthost.SignatureHelp.Response.t)) => {
               open Exthost.SignatureHelp;

               expect.equal(
                 signatureHelp,
                 Some({
                   id: 1,
                   signatures: [
                     Signature.{
                       label: "signature 1",
                       parameters: [
                         ParameterInformation.{label: "parameter 1"},
                       ],
                     },
                   ],
                   activeSignature: 0,
                   activeParameter: 0,
                 }),
               );

               true;
             },
           getSignatureHelp,
         )
      |> finishTest;
    })
  });
  describe("formatting", ({test, _}) => {
    let formattingOptions =
      FormattingOptions.{tabSize: 2, insertSpaces: true};

    let expectedRange =
      OneBasedRange.{
        startLineNumber: 2,
        endLineNumber: 4,
        startColumn: 3,
        endColumn: 5,
      };

    test("document formatting", ({expect, _}) => {
      let documentFormattingHandle = ref(-1);

      let getDocumentFormatEdits = client =>
        Request.LanguageFeatures.provideDocumentFormattingEdits(
          ~handle=documentFormattingHandle^,
          ~resource=testUri,
          ~options=formattingOptions,
          client,
        );

      let waitForRegisterDocumentFormattingSupport =
        fun
        | Msg.LanguageFeatures(
            RegisterDocumentFormattingSupport({handle, _}),
          ) => {
            documentFormattingHandle := handle;
            true;
          }
        | _ => false;

      startTest()
      |> Test.waitForMessage(
           ~name="RegisterDocumentFormattingSupport",
           waitForRegisterDocumentFormattingSupport,
         )
      |> Test.withClient(
           Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
             ~delta=addedDelta,
           ),
         )
      |> Test.withClientRequest(
           ~name="get document formatting",
           ~validate=
             (edits: option(list(Edit.SingleEditOperation.t))) => {
               expect.equal(
                 edits,
                 Some([
                   Edit.SingleEditOperation.{
                     range: expectedRange,
                     text: Some("document"),
                     forceMoveMarkers: false,
                   },
                 ]),
               );

               true;
             },
           getDocumentFormatEdits,
         )
      |> finishTest;
    });
    test("range formatting", ({expect, _}) => {
      let documentRangeFormattingHandle = ref(-1);

      let getDocumentRangeFormattingEdits = client =>
        Request.LanguageFeatures.provideDocumentRangeFormattingEdits(
          ~handle=documentRangeFormattingHandle^,
          ~range=expectedRange,
          ~resource=testUri,
          ~options=formattingOptions,
          client,
        );

      let waitForRegisterRangeFormattingSupport =
        fun
        | Msg.LanguageFeatures(RegisterRangeFormattingSupport({handle, _})) => {
            documentRangeFormattingHandle := handle;
            true;
          }
        | _ => false;

      startTest()
      |> Test.waitForMessage(
           ~name="RegisterRangeFormattingSupport",
           waitForRegisterRangeFormattingSupport,
         )
      |> Test.withClient(
           Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
             ~delta=addedDelta,
           ),
         )
      |> Test.withClientRequest(
           ~name="get document range formatting",
           ~validate=
             (edits: option(list(Edit.SingleEditOperation.t))) => {
               expect.equal(
                 edits,
                 Some([
                   Edit.SingleEditOperation.{
                     range: expectedRange,
                     text: Some("range"),
                     forceMoveMarkers: false,
                   },
                 ]),
               );

               true;
             },
           getDocumentRangeFormattingEdits,
         )
      |> finishTest;
    });
    test("on type formatting", ({expect, _}) => {
      let onTypeFormattingHandle = ref(-1);

      let getOnTypeFormattingEdits = client =>
        Request.LanguageFeatures.provideOnTypeFormattingEdits(
          ~handle=onTypeFormattingHandle^,
          ~resource=testUri,
          ~position=OneBasedPosition.{lineNumber: 1, column: 1},
          ~character="{",
          ~options=formattingOptions,
          client,
        );

      let waitForRegisterOnTypeFormattingSupport =
        fun
        | Msg.LanguageFeatures(RegisterOnTypeFormattingSupport({handle, _})) => {
            onTypeFormattingHandle := handle;
            true;
          }
        | _ => false;

      startTest()
      |> Test.waitForMessage(
           ~name="RegisterOnTypeFormattingSupport",
           waitForRegisterOnTypeFormattingSupport,
         )
      |> Test.withClient(
           Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
             ~delta=addedDelta,
           ),
         )
      |> Test.withClientRequest(
           ~name="get on type formatting",
           ~validate=
             (edits: option(list(Edit.SingleEditOperation.t))) => {
               expect.equal(
                 edits,
                 Some([
                   Edit.SingleEditOperation.{
                     range: expectedRange,
                     text: Some("as-you-type"),
                     forceMoveMarkers: false,
                   },
                 ]),
               );

               true;
             },
           getOnTypeFormattingEdits,
         )
      |> finishTest;
    });
  });
});
