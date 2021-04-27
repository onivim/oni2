open EditorCoreTypes;
open Oni_Core;

// EFFECTS
module Effects: {
  module Commands: {
    let executeContributedCommand:
      (
        ~command: string,
        ~arguments: list(Oni_Core.Json.t),
        Exthost.Client.t
      ) =>
      Isolinear.Effect.t(_);
  };

  module Decorations: {
    let provideDecorations:
      (
        ~handle: int,
        ~requests: list(Exthost.Request.Decorations.request),
        ~toMsg: Oni_Core.IntMap.t(Exthost.Request.Decorations.decoration) =>
                'msg,
        Exthost.Client.t
      ) =>
      Isolinear.Effect.t('msg);
  };

  module Documents: {
    let modelChanged:
      (
        ~previousBuffer: Oni_Core.Buffer.t,
        ~buffer: Oni_Core.Buffer.t,
        ~minimalUpdate: Oni_Core.MinimalUpdate.t,
        ~update: Oni_Core.BufferUpdate.t,
        Exthost.Client.t,
        unit => 'msg
      ) =>
      Isolinear.Effect.t('msg);

    let modelSaved:
      (~uri: Oni_Core.Uri.t, Exthost.Client.t, unit => 'msg) =>
      Isolinear.Effect.t('msg);
  };

  module FileSystemEventService: {
    let onBufferChanged:
      (~buffer: Oni_Core.Buffer.t, Exthost.Client.t) => Isolinear.Effect.t(_);

    let onPathChanged:
      (
        ~path: FpExp.t(FpExp.absolute),
        ~maybeStat: option(Luv.File.Stat.t),
        Exthost.Client.t
      ) =>
      Isolinear.Effect.t(_);
  };

  module SCM: {
    let getOriginalContent:
      (
        ~handle: int,
        ~uri: Oni_Core.Uri.t,
        ~toMsg: array(string) => 'msg,
        Exthost.Client.t
      ) =>
      Isolinear.Effect.t('msg);

    let onInputBoxValueChange:
      (~handle: int, ~value: string, Exthost.Client.t) =>
      Isolinear.Effect.t(_);
  };

  module LanguageFeatures: {
    let provideDocumentFormattingEdits:
      (
        ~handle: int,
        ~uri: Oni_Core.Uri.t,
        ~options: Exthost.FormattingOptions.t,
        Exthost.Client.t,
        result(list(Exthost.Edit.SingleEditOperation.t), string) => 'msg
      ) =>
      Isolinear.Effect.t('msg);

    let provideDocumentRangeFormattingEdits:
      (
        ~handle: int,
        ~uri: Oni_Core.Uri.t,
        ~range: CharacterRange.t,
        ~options: Exthost.FormattingOptions.t,
        Exthost.Client.t,
        result(list(Exthost.Edit.SingleEditOperation.t), string) => 'msg
      ) =>
      Isolinear.Effect.t('msg);

    let provideHover:
      (
        ~handle: int,
        ~uri: Oni_Core.Uri.t,
        ~position: EditorCoreTypes.CharacterPosition.t,
        Exthost.Client.t,
        result(Exthost.Hover.t, string) => 'msg
      ) =>
      Isolinear.Effect.t('msg);

    let provideReferences:
      (
        ~handle: int,
        ~uri: Oni_Core.Uri.t,
        ~position: EditorCoreTypes.CharacterPosition.t,
        ~context: Exthost.ReferenceContext.t,
        Exthost.Client.t,
        result(list(Exthost.Location.t), string) => 'msg
      ) =>
      Isolinear.Effect.t('msg);

    let provideRenameEdits:
      (
        ~handle: int,
        ~uri: Oni_Core.Uri.t,
        ~position: EditorCoreTypes.CharacterPosition.t,
        ~newName: string,
        Exthost.Client.t,
        result(option(Exthost.WorkspaceEdit.t), string) => 'msg
      ) =>
      Isolinear.Effect.t('msg);

    let resolveRenameLocation:
      (
        ~handle: int,
        ~uri: Oni_Core.Uri.t,
        ~position: EditorCoreTypes.CharacterPosition.t,
        Exthost.Client.t,
        result(option(Exthost.RenameLocation.t), string) => 'msg
      ) =>
      Isolinear.Effect.t('msg);
  };

  module Workspace: {
    let change:
      (~workspace: option(Exthost.WorkspaceData.t), Exthost.Client.t) =>
      Isolinear.Effect.t(_);
  };
};

module Sub: {
  let buffer:
    (
      ~buffer: Oni_Core.Buffer.t,
      ~client: Exthost.Client.t,
      ~toMsg: [ | `Added] => 'msg
    ) =>
    Isolinear.Sub.t('msg);

  let editor:
    (~editor: Exthost.TextEditor.AddData.t, ~client: Exthost.Client.t) =>
    Isolinear.Sub.t(unit);

  let activeEditor:
    (~activeEditorId: string, ~client: Exthost.Client.t) =>
    Isolinear.Sub.t(unit);

  // let codeActionsBySelection:
  //   (
  //     ~handle: int,
  //     ~context: Exthost.CodeAction.Context.t,
  //     ~buffer: Oni_Core.Buffer.t,
  //     ~range: CharacterRange.t,
  //     ~toMsg: result(option(Exthost.CodeAction.List.t), string) => 'msg,
  //     Exthost.Client.t
  //   ) =>
  //   Isolinear.Sub.t('msg);

  let codeActionsByRange:
    (
      ~handle: int,
      ~context: Exthost.CodeAction.Context.t,
      ~buffer: Oni_Core.Buffer.t,
      ~range: CharacterRange.t,
      ~toMsg: result(option(Exthost.CodeAction.List.t), string) => 'msg,
      Exthost.Client.t
    ) =>
    Isolinear.Sub.t('msg);

  let codeLenses:
    (
      ~handle: int,
      ~eventTick: int,
      ~buffer: Oni_Core.Buffer.t,
      ~startLine: EditorCoreTypes.LineNumber.t,
      ~stopLine: EditorCoreTypes.LineNumber.t,
      ~toMsg: result(list(Exthost.CodeLens.lens), string) => 'a,
      Exthost.Client.t
    ) =>
    Isolinear.Sub.t('a);

  let completionItems:
    (
      ~handle: int,
      ~context: Exthost.CompletionContext.t,
      ~buffer: Oni_Core.Buffer.t,
      ~position: EditorCoreTypes.CharacterPosition.t,
      ~toMsg: result(Exthost.SuggestResult.t, string) => 'a,
      Exthost.Client.t
    ) =>
    Isolinear.Sub.t('a);

  let completionItem:
    (
      ~handle: int,
      ~chainedCacheId: Exthost.ChainedCacheId.t,
      ~defaultRange: Exthost.SuggestItem.SuggestRange.t,
      ~toMsg: result(Exthost.SuggestItem.t, string) => 'a,
      Exthost.Client.t
    ) =>
    Isolinear.Sub.t('a);

  let definition:
    (
      ~handle: int,
      ~buffer: Oni_Core.Buffer.t,
      ~position: EditorCoreTypes.CharacterPosition.t,
      ~toMsg: list(Exthost.DefinitionLink.t) => 'a,
      Exthost.Client.t
    ) =>
    Isolinear.Sub.t('a);

  let documentHighlights:
    (
      ~handle: int,
      ~buffer: Oni_Core.Buffer.t,
      ~position: EditorCoreTypes.CharacterPosition.t,
      ~toMsg: list(Exthost.DocumentHighlight.t) => 'a,
      Exthost.Client.t
    ) =>
    Isolinear.Sub.t('a);

  let documentSymbols:
    (
      ~handle: int,
      ~buffer: Oni_Core.Buffer.t,
      ~toMsg: list(Exthost.DocumentSymbol.t) => 'a,
      Exthost.Client.t
    ) =>
    Isolinear.Sub.t('a);

  let renameEdits:
    (
      ~handle: int,
      ~buffer: Oni_Core.Buffer.t,
      ~position: EditorCoreTypes.CharacterPosition.t,
      ~newName: string,
      ~toMsg: result(option(Exthost.WorkspaceEdit.t), string) => 'msg,
      Exthost.Client.t
    ) =>
    Isolinear.Sub.t('msg);

  let signatureHelp:
    (
      ~handle: int,
      ~context: Exthost.SignatureHelp.RequestContext.t,
      ~buffer: Oni_Core.Buffer.t,
      ~position: EditorCoreTypes.CharacterPosition.t,
      ~toMsg: result(option(Exthost.SignatureHelp.Response.t), string) =>
              'msg,
      Exthost.Client.t
    ) =>
    Isolinear.Sub.t('msg);

  module SCM: {
    let originalUri:
      (
        ~handle: int,
        ~filePath: string,
        ~toMsg: Oni_Core.Uri.t => 'msg,
        Exthost.Client.t
      ) =>
      Isolinear.Sub.t('msg);
  };
};
