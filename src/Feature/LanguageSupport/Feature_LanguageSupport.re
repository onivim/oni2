open EditorCoreTypes;

type model = {
  codeLens: CodeLens.model,
  completion: Completion.model,
  definition: Definition.model,
  documentHighlights: DocumentHighlights.model,
  documentSymbols: DocumentSymbols.model,
  formatting: Formatting.model,
  hover: Hover.model,
  rename: Rename.model,
  references: References.model,
};

let initial = {
  codeLens: CodeLens.initial,
  completion: Completion.initial,
  definition: Definition.initial,
  documentHighlights: DocumentHighlights.initial,
  documentSymbols: DocumentSymbols.initial,
  formatting: Formatting.initial,
  hover: Hover.initial,
  rename: Rename.initial,
  references: References.initial,
};

[@deriving show]
type msg =
  | Exthost(Exthost.Msg.LanguageFeatures.msg)
  | Completion(Completion.msg)
  | Definition(Definition.msg)
  | DocumentHighlights(DocumentHighlights.msg)
  | DocumentSymbols(DocumentSymbols.msg)
  | Formatting(Formatting.msg)
  | Hover(Hover.msg)
  | References(References.msg)
  | Rename(Rename.msg)
  | CodeLens(CodeLens.msg)
  | KeyPressed(string)
  | Pasted(string);

type outmsg =
  | Nothing
  | ApplyCompletion({
      meetColumn: CharacterIndex.t,
      insertText: string,
      additionalEdits: list(Exthost.Edit.SingleEditOperation.t),
    })
  | InsertSnippet({
      meetColumn: CharacterIndex.t,
      snippet: string,
      additionalEdits: list(Exthost.Edit.SingleEditOperation.t),
    })
  | OpenFile({
      filePath: string,
      location: option(CharacterPosition.t),
    })
  | ReferencesAvailable
  | NotifySuccess(string)
  | NotifyFailure(string)
  | Effect(Isolinear.Effect.t(msg))
  | CodeLensesChanged({
      bufferId: int,
      lenses: list(CodeLens.codeLens),
    });

let map: ('a => msg, Outmsg.internalMsg('a)) => outmsg =
  f =>
    fun
    | Outmsg.ApplyCompletion({meetColumn, insertText, additionalEdits}) =>
      ApplyCompletion({meetColumn, insertText, additionalEdits})
    | Outmsg.InsertSnippet({meetColumn, snippet, additionalEdits}) =>
      InsertSnippet({meetColumn, snippet, additionalEdits})
    | Outmsg.Nothing => Nothing
    | Outmsg.NotifySuccess(msg) => NotifySuccess(msg)
    | Outmsg.NotifyFailure(msg) => NotifyFailure(msg)
    | Outmsg.ReferencesAvailable => ReferencesAvailable
    | Outmsg.OpenFile({filePath, location}) => OpenFile({filePath, location})
    | Outmsg.Effect(eff) => Effect(eff |> Isolinear.Effect.map(f))
    | Outmsg.CodeLensesChanged({bufferId, lenses}) =>
      CodeLensesChanged({bufferId, lenses});

module Msg = {
  let exthost = msg => Exthost(msg);

  let keyPressed = key => KeyPressed(key);
  let pasted = key => Pasted(key);

  module Formatting = {
    let formatDocument = Formatting(Formatting.(Command(FormatDocument)));

    let formatRange = (~startLine, ~endLine) =>
      Formatting(Formatting.FormatRange({startLine, endLine}));
  };

  module Hover = {
    let show = Hover(Hover.(Command(Show)));

    let mouseHovered = location => Hover(Hover.MouseHovered(location));
    let mouseMoved = location => Hover(Hover.MouseMoved(location));

    let keyPressed = key => Hover(Hover.KeyPressed(key));
  };
};

let update =
    (
      ~config,
      ~configuration,
      ~languageConfiguration,
      ~maybeSelection,
      ~maybeBuffer,
      ~editorId,
      ~cursorLocation,
      ~client,
      msg,
      model,
    ) =>
  switch (msg) {
  | KeyPressed(key) => (
      {...model, hover: Hover.keyPressed(key, model.hover)},
      Nothing,
    )
  | Pasted(_) => (model, Nothing)

  | Exthost(RegisterCodeLensSupport({handle, selector, _})) =>
    let codeLens' = CodeLens.register(~handle, ~selector, model.codeLens);
    ({...model, codeLens: codeLens'}, Nothing);

  | Exthost(RegisterDefinitionSupport({handle, selector})) =>
    let definition' =
      Definition.register(~handle, ~selector, model.definition);
    ({...model, definition: definition'}, Nothing);

  | Exthost(RegisterDocumentHighlightProvider({handle, selector})) =>
    let documentHighlights' =
      DocumentHighlights.register(
        ~handle,
        ~selector,
        model.documentHighlights,
      );
    ({...model, documentHighlights: documentHighlights'}, Nothing);

  | Exthost(RegisterDocumentSymbolProvider({handle, selector, _})) =>
    let documentSymbols' =
      DocumentSymbols.register(~handle, ~selector, model.documentSymbols);
    ({...model, documentSymbols: documentSymbols'}, Nothing);

  | Exthost(RegisterReferenceSupport({handle, selector})) =>
    let references' =
      References.register(~handle, ~selector, model.references);
    ({...model, references: references'}, Nothing);

  | Exthost(
      RegisterSuggestSupport({
        handle,
        selector,
        triggerCharacters,
        extensionId,
        supportsResolveDetails,
      }),
    ) =>
    let completion' =
      Completion.register(
        ~handle,
        ~selector,
        ~triggerCharacters,
        ~supportsResolveDetails,
        ~extensionId,
        model.completion,
      );
    ({...model, completion: completion'}, Nothing);

  | Exthost(
      RegisterRangeFormattingSupport({handle, selector, displayName, _}),
    ) =>
    let formatting' =
      Formatting.registerRangeFormatter(
        ~handle,
        ~selector,
        ~displayName,
        model.formatting,
      );
    ({...model, formatting: formatting'}, Nothing);

  | Exthost(
      RegisterDocumentFormattingSupport({handle, selector, displayName, _}),
    ) =>
    let formatting' =
      Formatting.registerDocumentFormatter(
        ~handle,
        ~selector,
        ~displayName,
        model.formatting,
      );
    ({...model, formatting: formatting'}, Nothing);

  | Exthost(RegisterHoverProvider({handle, selector})) =>
    let hover' = Hover.register(~handle, ~selector, model.hover);

    ({...model, hover: hover'}, Nothing);

  | Exthost(Unregister({handle})) => (
      {
        codeLens: CodeLens.unregister(~handle, model.codeLens),
        completion: Completion.unregister(~handle, model.completion),
        definition: Definition.unregister(~handle, model.definition),
        documentHighlights:
          DocumentHighlights.unregister(~handle, model.documentHighlights),
        documentSymbols:
          DocumentSymbols.unregister(~handle, model.documentSymbols),
        formatting: Formatting.unregister(~handle, model.formatting),
        hover: Hover.unregister(~handle, model.hover),
        references: References.unregister(~handle, model.references),
        rename: Rename.unregister(~handle, model.rename),
      },
      Nothing,
    )

  | Exthost(_) =>
    // TODO:
    (model, Nothing)

  | CodeLens(codeLensMsg) =>
    let (codeLens', eff) = CodeLens.update(codeLensMsg, model.codeLens);
    let outmsg =
      switch (eff) {
      | CodeLens.Nothing => Outmsg.Nothing
      | CodeLens.CodeLensesChanged({bufferId, lenses}) =>
        Outmsg.CodeLensesChanged({bufferId, lenses})
      };
    ({...model, codeLens: codeLens'}, outmsg |> map(msg => CodeLens(msg)));

  | Completion(completionMsg) =>
    let (completion', outmsg) =
      Completion.update(
        ~config,
        ~languageConfiguration,
        ~maybeBuffer,
        ~activeCursor=cursorLocation,
        completionMsg,
        model.completion,
      );

    (
      {...model, completion: completion'},
      outmsg |> map(msg => Completion(msg)),
    );

  | Definition(definitionMsg) =>
    let (definition', outmsg) =
      Definition.update(definitionMsg, model.definition);
    (
      {...model, definition: definition'},
      outmsg |> map(msg => Definition(msg)),
    );

  | DocumentHighlights(documentHighlightsMsg) =>
    let documentHighlights' =
      DocumentHighlights.update(
        documentHighlightsMsg,
        model.documentHighlights,
      );
    ({...model, documentHighlights: documentHighlights'}, Nothing);

  | DocumentSymbols(documentSymbolsMsg) =>
    let documentSymbols' =
      DocumentSymbols.update(documentSymbolsMsg, model.documentSymbols);
    ({...model, documentSymbols: documentSymbols'}, Nothing);

  | References(referencesMsg) =>
    let (references', outmsg) =
      References.update(
        ~maybeBuffer,
        ~cursorLocation,
        ~client,
        referencesMsg,
        model.references,
      );

    (
      {...model, references: references'},
      outmsg |> map(msg => References(msg)),
    );

  | Formatting(formatMsg) =>
    let (formatting', outMsg) =
      Formatting.update(
        ~languageConfiguration,
        ~configuration,
        ~maybeSelection,
        ~maybeBuffer,
        ~extHostClient=client,
        formatMsg,
        model.formatting,
      );

    // TODO:
    let outMsg' =
      switch (outMsg) {
      | Formatting.Nothing => Nothing
      | Formatting.Effect(eff) =>
        Effect(eff |> Isolinear.Effect.map(msg => Formatting(msg)))
      | Formatting.FormattingApplied({displayName, editCount}) =>
        NotifySuccess(
          Printf.sprintf(
            "Formatting: Applied %d edits with %s",
            editCount,
            displayName,
          ),
        )
      | Formatting.FormatError(errorMsg) => NotifyFailure(errorMsg)
      };

    ({...model, formatting: formatting'}, outMsg');

  | Hover(hoverMsg) =>
    let (hover', outMsg) =
      Hover.update(
        ~cursorLocation,
        ~maybeBuffer,
        ~editorId,
        ~extHostClient=client,
        model.hover,
        hoverMsg,
      );

    let outMsg' =
      switch (outMsg) {
      | Hover.Nothing => Nothing
      | Hover.Effect(eff) =>
        Effect(eff |> Isolinear.Effect.map(msg => Hover(msg)))
      };

    ({...model, hover: hover'}, outMsg');

  | Rename(renameMsg) =>
    let (rename', outmsg) = Rename.update(renameMsg, model.rename);
    ({...model, rename: rename'}, outmsg |> map(msg => Rename(msg)));
  };

let bufferUpdated =
    (
      ~languageConfiguration,
      ~buffer,
      ~config,
      ~activeCursor,
      ~syntaxScope,
      ~triggerKey,
      model,
    ) => {
  let completion =
    Completion.bufferUpdated(
      ~languageConfiguration,
      ~buffer,
      ~config,
      ~activeCursor,
      ~syntaxScope,
      ~triggerKey,
      model.completion,
    );
  {...model, completion};
};

let cursorMoved = (~previous, ~current, model) => {
  let completion =
    Completion.cursorMoved(~previous, ~current, model.completion);
  {...model, completion};
};

let startInsertMode = model => {
  {...model, completion: Completion.startInsertMode(model.completion)};
};

let stopInsertMode = model => {
  {...model, completion: Completion.stopInsertMode(model.completion)};
};

let isFocused = ({rename, _}) => Rename.isFocused(rename);

module Contributions = {
  open WhenExpr.ContextKeys.Schema;

  let colors = CodeLens.Contributions.colors @ Completion.Contributions.colors;

  let commands =
    (
      Completion.Contributions.commands
      |> List.map(Oni_Core.Command.map(msg => Completion(msg)))
    )
    @ (
      Rename.Contributions.commands
      |> List.map(Oni_Core.Command.map(msg => Rename(msg)))
    )
    @ (
      Definition.Contributions.commands
      |> List.map(Oni_Core.Command.map(msg => Definition(msg)))
    )
    @ (
      Hover.Contributions.commands
      |> List.map(Oni_Core.Command.map(msg => Hover(msg)))
    )
    @ (
      References.Contributions.commands
      |> List.map(Oni_Core.Command.map(msg => References(msg)))
    )
    @ (
      Formatting.Contributions.commands
      |> List.map(Oni_Core.Command.map(msg => Formatting(msg)))
    );

  let configuration =
    CodeLens.Contributions.configuration
    @ Completion.Contributions.configuration;

  let contextKeys =
    [
      Rename.Contributions.contextKeys
      |> fromList
      |> map(({rename, _}: model) => rename),
      Completion.Contributions.contextKeys
      |> fromList
      |> map(({completion, _}: model) => completion),
    ]
    |> unionMany;

  let keybindings =
    Rename.Contributions.keybindings
    @ Completion.Contributions.keybindings
    @ Definition.Contributions.keybindings
    @ References.Contributions.keybindings;
};

module OldCompletion = Completion;
module OldDefinition = Definition;
module OldHighlights = DocumentHighlights;

module Completion = {
  let isActive = ({completion, _}: model) =>
    OldCompletion.isActive(completion);

  let providerCount = ({completion, _}: model) =>
    OldCompletion.providerCount(completion);

  let availableCompletionCount = ({completion, _}: model) =>
    OldCompletion.availableCompletionCount(completion);

  module View = {
    let make =
        (~x, ~y, ~lineHeight, ~theme, ~tokenTheme, ~editorFont, ~model, ()) => {
      OldCompletion.View.make(
        ~x,
        ~y,
        ~lineHeight,
        ~theme,
        ~tokenTheme,
        ~editorFont,
        ~completions=model.completion,
        (),
      );
    };
  };
};

module Definition = {
  let get = (~bufferId, {definition, _}: model) => {
    OldDefinition.get(~bufferId, definition);
  };

  let getAt = (~bufferId, ~range, {definition, _}: model) => {
    OldDefinition.getAt(~bufferId, ~range, definition);
  };

  let isAvailable = (~bufferId, model) => {
    model |> get(~bufferId) |> Option.is_some;
  };
};

module DocumentHighlights = {
  let getByLine = (~bufferId, ~line, {documentHighlights, _}) => {
    OldHighlights.getByLine(~bufferId, ~line, documentHighlights);
  };

  let getLinesWithHighlight = (~bufferId, {documentHighlights, _}) => {
    OldHighlights.getLinesWithHighlight(~bufferId, documentHighlights);
  };
};

module OriginalDocumentSymbols = DocumentSymbols;

module DocumentSymbols = {
  include OriginalDocumentSymbols;

  let get = (~bufferId, {documentSymbols, _}) => {
    OriginalDocumentSymbols.get(~bufferId, documentSymbols);
  };
};

module OldHover = Hover;
module Hover = {
  module Popup = {
    let make =
        (
          ~diagnostics,
          ~theme,
          ~tokenTheme,
          ~languageInfo,
          ~uiFont,
          ~editorFont,
          ~grammars,
          ~model,
          ~buffer,
          ~editorId,
        ) => {
      let {hover, _} = model;
      OldHover.Popup.make(
        ~diagnostics,
        ~theme,
        ~tokenTheme,
        ~languageInfo,
        ~uiFont,
        ~editorFont,
        ~grammars,
        ~model=hover,
        ~buffer,
        ~editorId,
      );
    };
  };
};

module OldReferences = References;
module References = {
  let get = ({references, _}) => OldReferences.get(references);
};

module ShadowedCodeLens = CodeLens;
module CodeLens = {
  type t = ShadowedCodeLens.codeLens;

  let get = (~bufferId, model) => {
    ShadowedCodeLens.get(~bufferId, model.codeLens);
  };

  let lineNumber = codeLens => ShadowedCodeLens.lineNumber(codeLens);
  let uniqueId = codeLens => ShadowedCodeLens.uniqueId(codeLens);

  let text = codeLens => ShadowedCodeLens.text(codeLens);

  module View = ShadowedCodeLens.View;
};

let sub =
    (
      ~config,
      ~isInsertMode,
      ~activeBuffer,
      ~activePosition,
      ~visibleBuffers,
      ~visibleBuffersAndRanges,
      ~client,
      {
        codeLens,
        definition,
        completion,
        documentHighlights,
        documentSymbols,
        _,
      },
    ) => {
  let codeLensSub =
    ShadowedCodeLens.sub(
      ~config,
      ~visibleBuffers,
      ~visibleBuffersAndRanges,
      ~client,
      codeLens,
    )
    |> Isolinear.Sub.map(msg => CodeLens(msg));

  let definitionSub =
    isInsertMode
      ? Isolinear.Sub.none
      : OldDefinition.sub(
          ~buffer=activeBuffer,
          ~location=activePosition,
          ~client,
          definition,
        )
        |> Isolinear.Sub.map(msg => Definition(msg));

  let completionSub =
    !isInsertMode
      ? Isolinear.Sub.none
      : OldCompletion.sub(~activeBuffer, ~client, completion)
        |> Isolinear.Sub.map(msg => Completion(msg));

  let documentSymbolsSub =
    DocumentSymbols.sub(~buffer=activeBuffer, ~client, documentSymbols)
    |> Isolinear.Sub.map(msg => DocumentSymbols(msg));

  let documentHighlightsSub =
    OldHighlights.sub(
      ~buffer=activeBuffer,
      ~location=activePosition,
      ~client,
      documentHighlights,
    )
    |> Isolinear.Sub.map(msg => DocumentHighlights(msg));

  [
    codeLensSub,
    completionSub,
    definitionSub,
    documentHighlightsSub,
    documentSymbolsSub,
  ]
  |> Isolinear.Sub.batch;
};

module CompletionMeet = CompletionMeet;
