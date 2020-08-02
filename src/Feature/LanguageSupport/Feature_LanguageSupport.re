open EditorCoreTypes;

type model = {
  codeLens: CodeLens.model,
  completion: Completion.model,
  definition: Definition.model,
  documentHighlights: DocumentHighlights.model,
  formatting: Formatting.model,
  rename: Rename.model,
  references: References.model,
};

let initial = {
  codeLens: CodeLens.initial,
  completion: Completion.initial,
  definition: Definition.initial,
  documentHighlights: DocumentHighlights.initial,
  formatting: Formatting.initial,
  rename: Rename.initial,
  references: References.initial,
};

[@deriving show]
type msg =
  | Exthost(Exthost.Msg.LanguageFeatures.msg)
  | Completion(Completion.msg)
  | Definition(Definition.msg)
  | DocumentHighlights(DocumentHighlights.msg)
  | Formatting(Formatting.msg)
  | References(References.msg)
  | Rename(Rename.msg)
  | CodeLens(CodeLens.msg)
  | KeyPressed(string)
  | Pasted(string);

type outmsg =
  | Nothing
  | ApplyCompletion({
      meetColumn: Index.t,
      insertText: string,
    })
  | InsertSnippet({
      meetColumn: Index.t,
      snippet: string,
    })
  | OpenFile({
      filePath: string,
      location: option(Location.t),
    })
  | NotifySuccess(string)
  | NotifyFailure(string)
  | Effect(Isolinear.Effect.t(msg));

let map: ('a => msg, Outmsg.internalMsg('a)) => outmsg =
  f =>
    fun
    | Outmsg.ApplyCompletion({meetColumn, insertText}) =>
      ApplyCompletion({meetColumn, insertText})
    | Outmsg.InsertSnippet({meetColumn, snippet}) =>
      InsertSnippet({meetColumn, snippet})
    | Outmsg.Nothing => Nothing
    | Outmsg.NotifySuccess(msg) => NotifySuccess(msg)
    | Outmsg.NotifyFailure(msg) => NotifyFailure(msg)
    | Outmsg.OpenFile({filePath, location}) => OpenFile({filePath, location})
    | Outmsg.Effect(eff) => Effect(eff |> Isolinear.Effect.map(f));

module Msg = {
  let exthost = msg => Exthost(msg);

  let keyPressed = key => KeyPressed(key);
  let pasted = key => Pasted(key);

  module Formatting = {
    let formatDocument = Formatting(Formatting.(Command(FormatDocument)));

    let formatRange = (~startLine, ~endLine) =>
      Formatting(Formatting.FormatRange({startLine, endLine}));
  };
};

let update =
    (
      ~configuration,
      ~languageConfiguration,
      ~maybeSelection,
      ~maybeBuffer,
      ~cursorLocation,
      ~client,
      msg,
      model,
    ) =>
  switch (msg) {
  | KeyPressed(_)
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

  | Exthost(Unregister({handle})) => (
      {
        codeLens: CodeLens.unregister(~handle, model.codeLens),
        completion: Completion.unregister(~handle, model.completion),
        definition: Definition.unregister(~handle, model.definition),
        documentHighlights:
          DocumentHighlights.unregister(~handle, model.documentHighlights),
        formatting: Formatting.unregister(~handle, model.formatting),
        references: References.unregister(~handle, model.references),
        rename: Rename.unregister(~handle, model.rename),
      },
      Nothing,
    )

  | Exthost(_) =>
    // TODO:
    (model, Nothing)

  | CodeLens(codeLensMsg) =>
    let codeLens' = CodeLens.update(codeLensMsg, model.codeLens);
    ({...model, codeLens: codeLens'}, Nothing);

  | Completion(completionMsg) =>
    let (completion', outmsg) =
      Completion.update(
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

  | Rename(renameMsg) =>
    let (rename', outmsg) = Rename.update(renameMsg, model.rename);
    ({...model, rename: rename'}, outmsg |> map(msg => Rename(msg)));
  };

let bufferUpdated =
    (~buffer, ~config, ~activeCursor, ~syntaxScope, ~triggerKey, model) => {
  let completion =
    Completion.bufferUpdated(
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

let startInsertMode = model => model;
let stopInsertMode = model => {
  ...model,
  completion: Completion.stopInsertMode(model.completion),
};

let isFocused = ({rename, _}) => Rename.isFocused(rename);

module Contributions = {
  open WhenExpr.ContextKeys.Schema;

  let colors = Completion.Contributions.colors;

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
      References.Contributions.commands
      |> List.map(Oni_Core.Command.map(msg => References(msg)))
    )
    @ (
      Formatting.Contributions.commands
      |> List.map(Oni_Core.Command.map(msg => Formatting(msg)))
    );

  let configuration = Completion.Contributions.configuration;

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
    @ Definition.Contributions.keybindings
    @ Completion.Contributions.keybindings;
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

let sub =
    (
      ~isInsertMode,
      ~activeBuffer,
      ~activePosition,
      ~visibleBuffers,
      ~client,
      {definition, completion, documentHighlights, _},
    ) => {
  let codeLensSub =
    CodeLens.sub(~visibleBuffers, ~client)
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
      : OldCompletion.sub(~client, completion)
        |> Isolinear.Sub.map(msg => Completion(msg));

  let documentHighlightsSub =
    OldHighlights.sub(
      ~buffer=activeBuffer,
      ~location=activePosition,
      ~client,
      documentHighlights,
    )
    |> Isolinear.Sub.map(msg => DocumentHighlights(msg));

  [codeLensSub, completionSub, definitionSub, documentHighlightsSub]
  |> Isolinear.Sub.batch;
};

// TODO: Remove
module Diagnostic = Diagnostic;
module Diagnostics = Diagnostics;
module LanguageFeatures = LanguageFeatures;
