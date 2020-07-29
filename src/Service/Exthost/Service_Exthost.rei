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

  module SCM: {
    let provideOriginalResource:
      (
        ~handles: list(int),
        Exthost.Client.t,
        string,
        Oni_Core.Uri.t => 'msg
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
        ~range: Range.t,
        ~options: Exthost.FormattingOptions.t,
        Exthost.Client.t,
        result(list(Exthost.Edit.SingleEditOperation.t), string) => 'msg
      ) =>
      Isolinear.Effect.t('msg);

    let provideHover:
      (
        ~handle: int,
        ~uri: Oni_Core.Uri.t,
        ~position: EditorCoreTypes.Location.t,
        Exthost.Client.t,
        result(Exthost.Hover.t, string) => 'msg
      ) =>
      Isolinear.Effect.t('msg);

    let provideReferences:
      (
        ~handle: int,
        ~uri: Oni_Core.Uri.t,
        ~position: EditorCoreTypes.Location.t,
        ~context: Exthost.ReferenceContext.t,
        Exthost.Client.t,
        result(list(Exthost.Location.t), string) => 'msg
      ) =>
      Isolinear.Effect.t('msg);

    let provideSignatureHelp:
      (
        ~handle: int,
        ~uri: Oni_Core.Uri.t,
        ~position: EditorCoreTypes.Location.t,
        ~context: Exthost.SignatureHelp.RequestContext.t,
        Exthost.Client.t,
        result(option(Exthost.SignatureHelp.Response.t), string) => 'msg
      ) =>
      Isolinear.Effect.t('msg);
  };
};

module Sub: {
  let buffer:
    (~buffer: Oni_Core.Buffer.t, ~client: Exthost.Client.t) =>
    Isolinear.Sub.t(unit);

  let editor:
    (~editor: Exthost.TextEditor.AddData.t, ~client: Exthost.Client.t) =>
    Isolinear.Sub.t(unit);

  let activeEditor:
    (~activeEditorId: string, ~client: Exthost.Client.t) =>
    Isolinear.Sub.t(unit);

  let definition:
    (
      ~handle: int,
      ~buffer: Oni_Core.Buffer.t,
      ~position: EditorCoreTypes.Location.t,
      ~toMsg: list(Exthost.DefinitionLink.t) => 'a,
      Exthost.Client.t
    ) =>
    Isolinear.Sub.t('a);

  let documentHighlights:
    (
      ~handle: int,
      ~buffer: Oni_Core.Buffer.t,
      ~position: EditorCoreTypes.Location.t,
      ~toMsg: list(Exthost.DocumentHighlight.t) => 'a,
      Exthost.Client.t
    ) =>
    Isolinear.Sub.t('a);
};
