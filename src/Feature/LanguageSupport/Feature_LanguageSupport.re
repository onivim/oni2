open EditorCoreTypes;

type model = {
  codeActions: CodeActions.model,
  codeLens: CodeLens.model,
  completion: Completion.model,
  definition: Definition.model,
  documentHighlights: DocumentHighlights.model,
  documentSymbols: DocumentSymbols.model,
  formatting: Formatting.model,
  hover: Hover.model,
  rename: Rename.model,
  references: References.model,
  signatureHelp: SignatureHelp.model,
  languageInfo: Exthost.LanguageInfo.t,
};

let initial = {
  codeActions: CodeActions.initial,
  codeLens: CodeLens.initial,
  completion: Completion.initial,
  definition: Definition.initial,
  documentHighlights: DocumentHighlights.initial,
  documentSymbols: DocumentSymbols.initial,
  formatting: Formatting.initial,
  hover: Hover.initial,
  rename: Rename.initial,
  references: References.initial,
  signatureHelp: SignatureHelp.initial,
  languageInfo: Exthost.LanguageInfo.initial,
};

let languageInfo = ({languageInfo, _}) => languageInfo;

[@deriving show]
type command =
  | CloseAllLanguagePopups;

[@deriving show]
type msg =
  | Exthost(Exthost.Msg.LanguageFeatures.msg)
  | CodeActions(CodeActions.msg)
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
  | SignatureHelp(SignatureHelp.msg)
  | Command(command)
  | Pasted(string);

type outmsg =
  | Nothing
  | ApplyCompletion({
      replaceSpan: CharacterSpan.t,
      insertText: string,
      additionalEdits: list(Exthost.Edit.SingleEditOperation.t),
    })
  | ApplyWorkspaceEdit(Exthost.WorkspaceEdit.t)
  | FormattingApplied({
      displayName: string,
      editCount: int,
      needsToSave: bool,
    })
  | InsertSnippet({
      replaceSpan: CharacterSpan.t,
      snippet: string,
      additionalEdits: list(Exthost.Edit.SingleEditOperation.t),
    })
  | OpenFile({
      filePath: string,
      location: option(CharacterPosition.t),
      direction: Oni_Core.SplitDirection.t,
    })
  | PreviewFile({
      filePath: string,
      position: EditorCoreTypes.CharacterPosition.t,
    })
  | ReferencesAvailable
  | NotifySuccess(string)
  | NotifyFailure(string)
  | Effect(Isolinear.Effect.t(msg))
  | CodeLensesChanged({
      handle: int,
      bufferId: int,
      startLine: EditorCoreTypes.LineNumber.t,
      stopLine: EditorCoreTypes.LineNumber.t,
      lenses: list(CodeLens.codeLens),
    })
  | SetSelections({
      editorId: int,
      ranges: list(CharacterRange.t),
    })
  | ShowMenu(Feature_Quickmenu.Schema.menu(msg))
  | TransformConfiguration(Oni_Core.ConfigurationTransformer.t);

let map: ('a => msg, Outmsg.internalMsg('a)) => outmsg =
  f =>
    fun
    | Outmsg.ApplyCompletion({replaceSpan, insertText, additionalEdits}) =>
      ApplyCompletion({replaceSpan, insertText, additionalEdits})
    | Outmsg.ApplyWorkspaceEdit(edit) => ApplyWorkspaceEdit(edit)
    | Outmsg.FormattingApplied({displayName, editCount, needsToSave}) =>
      FormattingApplied({displayName, editCount, needsToSave})
    | Outmsg.InsertSnippet({replaceSpan, snippet, additionalEdits}) =>
      InsertSnippet({replaceSpan, snippet, additionalEdits})
    | Outmsg.Nothing => Nothing
    | Outmsg.NotifySuccess(msg) => NotifySuccess(msg)
    | Outmsg.NotifyFailure(msg) => NotifyFailure(msg)
    | Outmsg.ReferencesAvailable => ReferencesAvailable
    | Outmsg.OpenFile({filePath, location, direction}) =>
      OpenFile({filePath, location, direction})
    | Outmsg.PreviewFile({filePath, position}) =>
      PreviewFile({filePath, position})
    | Outmsg.Effect(eff) => Effect(eff |> Isolinear.Effect.map(f))
    | Outmsg.CodeLensesChanged({
        handle,
        bufferId,
        lenses,
        startLine,
        stopLine,
      }) =>
      CodeLensesChanged({handle, bufferId, lenses, startLine, stopLine})
    | Outmsg.SetSelections({editorId, ranges}) =>
      SetSelections({editorId, ranges})
    | Outmsg.ShowMenu(menu) =>
      ShowMenu(menu |> Feature_Quickmenu.Schema.map(f))
    | Outmsg.TransformConfiguration(transformer) =>
      TransformConfiguration(transformer);

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

// ~previewEnabled,
// ~languageInfo,
// ~buffers,
// ~font,
let update =
    (
      ~buffers,
      ~config,
      ~diagnostics,
      ~extensions,
      ~font,
      ~languageConfiguration,
      ~languageInfo,
      ~maybeSelection,
      ~maybeBuffer,
      ~previewEnabled,
      ~editorId,
      ~cursorLocation,
      ~client,
      msg,
      model,
    ) =>
  switch (msg) {
  | KeyPressed(key) => (
      {
        ...model,
        hover: Hover.keyPressed(key, model.hover),
        rename:
          Rename.isFocused(model.rename)
            ? Rename.keyPressed(key, model.rename) : model.rename,
      },
      Nothing,
    )
  | Pasted(_) => (model, Nothing)

  | Exthost(EmitCodeLensEvent({eventHandle, _})) =>
    let codeLens' = CodeLens.emit(~eventHandle, model.codeLens);
    ({...model, codeLens: codeLens'}, Nothing);

  | Exthost(RegisterCodeLensSupport({handle, selector, eventHandle})) =>
    let codeLens' =
      CodeLens.register(
        ~handle,
        ~selector,
        ~maybeEventHandle=eventHandle,
        model.codeLens,
      );
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

  | Exthost(
      RegisterQuickFixSupport({
        handle,
        selector,
        metadata,
        displayName,
        supportsResolve,
      }),
    ) =>
    let codeActions' =
      CodeActions.register(
        ~handle,
        ~selector,
        ~metadata,
        ~displayName,
        ~supportsResolve,
        model.codeActions,
      );

    ({...model, codeActions: codeActions'}, Nothing);

  | Exthost(RegisterReferenceSupport({handle, selector})) =>
    let references' =
      References.register(~handle, ~selector, model.references);
    ({...model, references: references'}, Nothing);

  | Exthost(RegisterSignatureHelpProvider({handle, selector, metadata})) =>
    let sigHelp' =
      SignatureHelp.register(
        ~handle,
        ~selector,
        ~metadata,
        model.signatureHelp,
      );
    ({...model, signatureHelp: sigHelp'}, Nothing);

  | Exthost(
      RegisterRenameSupport({handle, selector, supportsResolveInitialValues}),
    ) =>
    let rename' =
      Rename.register(
        ~handle,
        ~selector,
        ~supportsResolveInitialValues,
        model.rename,
      );
    ({...model, rename: rename'}, Nothing);

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
      RegisterRangeFormattingSupport({
        handle,
        selector,
        displayName,
        extensionId,
      }),
    ) =>
    let formatting' =
      Formatting.registerRangeFormatter(
        ~handle,
        ~selector,
        ~extensionId,
        ~displayName,
        model.formatting,
      );
    ({...model, formatting: formatting'}, Nothing);

  | Exthost(
      RegisterDocumentFormattingSupport({
        handle,
        selector,
        displayName,
        extensionId,
      }),
    ) =>
    let formatting' =
      Formatting.registerDocumentFormatter(
        ~handle,
        ~selector,
        ~extensionId,
        ~displayName,
        model.formatting,
      );
    ({...model, formatting: formatting'}, Nothing);

  | Exthost(RegisterHoverProvider({handle, selector})) =>
    let hover' = Hover.register(~handle, ~selector, model.hover);

    ({...model, hover: hover'}, Nothing);

  | Exthost(Unregister({handle})) => (
      {
        ...model,
        codeActions: CodeActions.unregister(~handle, model.codeActions),
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
        signatureHelp: SignatureHelp.unregister(~handle, model.signatureHelp),
      },
      Nothing,
    )

  | Exthost(SetLanguageConfiguration({handle, languageId, configuration})) =>
    // TODO - Wire up to language info pipeline, and use the onEnterRules:
    ignore(handle);
    ignore(languageId);
    ignore(configuration);

    (model, Nothing);

  | Exthost(_) =>
    // TODO:
    (model, Nothing)

  | CodeActions(codeActionsMsg) =>
    maybeBuffer
    |> Option.map(buffer => {
         let (codeActions', outmsg) =
           CodeActions.update(
             ~buffer,
             ~editorId,
             ~cursorLocation,
             codeActionsMsg,
             model.codeActions,
           );
         let outmsg' = outmsg |> map(msg => CodeActions(msg));
         ({...model, codeActions: codeActions'}, outmsg');
       })
    |> Option.value(~default=(model, Nothing))

  | CodeLens(codeLensMsg) =>
    let (codeLens', eff) = CodeLens.update(codeLensMsg, model.codeLens);
    let outmsg =
      switch (eff) {
      | CodeLens.Nothing => Outmsg.Nothing
      | CodeLens.CodeLensesChanged({
          handle,
          bufferId,
          startLine,
          stopLine,
          lenses,
        }) =>
        Outmsg.CodeLensesChanged({
          handle,
          bufferId,
          startLine,
          stopLine,
          lenses,
        })
      };
    ({...model, codeLens: codeLens'}, outmsg |> map(msg => CodeLens(msg)));

  | Completion(completionMsg) =>
    let (completion', outmsg) =
      Completion.update(
        ~config,
        ~extensions,
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
    let (documentHighlights', outmsg) =
      DocumentHighlights.update(
        ~maybeBuffer,
        ~editorId,
        documentHighlightsMsg,
        model.documentHighlights,
      );

    (
      {...model, documentHighlights: documentHighlights'},
      outmsg |> map(msg => DocumentHighlights(msg)),
    );

  | DocumentSymbols(documentSymbolsMsg) =>
    let (documentSymbols', outmsg) =
      DocumentSymbols.update(
        ~maybeBuffer,
        documentSymbolsMsg,
        model.documentSymbols,
      );
    (
      {...model, documentSymbols: documentSymbols'},
      outmsg |> map(msg => DocumentSymbols(msg)),
    );

  | References(referencesMsg) =>
    let (references', outmsg) =
      References.update(
        ~buffers,
        ~font,
        ~languageInfo,
        ~previewEnabled,
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
        ~config,
        ~languageConfiguration,
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
      | Formatting.FormattingApplied({displayName, editCount, needsToSave}) =>
        FormattingApplied({displayName, editCount, needsToSave})
      | Formatting.FormatError(errorMsg) => NotifyFailure(errorMsg)
      | Formatting.ShowMenu(menu) =>
        let menu' =
          menu |> Feature_Quickmenu.Schema.map(msg => Formatting(msg));
        ShowMenu(menu');

      | Formatting.TransformConfiguration(transformer) =>
        TransformConfiguration(transformer)
      };

    ({...model, formatting: formatting'}, outMsg');

  | Hover(hoverMsg) =>
    let (hover', outMsg) =
      Hover.update(
        ~languageConfiguration,
        ~cursorLocation,
        ~diagnostics,
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
    let (rename', outmsg) =
      Rename.update(
        ~client,
        ~maybeBuffer,
        ~cursorLocation,
        renameMsg,
        model.rename,
      );
    ({...model, rename: rename'}, outmsg |> map(msg => Rename(msg)));

  | SignatureHelp(sigHelpMsg) =>
    let (sigHelp', outmsg) =
      SignatureHelp.update(
        ~maybeBuffer,
        ~cursor=cursorLocation,
        model.signatureHelp,
        sigHelpMsg,
      );
    let outmsg' =
      switch (outmsg) {
      | SignatureHelp.Nothing => Nothing
      | SignatureHelp.Effect(eff) =>
        Effect(eff |> Isolinear.Effect.map(msg => SignatureHelp(msg)))
      | SignatureHelp.Error(msg) => NotifyFailure(msg)
      };
    ({...model, signatureHelp: sigHelp'}, outmsg');

  | Command(CloseAllLanguagePopups) => (
      {
        ...model,
        signatureHelp: SignatureHelp.cancel(model.signatureHelp),
        completion: Completion.cancel(model.completion),
      },
      Nothing,
    )
  };

let bufferUpdated =
    (
      ~languageConfiguration,
      ~buffer,
      ~config,
      ~extensions,
      ~activeCursor,
      ~syntaxScope,
      ~triggerKey,
      model,
    ) => {
  let completion =
    Completion.bufferUpdated(
      ~languageConfiguration,
      ~extensions,
      ~buffer,
      ~config,
      ~activeCursor,
      ~syntaxScope,
      ~triggerKey,
      model.completion,
    );

  let signatureHelp =
    SignatureHelp.bufferUpdated(
      ~languageConfiguration,
      ~buffer,
      ~activeCursor,
      model.signatureHelp,
    );
  {...model, completion, signatureHelp};
};

let bufferSaved =
    (
      ~reason,
      ~isLargeBuffer,
      ~buffer,
      ~config,
      ~savedBufferId as _,
      ~activeBufferId,
      model,
    ) =>
  if (reason == Oni_Core.SaveReason.AutoSave
      || reason == Oni_Core.SaveReason.UserInitiated({allowFormatting: false})) {
    (model, Isolinear.Effect.none);
  } else {
    let (formatting', formattingEffect) =
      Formatting.bufferSaved(
        ~isLargeBuffer,
        ~buffer,
        ~config,
        ~activeBufferId,
        model.formatting,
      );
    (
      {...model, formatting: formatting'},
      formattingEffect |> Isolinear.Effect.map(msg => Formatting(msg)),
    );
  };

let configurationChanged = (~config, model) => {
  ...model,
  codeActions: CodeActions.configurationChanged(~config, model.codeActions),
  completion: Completion.configurationChanged(~config, model.completion),
  documentHighlights:
    DocumentHighlights.configurationChanged(
      ~config,
      model.documentHighlights,
    ),
};

let cursorMoved =
    (~editorId, ~languageConfiguration, ~buffer, ~previous, ~current, model) => {
  let codeActions =
    CodeActions.cursorMoved(
      ~editorId,
      ~buffer,
      ~cursor=current,
      model.codeActions,
    );
  let completion =
    Completion.cursorMoved(
      ~languageConfiguration,
      ~buffer,
      ~current,
      model.completion,
    );

  let documentHighlights =
    DocumentHighlights.cursorMoved(
      ~buffer,
      ~cursor=current,
      model.documentHighlights,
    );

  let signatureHelp =
    SignatureHelp.cursorMoved(~previous, ~current, model.signatureHelp);
  {...model, codeActions, completion, documentHighlights, signatureHelp};
};

let moveMarkers = (~newBuffer, ~markerUpdate, model) => {
  {
    ...model,
    documentHighlights:
      DocumentHighlights.moveMarkers(
        ~buffer=newBuffer,
        ~markerUpdate,
        model.documentHighlights,
      ),
  };
};

let startInsertMode = (~config, ~maybeBuffer, model) => {
  {
    ...model,
    completion: Completion.startInsertMode(model.completion),
    signatureHelp:
      SignatureHelp.startInsert(~config, ~maybeBuffer, model.signatureHelp),
  };
};

let stopInsertMode = model => {
  {
    ...model,
    completion: Completion.stopInsertMode(model.completion),
    signatureHelp: SignatureHelp.stopInsert(model.signatureHelp),
  };
};

let startSnippet = model => {
  ...model,
  completion: Completion.startSnippet(model.completion),
};

let stopSnippet = model => {
  ...model,
  completion: Completion.stopSnippet(model.completion),
};

let isFocused = ({rename, _}) => Rename.isFocused(rename);

module Commands = {
  open Feature_Commands.Schema;
  let close =
    define("closeAllLanguagePopups", Command(CloseAllLanguagePopups));
};

module Keybindings = {
  open Feature_Input.Schema;
  let close =
    bind(
      ~key="<S-ESC>",
      ~command=Commands.close.id,
      ~condition=
        WhenExpr.And([
          WhenExpr.Defined("editorTextFocus"),
          WhenExpr.Or([
            WhenExpr.Defined("parameterHintsVisible"),
            WhenExpr.Defined("suggestWidgetVisible"),
          ]),
        ]),
    );
};

module Contributions = {
  open WhenExpr.ContextKeys.Schema;

  let colors = CodeLens.Contributions.colors @ Completion.Contributions.colors;

  let commands = model =>
    [Commands.close]
    @ (
      Completion.Contributions.commands
      |> List.map(Oni_Core.Command.map(msg => Completion(msg)))
    )
    @ (
      CodeActions.Contributions.commands(model.codeActions)
      |> List.map(Oni_Core.Command.map(msg => CodeActions(msg)))
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
      DocumentHighlights.Contributions.commands
      |> List.map(Oni_Core.Command.map(msg => DocumentHighlights(msg)))
    )
    @ (
      DocumentSymbols.Contributions.commands
      |> List.map(Oni_Core.Command.map(msg => DocumentSymbols(msg)))
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
    )
    @ (
      SignatureHelp.Contributions.commands
      |> List.map(Oni_Core.Command.map(msg => SignatureHelp(msg)))
    );

  let configuration =
    CodeActions.Contributions.configuration
    @ CodeLens.Contributions.configuration
    @ Completion.Contributions.configuration
    @ DocumentHighlights.Contributions.configuration
    @ Formatting.Contributions.configuration
    @ SignatureHelp.Contributions.configuration;

  let contextKeys = {
    let static = [
      Rename.Contributions.contextKeys
      |> fromList
      |> map(({rename, _}: model) => rename),
      Completion.Contributions.contextKeys
      |> fromList
      |> map(({completion, _}: model) => completion),
      SignatureHelp.Contributions.contextKeys
      |> fromList
      |> map(({signatureHelp, _}: model) => signatureHelp),
    ];
    model => {
      let staticContextKeys =
        static |> unionMany |> WhenExpr.ContextKeys.fromSchema(model);

      let codeActions =
        CodeActions.Contributions.contextKeys(model.codeActions);

      WhenExpr.ContextKeys.unionMany([staticContextKeys, codeActions]);
    };
  };

  let keybindings =
    Keybindings.[close]
    @ Rename.Contributions.keybindings
    @ CodeActions.Contributions.keybindings
    @ Completion.Contributions.keybindings
    @ Definition.Contributions.keybindings
    @ DocumentHighlights.Contributions.keybindings
    @ DocumentSymbols.Contributions.keybindings
    @ Formatting.Contributions.keybindings
    @ References.Contributions.keybindings
    @ SignatureHelp.Contributions.keybindings;

  let panes = [
    References.Contributions.pane
    |> Feature_Pane.Schema.map(
         ~msg=msg => References(msg),
         ~model=({references, _}) => references,
       ),
  ];

  let menuGroups =
    DocumentSymbols.Contributions.menuGroups
    @ Formatting.Contributions.menuGroups;
};

module OldCompletion = Completion;
module OldDefinition = Definition;
module OldHighlights = DocumentHighlights;
module OldSignatureHelp = SignatureHelp;

module Completion = {
  let isActive = ({completion, _}: model) =>
    OldCompletion.isActive(completion);

  let providerCount = ({completion, _}: model) =>
    OldCompletion.providerCount(completion);

  let availableCompletionCount = ({completion, _}: model) =>
    OldCompletion.availableCompletionCount(completion);
};

module SignatureHelp = {
  let isActive = ({signatureHelp, _}: model) =>
    OldSignatureHelp.isShown(signatureHelp);

  module View = {
    let make =
        (
          ~x,
          ~y,
          ~theme,
          ~tokenTheme,
          ~editorFont,
          ~uiFont,
          ~languageInfo,
          ~buffer,
          ~grammars,
          ~dispatch,
          ~model,
          (),
        ) => {
      OldSignatureHelp.View.make(
        ~x,
        ~y,
        ~colorTheme=theme,
        ~tokenTheme,
        ~editorFont,
        ~uiFont,
        ~languageInfo,
        ~buffer,
        ~grammars,
        ~dispatch=msg => dispatch(SignatureHelp(msg)),
        ~model=model.signatureHelp,
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

  let lineNumber = codeLens => ShadowedCodeLens.lineNumber(codeLens);

  let text = codeLens => ShadowedCodeLens.text(codeLens);

  module View = ShadowedCodeLens.View;
};

module ShadowedRename = Rename;

module Rename = {
  let isActive = model => ShadowedRename.isFocused(model.rename);

  module View = {
    let make = (~x, ~y, ~theme, ~model, ~font, ~dispatch, ()) => {
      let dispatch = msg => dispatch(Rename(msg));
      let model = model.rename;

      <ShadowedRename.View x y theme model font dispatch />;
    };
  };
};

let sub =
    (
      ~config,
      ~isInsertMode,
      ~isAnimatingScroll,
      ~activeBuffer,
      ~activeEditor,
      ~activePosition,
      ~lineHeightInPixels,
      ~positionToRelativePixel,
      ~topVisibleBufferLine,
      ~bottomVisibleBufferLine,
      ~visibleBuffers,
      ~client,
      {
        codeActions,
        codeLens,
        definition,
        completion,
        documentHighlights,
        documentSymbols,
        rename,
        signatureHelp,
        _,
      },
    ) => {
  let codeLensSub =
    ShadowedCodeLens.sub(
      ~config,
      ~isAnimatingScroll,
      ~visibleBuffers,
      ~topVisibleBufferLine,
      ~bottomVisibleBufferLine,
      ~client,
      codeLens,
    )
    |> Isolinear.Sub.map(msg => CodeLens(msg));

  let codeActionsSub =
    CodeActions.sub(
      ~buffer=activeBuffer,
      ~config,
      ~activePosition,
      ~activeEditor,
      ~topVisibleBufferLine,
      ~bottomVisibleBufferLine,
      ~lineHeightInPixels,
      ~positionToRelativePixel,
      ~client,
      codeActions,
    )
    |> Isolinear.Sub.map(msg => CodeActions(msg));

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
      ~isInsertMode,
      ~config,
      ~buffer=activeBuffer,
      ~location=activePosition,
      ~client,
      documentHighlights,
    )
    |> Isolinear.Sub.map(msg => DocumentHighlights(msg));

  let renameSub =
    ShadowedRename.sub(~activeBuffer, ~activePosition, ~client, rename)
    |> Isolinear.Sub.map(msg => Rename(msg));

  let signatureHelpSub =
    OldSignatureHelp.sub(
      ~isInsertMode,
      ~buffer=activeBuffer,
      ~activePosition,
      ~client,
      signatureHelp,
    )
    |> Isolinear.Sub.map(msg => SignatureHelp(msg));

  [
    codeActionsSub,
    codeLensSub,
    completionSub,
    definitionSub,
    documentHighlightsSub,
    documentSymbolsSub,
    renameSub,
    signatureHelpSub,
  ]
  |> Isolinear.Sub.batch;
};

let extensionsAdded = (extensions, model) => {
  ...model,
  languageInfo:
    Exthost.LanguageInfo.addExtensions(extensions, model.languageInfo),
};

module CompletionMeet = CompletionMeet;

module View = {
  module EditorWidgets = {
    let make =
        (
          ~x,
          ~y,
          ~editorId,
          ~theme,
          ~model,
          ~editorFont,
          ~uiFont as _,
          ~dispatch as _,
          (),
        ) => {
      <CodeActions.View.EditorWidgets
        x
        y
        editorId
        editorFont
        theme
        model={model.codeActions}
      />;
    };
  };

  module Overlay = {
    let make =
        (
          ~activeEditorId: int,
          ~activeBuffer: Oni_Core.Buffer.t,
          ~cursorPosition: CharacterPosition.t,
          ~lineHeight: float,
          ~toPixel,
          ~theme,
          ~tokenTheme,
          ~model,
          ~editorFont,
          ~uiFont,
          ~dispatch,
          (),
        ) => {
      [
        <CodeActions.View.Overlay
          toPixel
          theme
          model={model.codeActions}
          dispatch={msg => dispatch(CodeActions(msg))}
          editorFont
          uiFont
        />,
        <OldCompletion.View.Overlay
          activeEditorId
          cursorPosition
          buffer=activeBuffer
          toPixel
          theme
          lineHeight
          tokenTheme
          model={model.completion}
          dispatch={msg => dispatch(Completion(msg))}
          editorFont
          uiFont
        />,
      ]
      |> Revery.UI.React.listToElement;
    };
  };
};
