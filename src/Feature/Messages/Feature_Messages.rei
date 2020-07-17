open Oni_Core;

[@deriving show]
type msg;

module Msg: {
  let exthost:
    (~dispatch: msg => unit, Exthost.Msg.MessageService.msg) =>
    Lwt.t(option(Exthost.Message.handle));
};

type model;

let initial: model;

type outmsg =
  | Nothing
  | Notification({
      kind: Feature_Notification.kind,
      message: string,
    })
  | Effect(Isolinear.Effect.t(msg));

let update: (msg, model) => (model, outmsg);

module View: {
  let make:
    (
      ~theme: ColorTheme.Colors.t,
      ~model: model,
      ~font: UiFont.t,
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
};
