// MODEL

[@deriving show]
type msg;

module Msg: {
  let document: (Exthost.Msg.Documents.msg, Lwt.u(Exthost.Reply.t)) => msg;

  let textEditors:
    (Exthost.Msg.TextEditors.msg, Lwt.u(Exthost.Reply.t)) => msg;

  let initialized: msg;
};

type model;

let initial: model;

let isInitialized: model => bool;

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

let update:
  (
    ~buffers: Feature_Buffers.model,
    ~editors: list(Feature_Editor.Editor.t),
    msg,
    model
  ) =>
  (model, outmsg);

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
