open EditorCoreTypes;
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
        ~update: Oni_Core.BufferUpdate.t,
        Exthost.Client.t,
        unit => 'msg
      ) =>
      Isolinear.Effect.t('msg);
  };

  module FileSystemEventService: {
    let onFileEvent:
      (~events: Exthost.Files.FileSystemEvents.t, Exthost.Client.t) =>
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

    let provideSignatureHelp:
      (
        ~handle: int,
        ~uri: Oni_Core.Uri.t,
        ~position: EditorCoreTypes.CharacterPosition.t,
        ~context: Exthost.SignatureHelp.RequestContext.t,
        Exthost.Client.t,
        result(option(Exthost.SignatureHelp.Response.t), string) => 'msg
      ) =>
      Isolinear.Effect.t('msg);
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

  let codeLenses:
    (
      ~handle: int,
      ~buffer: Oni_Core.Buffer.t,
      ~toMsg: result(list(Exthost.CodeLens.t), string) => 'a,
      Exthost.Client.t
    ) =>
    Isolinear.Sub.t('a);

  let completionItems:
    // TODO: ~base: option(string),
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
