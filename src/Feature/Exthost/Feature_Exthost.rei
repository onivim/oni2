[@deriving show]
type msg;

module Msg: {
  let document: (Exthost.Msg.Documents.msg, Lwt.u(Exthost.Reply.t)) => msg;
};

type model;

let initial: model;

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

let update: (msg, model) => (model, outmsg);

// SUBSCRIPTION

let subscription:
  (
    ~buffers: Feature_Buffers.model,
    ~editors: list(Feature_Editor.Editor.t),
    ~activeEditorId: option(Feature_Editor.EditorId.t),
    ~client: Exthost.Client.t,
    model
  ) =>
  Isolinear.Sub.t(msg);
