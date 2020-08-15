// SUBSCRIPTION

let subscription:
  (
    ~buffers: list(Oni_Core.Buffer.t),
    ~editors: list(Feature_Editor.Editor.t),
    ~activeEditorId: option(Feature_Editor.EditorId.t),
    ~client: Exthost.Client.t
  ) =>
  Isolinear.Sub.t(unit);
