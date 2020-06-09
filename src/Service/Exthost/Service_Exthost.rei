// EFFECTS
module Effects: {
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
    let provideHover:
      (
        ~handle: int,
        ~uri: Oni_Core.Uri.t,
        ~position: EditorCoreTypes.Location.t,
        Exthost.Client.t,
        result(Exthost.Hover.t, string) => 'msg
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
};
